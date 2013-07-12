/*Copyright (C) 2010-2012 Institute for Software Integrated Systems (ISIS)
This software was developed by the Institute for Software Integrated
Systems (ISIS) at Vanderbilt University, Tennessee, USA for the 
Transformative Apps program under DARPA, Contract # HR011-10-C-0175.
The United States Government has unlimited rights to this software. 
The US government has the right to use, modify, reproduce, release, 
perform, display, or disclose computer software or computer software 
documentation in whole or in part, in any manner and for any 
purpose whatsoever, and to have or authorize others to do so.
*/
package edu.vu.isis.ammo.rmcastplugin;

import java.io.File;
import java.net.InetAddress;
import java.net.NetworkInterface;
import java.net.SocketException;
import java.nio.BufferUnderflowException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.ArrayList;
import java.util.Comparator;
import java.util.Enumeration;
import java.util.List;
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.PriorityBlockingQueue;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.zip.CRC32;

import org.jgroups.Address;
import org.jgroups.Channel;
import org.jgroups.ChannelListener;
import org.jgroups.JChannel;
import org.jgroups.MembershipListener;
import org.jgroups.ReceiverAdapter;
import org.jgroups.View;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import edu.vu.isis.ammo.core.pb.AmmoMessages;

/**
 * Two long running threads and one short.
 * The long threads are for sending and receiving messages.
 * The short thread is to connect the socket.
 * The sent messages are placed into a queue if the socket is connected.
 *
 */
class ReliableMulticastConnector {
//    private static final Logger logger = LoggerFactory.getLogger("net.rmcast");
    private Logger logger = null;

    private static final int BURP_TIME = 5 * 1000; // 5 seconds expressed in milliseconds
    
    /**
     * $ sysctl net.ipv4.tcp_rmem 
     * or
     * $ cat /proc/sys/net/ipv4/tcp_rmem
     * 4096   87380   4194304
     * 0x1000 0x15554 0x400000
     * 
     * The first value tells the kernel the minimum receive buffer for each TCP connection, and 
     * this buffer is always allocated to a TCP socket, even under high pressure on the system.
     * 
     * The second value specified tells the kernel the default receive buffer allocated for each TCP socket. 
     * This value overrides the /proc/sys/net/core/rmem_default value used by other protocols.
     * 
     * The third and last value specified in this variable specifies the maximum receive buffer 
     * that can be allocated for a TCP socket.
     * 
     */

    // Three threads
    private ConnectorThread connectorThread; // long running
    private SenderThread mSender;	     // created after connection
    private ChannelReceiver mReceiver;	     // created after connection

    private JChannel mJGroupChannel;
    private String mMulticastAddress = null;
    private int mMulticastPort = -1;

    private ByteOrder endian = ByteOrder.LITTLE_ENDIAN;

    private final SenderQueue mSenderQueue;
    private final PluginServiceHandler mPlugin;

    /**
     * Constants
     **/
    static final int EXCEPTION       =  1; // the run failed by some unhandled exception

    static final int CONNECTING      = 20; // trying to connect
    static final int CONNECTED       = 21; // the socket is good an active

    static final int DISCONNECTED    = 30; // the socket is disconnected
    static final int STALE           = 31; // indicating there is a message

    static final int WAIT_CONNECT    = 40; // waiting for connection
    static final int SENDING         = 41; // indicating the next thing is the size
    static final int TAKING          = 42; // indicating the next thing is the size
    static final int INTERRUPTED     = 43; // the run was canceled via an interrupt

    static final int START           = 52; // indicating the next thing is the size
    static final int RESTART         = 53; // indicating the next thing is the size
    static final int WAIT_RECONNECT  = 54; // waiting for connection
    static final int STARTED         = 55; // indicating there is a message
    static final int SIZED           = 56; // indicating the next thing is a checksum
    static final int CHECKED         = 57; // indicating the bytes are being read
    static final int DELIVER         = 58; // indicating the message has been read

    static final int GATEWAY_HEADER_SIZE =
	  4 // magic
	+ 4 // message size
	+ 1 // priority
	+ 3 // reserved
	+ 4 // payload checksum
	+ 4; // header checksum
    static final int GATEWAY_MESSAGE_MAGIC = 0xdeadbeef;
    //static final byte[] GATEWAY_MESSAGE_MAGICB = { (byte)0xde, (byte)0xad, (byte)0xbe, (byte)0xef };
    static final byte[] GATEWAY_MESSAGE_MAGICB = { (byte)0xed, (byte)0xad, (byte)0xbe, (byte)0xef };

    private String mChannelName = null;
    private String mConfigFile = null;
    private String mGroupName = null;

    public ReliableMulticastConnector(PluginServiceHandler plugin, 
				      String multicastAddr, 
				      int multicastPort,
				      String channelName,
				      String configFile,
				      String groupName) {
  logger = LoggerFactory.getLogger("net." + mChannelName);
	logger.info("Thread <{}>ReliableMulticastConnector::<constructor>", Thread.currentThread().getId());
	this.mPlugin = plugin;
	this.mMulticastAddress = multicastAddr;
	this.mMulticastPort = multicastPort;
	this.mGroupName = groupName;
	
	mChannelName = channelName;
	mConfigFile = configFile;

	this.connectorThread = new ConnectorThread(this);
	mSenderQueue = new SenderQueue( this );
	this.connectorThread.start();
    }

    public boolean isConnected() { 
	return this.connectorThread.isConnected(); 
    }

    public boolean close() { return false; }

    public String toString() {
	return new StringBuilder().append("channel ").append(super.toString())
            .append("socket: host[").append(this.mMulticastAddress).append("] ")
            .append("port[").append(this.mMulticastPort).append("]").toString();
    }

    /**
     * forces a reconnection.
     */
    // Called by ReceiverThread to send an incoming message to the
    // appropriate destination.
    private boolean deliverMessage( AmmoMessages.MessageWrapper message )
    {
	logger.info( "deliverMessage() {} ", message );
	mPlugin.onAmmoMessageReceived(message);
	
	return true;
    }

    /**
     * do your best to send the message.
     * This makes use of the blocking "put" call.
     * A proper producer-consumer should use put or add and not offer.
     * "put" is blocking call.
     * If this were on the UI thread then offer would be used.
     *
     * @param agm AmmoMessages.MessageWrapper
     * @return
     */
    public boolean sendMessage( AmmoMessages.MessageWrapper agm )
    {
        return mSenderQueue.put( agm );
    }

    ///////////////////////////////////////////////////////////////////////////
    //
    class SenderQueue
    {
	class PriorityComparator implements Comparator<AmmoMessages.MessageWrapper> {
	    @Override
		public int  compare(AmmoMessages.MessageWrapper x, AmmoMessages.MessageWrapper y)
	    {
		if (x.getMessagePriority() > y.getMessagePriority() )
		    return 1;
		else if (x.getMessagePriority() == y.getMessagePriority() )
		    return 0;

		return -1;
	    }
	    
	}

        public SenderQueue( ReliableMulticastConnector iChannel )
        {
            mChannel = iChannel;
            mDistQueue = new PriorityBlockingQueue<AmmoMessages.MessageWrapper>( 20, new PriorityComparator() );
        }

        public boolean put(AmmoMessages.MessageWrapper iMessage )
        {
            logger.info( "insert into sender queue: {}", mDistQueue.size() );
            try {
		if (! mDistQueue.offer( iMessage, 1, TimeUnit.SECONDS )) {
		    logger.warn("channel not taking messages {}", "BUSY" );
		    return false;
		}
	    } catch (InterruptedException e) {
		return false;
	    }
            return true;
        }

        public AmmoMessages.MessageWrapper take() throws InterruptedException
        {
            logger.info( "taking from SenderQueue" );
	    // This is where the authorized SenderThread blocks.
	    return mDistQueue.take();
        }


        // Somehow synchronize this here.
        public synchronized void reset()
        {
            logger.info( "reset()ing the SenderQueue" );
            // Tell the distributor that we couldn't send these
            // packets.
            // AmmoMessages.MessageWrapper msg = mDistQueue.poll();
            // while ( msg != null )
	    // 	{
	    // 	    if ( msg.handler != null )
	    // 		mChannel.ackToHandler( msg.handler, ChannelDisposal.PENDING );
	    // 	    msg = mDistQueue.poll();
	    // 	}
        }

        private BlockingQueue<AmmoMessages.MessageWrapper> mDistQueue;
        private ReliableMulticastConnector mChannel;
    }


    /**
     * manages the connection.
     * enable or disable expresses the operator intent.
     * There is no reason to run the thread unless the channel is enabled.
     *
     * Any of the properties of the channel
     *
     */
    private class ConnectorThread extends Thread implements ChannelListener { // jgroups listener interface
//	private final Logger logger = LoggerFactory.getLogger( "net.rmcast.connector" );
	private Logger logger = null;

	private final String DEFAULT_HOST = "127.0.0.1";
	private final int DEFAULT_PORT = 12475;
	private final int GATEWAY_RETRY_TIME = 20 * 1000; // 20 seconds

	private ReliableMulticastConnector parent;
	private final State state;

	private AtomicBoolean mIsConnected;

	// methods from JGroups ChannelListener
	@Override
	    public void channelConnected(Channel channel) { parent.mPlugin.onConnect(); } // jgroups connected callback
	@Override
	    public void channelDisconnected(Channel channel) { parent.mPlugin.onDisconnect(); socketOperationFailed(); } // MJ: is this right
	@Override
	    public void channelClosed(Channel channel) { socketOperationFailed(); } // MJ: is this right

	// Called by the sender and receiver when they have an exception on the
	// SocketChannel.  We only want to call reset() once, so we use an
	// AtomicBoolean to keep track of whether we need to call it.
	public void socketOperationFailed()
	{
	    if ( mIsConnected.compareAndSet( true, false )) {
		disconnect();
	    }
	}


	private ConnectorThread(ReliableMulticastConnector parent) {
	    logger = LoggerFactory.getLogger( "net." + parent.mChannelName + ".connector" );
	    logger.info("Thread <{}>ConnectorThread::<constructor>", Thread.currentThread().getId());
	    this.parent = parent;
	    this.state = new State();
	    mIsConnected = new AtomicBoolean( false );
      CONFIG_FILE = "jgroups/" + mConfigFile;
	}

	private class State {
	    private int value;

	    public State() {
		this.value = DISCONNECTED;
	    }
	    public synchronized void set(int state) {
		logger.info("Thread <{}>State::set", Thread.currentThread().getId());
		this.value = state;
		this.notifyAll();
	    }
            public synchronized int get() { return this.value; }

            public synchronized boolean isConnected() {
                return this.value == CONNECTED;
            }
        }

        public boolean isConnected() {
            return this.state.isConnected();
        }

        /**
         * A value machine based.
         * Most of the time this machine will be in a CONNECTED value.
         * In that CONNECTED value the machine wait for the connection value to
         * change or for an interrupt indicating that the thread is being shut down.
         *
         *  The value machine takes care of the following constraints:
         * We don't need to reconnect unless.
         * 1) the connection has been lost
         * 2) the connection has been marked stale
         * 3) the connection is enabled.
         * 4) an explicit reconnection was requested
         *
         * @return
         */
        @Override
	    public void run()
	{
            try {
                logger.info("Thread <{}>ConnectorThread::run", Thread.currentThread().getId());
                while (true) {
                    switch (this.state.get()) {
                    case ReliableMulticastConnector.DISCONNECTED:
			if ( !this.connect() ) {
			    try {
				Thread.sleep(GATEWAY_RETRY_TIME);
			    } catch (InterruptedException ex) {
				logger.info("sleep interrupted - intentional disable, exiting thread ...");
			    }
			}
			break;

                    case ReliableMulticastConnector.CONNECTED:
			try {
			    synchronized (this.state) {
				while (this.isConnected())
				    this.state.wait(BURP_TIME);
			    }
			} catch (InterruptedException ex) {
			    logger.warn("connection intentionally disabled {}", this.state );
			}
			break;
                    }
                }

            } catch (Exception ex) {
                logger.error("channel exception {} \n {}", ex.getLocalizedMessage(), ex.getStackTrace());
            }
		
        }


        private boolean connect()
        {
	    // connect should only be called from connector thread 
            logger.info( "Thread <{}>ConnectorThread::connect", Thread.currentThread().getId() );

            // Create the MulticastSocket.
            if ( parent.mJGroupChannel != null )
                logger.error( "Tried to create mJGroupChannel when we already had one." );
            try
            {
            	File configFile = new File( findConfigFile("jgroups/" + mConfigFile) );
            	parent.mJGroupChannel = new JChannel( configFile );
		parent.mJGroupChannel.addChannelListener( this );
            	//parent.mJGroupChannel.setName( getLocalIpAddress() );
		          parent.mJGroupChannel.setName( getLocalIpAddress() + " " + mChannelName );
            	//parent.mJGroupChannel.setOpt( Channel.AUTO_RECONNECT, Boolean.TRUE ); // deprecated
            }
            catch ( Exception e )
            {
                logger.warn( "attempt to create JChannel failed: " + e.getLocalizedMessage() );
                parent.mJGroupChannel = null;
                return false;
            }

            logger.info( "connection to JChannel established ",
                         parent.mMulticastAddress,
                         parent.mMulticastPort );

            mIsConnected.set( true );
	    state.set(ReliableMulticastConnector.CONNECTED);

            // Create the sending thread.
            if ( parent.mSender != null )
                logger.error( "Tried to create Sender when we already had one." );
            parent.mSender = new SenderThread( this,
                                               parent,
                                               parent.mSenderQueue,
                                               parent.mJGroupChannel );
            parent.mSender.start();

            // Create the channel receiver
            if ( parent.mReceiver != null )
                logger.error( "Tried to create Receiver when we already had one." );
            parent.mReceiver = new ChannelReceiver( this, parent );

	    // providing receiver to JGroups
	    parent.mJGroupChannel.setReceiver( parent.mReceiver );
	    
	    // connect Jchannel
	    try {
		parent.mJGroupChannel.connect( mGroupName);
	    } catch ( Exception ex ) {
		logger.error( "Exception while connecting to JGroups channel:  {} \n {}", ex.getMessage(), ex.getStackTrace() );
	    }

            return true;
        }


        private boolean disconnect()
        {
	    // disconnect should only be called from sender/receiver or app thread 
            logger.info( "Thread <{}>ConnectorThread::disconnect",
                         Thread.currentThread().getId() );
	    boolean ret = true;
	    mIsConnected.set( false );

	    if ( mSender != null ) {
		logger.debug( "interrupting SenderThread" );
		mSender.interrupt();
	    }

	    mSenderQueue.reset();

	    if ( parent.mJGroupChannel != null )
                {
                    logger.debug( "Closing ReliableMulticastSocket." );
                    parent.mJGroupChannel.close(); // will disconnect first if still connected
                    logger.debug( "Done" );

                    parent.mJGroupChannel = null;
                }

	    parent.mSender = null;
	    parent.mReceiver = null;
            logger.debug( "returning after successful disconnect()." );

	    // we need to pend on the sender / receiver thread before we attempt a reconnect
	    try {
		if (mSender != null && Thread.currentThread().getId() != mSender.getId() )
		    mSender.join();
	    } catch (java.lang.InterruptedException ex) {
		logger.warn("disconnect: interrupted exception while waiting for sender thread to die");
	    }

	    // setting the state to disconnected will cause the connector thread to attempt a reconnect
	    state.set(ReliableMulticastConnector.DISCONNECTED);
            return ret;
        }

        private final static String CONFIG_DIRECTORY = "ammo-gateway";
        private String CONFIG_FILE = "jgroups/udp.xml";

        private String findConfigFile( String configFile ) {
            final String os = System.getProperty("os.name").toLowerCase();
            String filePath;

            if (os.indexOf("win") >= 0) {
                filePath = findConfigFileWindows(configFile);
            }
            else {
                filePath = findConfigFileLinux(configFile);
            }

            logger.info("findConfigFile: using config file {}", filePath);
            return filePath;
        }

        /**
         * Searches for the gateway config file.  Search order:
         *   1) The current working directory
         *   2) ~/.ammo-gateway/
         *   3) /etc/ammo-gateway/
         *   Fallback locations (don't rely on these; they may change or disappear in a
         *   future release.  Gateway installation should put the config file into
         *   a location that's searched by default):
         *   4) $GATEWAY_ROOT/etc
         *   5) $GATEWAY_ROOT/build/etc
         *   6) ../etc
         */
        private String findConfigFileLinux( String configFile ) {
            String filePath = configFile;
            String home = System.getenv("HOME");
            if (home == null) home = new String("");
            String gatewayRoot = System.getenv("GATEWAY_ROOT");
            if (gatewayRoot == null) gatewayRoot = new String("");

            if (new File(filePath).exists() == false) {
                filePath = home + "/." + CONFIG_DIRECTORY + "/" + CONFIG_FILE;
                if (new File(filePath).exists() == false) {
                    filePath = new String("/etc/") + CONFIG_DIRECTORY + "/" + CONFIG_FILE;
                    if (new File(filePath).exists() == false) {
                        filePath = gatewayRoot + "/etc/" + CONFIG_FILE;
                        if (new File(filePath).exists() == false) {
                            filePath = gatewayRoot + "/build/etc/" + CONFIG_FILE;
                            if (new File(filePath).exists() == false) {
                                filePath = new String("../etc/") + CONFIG_FILE;
                                if (new File(filePath).exists() == false) {
                                    logger.error("findConfigFile: unable to find config file");
                                    return "";
                                }
                            }
                        }
                    }
                }
            }

            return filePath;
        }

        /**
         * Searches for the gateway config file.  Search order:
         *   1) The current working directory
         *   2) The user's configuration directory (Roaming appdata directory/ammo-gateway)
         *   3) The all users configuration directory (i.e. C:\ProgramData\ammo-gateway on Vista/7)
         *   Fallback locations (don't rely on these; they may change or disappear in a
         *   future release.  Gateway installation should put the config file into
         *   a location that's searched by default):
         *   4) $GATEWAY_ROOT/etc
         *   5) $GATEWAY_ROOT/build/etc
         *   6) ../etc
         */
        private String findConfigFileWindows( String configFile ) {
            String filePath = configFile;
            String userConfigPath = System.getenv("APPDATA");
            if (userConfigPath == null) userConfigPath = new String("");
            String systemConfigPath = System.getenv("PROGRAMDATA");
            if (systemConfigPath == null) systemConfigPath = new String("");
            String gatewayRoot = System.getenv("GATEWAY_ROOT");
            if (gatewayRoot == null) gatewayRoot = new String("");

            if (new File(filePath).exists() == false) {
                filePath = userConfigPath + "/" + CONFIG_DIRECTORY + "/" + CONFIG_FILE;
                if (new File(filePath).exists() == false) {
                    filePath = systemConfigPath + "/" + CONFIG_DIRECTORY + "/" + CONFIG_FILE;
                    if (new File(filePath).exists() == false) {
                        filePath = gatewayRoot + "/etc/" + CONFIG_FILE;
                        if (new File(filePath).exists() == false) {
                            filePath = gatewayRoot + "/build/etc/" + CONFIG_FILE;
                            if (new File(filePath).exists() == false) {
                                filePath = new String("../etc/") + CONFIG_FILE;
                                if (new File(filePath).exists() == false) {
                                    logger.error("findConfigFile: unable to find config file");
                                    return "";
                                }
                            }
                        }
                    }
                }
            }

            return filePath;
        }
    }


    ///////////////////////////////////////////////////////////////////////////
    //
    class SenderThread extends Thread
    {
        public SenderThread( ConnectorThread iParent,
                             ReliableMulticastConnector iChannel,
                             SenderQueue iQueue,
                             JChannel iJChannel )
        {
            logger = LoggerFactory.getLogger( "net." + mChannelName + ".sender" );
            mParent = iParent;
            mChannel = iChannel;
            mQueue = iQueue;
            mJChannel = iJChannel;
        }


        /**
         * the message format is
         * 
         */
        @Override
	    public void run()
	{
            logger.info( "Thread <{}>::run()", Thread.currentThread().getId() );

            // Block on reading from the queue until we get a message to send.
            // Then send it on the socket channel. Upon getting a socket error,
            // notify our parent and go into an error state.

            while ( mState != ReliableMulticastConnector.INTERRUPTED ) {
		AmmoMessages.MessageWrapper msg = null;
		try	{
		    setSenderState( ReliableMulticastConnector.TAKING );
		    msg = mQueue.take(); // The main blocking call
		    logger.debug( "Took a message from the send queue" );
		} catch ( InterruptedException ex )	{
		    logger.debug( "interrupted taking messages from send queue: {}",
				  ex.getLocalizedMessage() );
		    break;
		}

		try	{
		    int payloadSize = msg.getSerializedSize();
		    byte[] payload = msg.toByteArray();

		    ByteBuffer buf = ByteBuffer.allocate( GATEWAY_HEADER_SIZE  + payloadSize );
		    if (buf == null) {
			logger.error("failed to allocate memory for header byte buffer: {}", GATEWAY_HEADER_SIZE); 
		    }
		    buf.order(endian);

		    buf.clear(); // prepare buffer for writing

		    buf.put( (byte)GATEWAY_MESSAGE_MAGICB[3] );
		    buf.put( (byte)GATEWAY_MESSAGE_MAGICB[2] );
		    buf.put( (byte)GATEWAY_MESSAGE_MAGICB[0] );
		    buf.put( (byte)0xfe ); // VERSION_1_FULL 

		    buf.putInt(payloadSize);
		    buf.put( (byte)msg.getMessagePriority() );
		    buf.put( (byte)0);
		    buf.put( (byte)0);
		    buf.put( (byte)0);
		    // payload checksum
		    CRC32 crc32Payload = new CRC32();
		    crc32Payload.update(payload);
		    buf.putInt( (int)crc32Payload.getValue() );
		    // header checksum
		    CRC32 crc32Header = new CRC32();
		    crc32Header.update( buf.array(), 0, GATEWAY_HEADER_SIZE - 4 );
		    buf.putInt( (int)crc32Header.getValue() );

		    // copy data
		    buf.put(payload);
		    
		    // prepare for draining
		    buf.flip();

		    setSenderState( ReliableMulticastConnector.SENDING );
		    mJChannel.send( null, buf.array() );
		    logger.info( "Wrote to JChannel" );

		    // legitimately sent to gateway.
		    // if ( msg.handler != null )
		    // 	mChannel.ackToHandler( msg.handler, ChannelDisposal.SENT );
		}  catch ( Exception ex ) {
		    logger.warn("sender threw exception {} \n {}", ex.getMessage(), ex.getStackTrace() );
		    break;
		}
	    }
	    logger.error("sender thread exiting...");
	    mParent.socketOperationFailed();
        }


        private void setSenderState( int iState )
        {
            synchronized ( this ) {
		mState = iState;
	    }
        }

        public synchronized int getSenderState() { return mState; }

        private int mState = ReliableMulticastConnector.TAKING;
        private ConnectorThread mParent;
        private ReliableMulticastConnector mChannel;
        private SenderQueue mQueue;
        private JChannel mJChannel;
        private Logger logger = null;
    }


    ///////////////////////////////////////////////////////////////////////////
    //
    class ChannelReceiver extends ReceiverAdapter implements MembershipListener
    {
        public ChannelReceiver( ConnectorThread iParent,
                               ReliableMulticastConnector iDestination )
        {
            mParent = iParent;
            mDestination = iDestination;
            logger = LoggerFactory.getLogger( "net." + mChannelName + ".receiver" );
        }


        @Override
        public void receive( org.jgroups.Message msg )
        {
            logger.info( "Thread <{}>: ChannelReceiver::receive() size {}, src {} ", new Object[] {Thread.currentThread().getId(), msg.getLength(), msg.getSrc() } );
	    List<String> addresses = getLocalIpAddresses();
	    if ( addresses.contains(  msg.getSrc().toString() ) )
		return;		// loopback message - discard it



	    ByteBuffer buf = ByteBuffer.wrap( msg.getBuffer() );
	    buf.order( endian );

	    Message message = extractHeader( buf );
	    int readState = extractMessage( buf, message );

	    if (readState == 0)	{// should have received a full message
		try {
		    AmmoMessages.MessageWrapper agm =
		        AmmoMessages.MessageWrapper.parseFrom(message.payload);
		    logger.info( "received a message {}", agm.getType() );
		    mDestination.deliverMessage( agm );
		} catch(com.google.protobuf.InvalidProtocolBufferException ex) {
		    logger.error("exception while parsing message protocol buffer: {} \n {}", ex.getMessage(), ex.getStackTrace() );
		}
	    } else {
		logger.error( "Jchannel receiver called recieve without a complete message" );
	    }

        }

        @Override
        public void viewAccepted(View new_view)
        {
            // I have kept this error, need to change it to info .. NR
            logger.error( "Membership View Changed: {}", new_view );
        }
        
        @Override
        public void suspect(Address suspected_mbr)
        {
            logger.error( "Member Suspected : {}", suspected_mbr.toString());
        }

	private class Message {
	    int  payloadSize;
	    byte priority;
	    byte error;
	    short reserved;
	    int  payloadChksum;
	    int  headerChksum;
	    byte[] payload;
	    int  receivedSize;
	}

	Message extractHeader(ByteBuffer bbuf)
	{
	    try {
		bbuf.mark();
		
		// scan for magic
		while (true) {
		    if (bbuf.get() != GATEWAY_MESSAGE_MAGICB[3]) continue;
		    if (bbuf.get() != GATEWAY_MESSAGE_MAGICB[2]) continue;
		    //if (bbuf.get() != GATEWAY_MESSAGE_MAGICB[1]) continue;
		    if (bbuf.get() != GATEWAY_MESSAGE_MAGICB[0]) continue;
		    bbuf.get();   // this is the VERSION byte - TBD SKN process this to differentiate between FULL and TERSE encoding
		    break;
		}
		// got a magic sequence - now should see the header proper
		Message ret = new Message();
		ret.payloadSize = bbuf.getInt();
		ret.priority = bbuf.get();
		ret.error = bbuf.get();
		ret.reserved = bbuf.getShort(); // reserved field - just eat it
		ret.payloadChksum = bbuf.getInt();
		ret.headerChksum = bbuf.getInt();

		// header checksum is over the (length of header - size of checksum field)
		CRC32 crc = new CRC32();
		crc.update( bbuf.array(), bbuf.position() - GATEWAY_HEADER_SIZE, GATEWAY_HEADER_SIZE - 4 ); 
		
		if ( ret.headerChksum != (int)crc.getValue() ) {
		    logger.error("extractHeader: header checksum mismatch: sent {}, computed {}", Long.toHexString(ret.headerChksum), Long.toHexString( (int)crc.getValue() ) );
		    return null;
		}

		// allocate space for message
		ret.payload = new byte[ret.payloadSize];
		ret.receivedSize = 0; // set the received counter to 0
		return ret;
	    } catch(BufferUnderflowException ex) {
		logger.error("extractHeader: {}", ex.getMessage());
		ex.printStackTrace();
		bbuf.reset();
	    }
	    return null;
	}

	int extractMessage(ByteBuffer bbuf, Message message)
	{
	    if (message == null) {
		logger.error("extractMessage: message is null - internal error");
		return 0;
	    }

	    logger.info("extractMessage: available in buf {}, read in msg {}",
			bbuf.remaining(),
			message.receivedSize);
	    int toGet = message.payloadSize - message.receivedSize;
	    toGet = Math.min(toGet, bbuf.remaining());
	    bbuf.get(message.payload, message.receivedSize, toGet);
	    message.receivedSize += toGet;
	    
	    if (message.receivedSize >= message.payloadSize) { // got a complete message
		CRC32 crc = new CRC32();
		crc.update(message.payload);

		if ( message.payloadChksum != (int)crc.getValue() ) {
		    logger.error("extractMessage: payload checksum mismatch: sent {}, received {}",
				 Long.toHexString(message.payloadChksum),
				 Long.toHexString( (int)crc.getValue() ));
		}
		return 0;	// revert back to reading header
	    }
	    return 1;		// continue to read more in message
	}


        private void setReceiverState( int iState )
        {
            synchronized ( this ) {
		mState = iState;
	    }
        }

        public synchronized int getReceiverState() { return mState; }

        private int mState = ReliableMulticastConnector.TAKING; // FIXME
        private ConnectorThread mParent;
        private ReliableMulticastConnector mDestination;
        private Logger logger = null;
    }


    // ********** UTILITY METHODS ****************

    /**
     * A routine to get the local ip address
     * TODO use this someplace
     *
     * @return
     */
    public List<String> getLocalIpAddresses()
    {
        List<String> addresses = new ArrayList<String>();
        try
        {
            for (Enumeration<NetworkInterface> en = NetworkInterface.getNetworkInterfaces(); en.hasMoreElements();)
            {
                NetworkInterface intf = en.nextElement();
                for (Enumeration<InetAddress> enumIpAddr = intf.getInetAddresses(); enumIpAddr.hasMoreElements();)
                {
                    InetAddress inetAddress = enumIpAddr.nextElement();
                    addresses.add( inetAddress.getHostAddress().toString() );
                }
            }
        }
        catch (SocketException ex)
        {
            logger.error("getLocalIpAddresses: {}", ex.toString());
        }

        return addresses;
    }
    public String getLocalIpAddress()
    {
        try
        {
            for (Enumeration<NetworkInterface> en = NetworkInterface.getNetworkInterfaces(); en.hasMoreElements();)
            {
                NetworkInterface intf = en.nextElement();
                for (Enumeration<InetAddress> enumIpAddr = intf.getInetAddresses(); enumIpAddr.hasMoreElements();)
                {
                    InetAddress inetAddress = enumIpAddr.nextElement();
                    logger.info( "address: {}", inetAddress );
		    if (!inetAddress.isLoopbackAddress()) {
			return inetAddress.getHostAddress().toString();
		    }
		    
                }
            }
        }
        catch (SocketException ex)
        {
            logger.error("getLocalIpAddresses: {}", ex.toString());
        }

        return null;
    }

	public boolean isBusy() {
	return false;
    }

}
