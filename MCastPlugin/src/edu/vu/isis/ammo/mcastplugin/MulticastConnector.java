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
package edu.vu.isis.ammo.mcastplugin;

import java.util.ArrayList;
import java.util.List;
import java.io.IOException;
import java.io.File;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.NetworkInterface;
import java.net.Socket;
import java.net.SocketException;
import java.net.UnknownHostException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.BufferUnderflowException;
import java.nio.channels.AsynchronousCloseException;
import java.nio.channels.ClosedChannelException;
import java.nio.channels.SocketChannel;
import java.util.Enumeration;
import java.util.LinkedList;
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.PriorityBlockingQueue;
import java.util.Comparator;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.atomic.AtomicLong;
import java.util.concurrent.atomic.AtomicReference;
import java.util.zip.CRC32;
import java.io.IOException;
import java.net.DatagramPacket;
import java.net.InetAddress;
import java.net.MulticastSocket;
import java.net.NetworkInterface;
import java.net.Socket;
import java.net.SocketException;
import java.net.UnknownHostException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;

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
class MulticastConnector {
    private static final Logger logger = LoggerFactory.getLogger("net.mcast");

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
    private ReceiverThread mReceiver;	     // created after connection

    private MulticastSocket mSocket;
    private String mMulticastAddress = null;
    private int mMulticastPort = -1;
    private InetAddress mMulticastGroup = null;
    private Integer mMulticastTTL;

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
    

    public MulticastConnector(PluginServiceHandler plugin, String multicastAddr, int multicastPort) {
	logger.info("Thread <{}>MulticastConnector::<constructor>", Thread.currentThread().getId());
	this.mPlugin = plugin;
	this.mMulticastAddress = multicastAddr;
	this.mMulticastPort = multicastPort;
	this.mMulticastTTL = 2;

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

        public SenderQueue( MulticastConnector iChannel )
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
        private MulticastConnector mChannel;
    }


    /**
     * manages the connection.
     * enable or disable expresses the operator intent.
     * There is no reason to run the thread unless the channel is enabled.
     *
     * Any of the properties of the channel
     *
     */
    private class ConnectorThread extends Thread { 
	private final Logger logger = LoggerFactory.getLogger( "net.mcast.connector" );

	private final String DEFAULT_MCAST_ADDRESS = "228.10.10.90";
	private final int DEFAULT_PORT = 9982;
	private final int GATEWAY_RETRY_TIME = 20 * 1000; // 20 seconds

	private MulticastConnector parent;
	private final State state;

	private AtomicBoolean mIsConnected;

	// Called by the sender and receiver when they have an exception on the
	// SocketChannel.  We only want to call reset() once, so we use an
	// AtomicBoolean to keep track of whether we need to call it.
	public void socketOperationFailed()
	{
	    if ( mIsConnected.compareAndSet( true, false )) {
		disconnect();
	    }
	}


	private ConnectorThread(MulticastConnector parent) {
	    logger.info("Thread <{}>ConnectorThread::<constructor>", Thread.currentThread().getId());
	    this.parent = parent;
	    this.state = new State();
	    mIsConnected = new AtomicBoolean( false );
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
                    case MulticastConnector.DISCONNECTED:
			if ( !this.connect() ) {
			    try {
				Thread.sleep(GATEWAY_RETRY_TIME);
			    } catch (InterruptedException ex) {
				logger.info("sleep interrupted - intentional disable, exiting thread ...");
			    }
			}
			break;

                    case MulticastConnector.CONNECTED:
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
            if ( parent.mSocket != null )
                logger.error( "Tried to create mSocket when we already had one." );
            try
            {
                parent.mMulticastGroup = InetAddress.getByName( parent.mMulticastAddress );
                parent.mSocket = new MulticastSocket( parent.mMulticastPort );
                parent.mSocket.joinGroup( parent.mMulticastGroup );
            }
            catch ( Exception e )
            {
                logger.warn( "connection to {}:{} failed: " + e.getLocalizedMessage(), parent.mMulticastGroup, parent.mMulticastPort );
                parent.mSocket = null;
                return false;
            }

            logger.info( "connection to {}:{} established ",
                         parent.mMulticastAddress,
                         parent.mMulticastPort );

            mIsConnected.set( true );
	    state.set(MulticastConnector.CONNECTED);

            // Create the sending thread.
            if ( parent.mSender != null )
                logger.error( "Tried to create Sender when we already had one." );
            parent.mSender = new SenderThread( this,
                                               parent,
                                               parent.mSenderQueue,
                                               parent.mSocket );
            parent.mSender.start();

            // Create the channel receiver
            if ( parent.mReceiver != null )
                logger.error( "Tried to create Receiver when we already had one." );
            parent.mReceiver = new ReceiverThread( this, parent, parent.mSocket );
            parent.mReceiver.start();

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

	    if ( parent.mSocket != null )
                {
                    logger.debug( "Closing MulticastSocket." );
                    parent.mSocket.close(); // will disconnect first if still connected
                    logger.debug( "Done" );

                    parent.mSocket = null;
                }

	    parent.mSender = null;
	    parent.mReceiver = null;
            logger.debug( "returning after successful disconnect()." );

	    // we need to pend on the sender / receiver thread before we attempt a reconnect
	    try {
		if (mSender != null && Thread.currentThread().getId() != mSender.getId() )
		    mSender.join();
		if (mReceiver != null && Thread.currentThread().getId() != mReceiver.getId() )
		    mReceiver.join();
	    } catch (java.lang.InterruptedException ex) {
		logger.warn("disconnect: interrupted exception while waiting for sender thread to die");
	    }

	    // setting the state to disconnected will cause the connector thread to attempt a reconnect
	    state.set(MulticastConnector.DISCONNECTED);
            return ret;
        }
    }


    ///////////////////////////////////////////////////////////////////////////
    //
    class SenderThread extends Thread
    {
        public SenderThread( ConnectorThread iParent,
                             MulticastConnector iChannel,
                             SenderQueue iQueue,
                             MulticastSocket iSocket )
        {
            mParent = iParent;
            mChannel = iChannel;
            mQueue = iQueue;
            mSocket = iSocket;
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

            while ( mState != MulticastConnector.INTERRUPTED ) {
		AmmoMessages.MessageWrapper msg = null;
		try	{
		    setSenderState( MulticastConnector.TAKING );
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

		    setSenderState( MulticastConnector.SENDING );
		    DatagramPacket packet =
			new DatagramPacket( buf.array(),
					    buf.remaining(),
					    mChannel.mMulticastGroup,
					    mChannel.mMulticastPort );
		    mSocket.setTimeToLive( mChannel.mMulticastTTL );
		    mSocket.send( packet );
		    logger.info( "Wrote to MulticastSocket" );

		    // legitimately sent to gateway.
		    // if ( msg.handler != null )
		    // 	mChannel.ackToHandler( msg.handler, ChannelDisposal.SENT );
		}  catch ( Exception ex ) {
		    logger.warn("sender threw exception {} \n {}", ex.getMessage(), ex.getStackTrace() );
		    ex.printStackTrace();
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

        private int mState = MulticastConnector.TAKING;
        private ConnectorThread mParent;
        private MulticastConnector mChannel;
        private SenderQueue mQueue;
        private MulticastSocket mSocket;
        private final Logger logger = LoggerFactory.getLogger( "net.mcast.sender" );
    }


    ///////////////////////////////////////////////////////////////////////////
    //
    class ReceiverThread extends Thread
    {
        public ReceiverThread( ConnectorThread iParent,
                               MulticastConnector iDestination,
			       MulticastSocket iSocket)
        {
            mParent = iParent;
            mDestination = iDestination;
	    mSocket = iSocket;
        }


        @Override
	    public void run()
        {
            logger.info( "Thread <{}>: ChannelReceiver::run()", Thread.currentThread().getId() );
            List<InetAddress> addresses = getLocalIpAddresses();
            byte[] raw = new byte[65536]; // FIXME: What is max datagram size?
            while ( getReceiverState() != MulticastConnector.INTERRUPTED ) {
                try {
                    DatagramPacket packet = new DatagramPacket( raw, raw.length );
                    setReceiverState( MulticastConnector.START );
                    mSocket.receive( packet );
                    if ( addresses.contains( packet.getAddress() ))
                    {
                        continue;
                    }
                    logger.info( "Received a packet from ({}) size({})", packet.getAddress(), packet.getLength()  );

                    ByteBuffer buf = ByteBuffer.wrap( packet.getData(),
                                                      packet.getOffset(),
                                                      packet.getLength() );
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
			logger.error( "Mcast receiver called recieve without a complete message" );
		    }
		} catch ( ClosedChannelException ex ) {
                    logger.warn( "receiver threw ClosedChannelException {}", ex.getStackTrace() );
                    setReceiverState( MulticastConnector.INTERRUPTED );
                    mParent.socketOperationFailed();
                }
                catch ( Exception ex )
                {
                    logger.warn( "receiver threw exception {}", ex.getStackTrace() );
                    setReceiverState( MulticastConnector.INTERRUPTED );
                    mParent.socketOperationFailed();
                }
	    }
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

        private int mState = MulticastConnector.TAKING; // FIXME
        private ConnectorThread mParent;
        private MulticastConnector mDestination;
	private MulticastSocket mSocket;
        private final Logger logger
	    = LoggerFactory.getLogger( "net.mcast.receiver" );
    }


    // ********** UTILITY METHODS ****************

    /**
     * A routine to get the local ip address
     * TODO use this someplace
     *
     * @return
     */
    public List<InetAddress> getLocalIpAddresses()
    {
        List<InetAddress> addresses = new ArrayList<InetAddress>();
        try
        {
            for (Enumeration<NetworkInterface> en = NetworkInterface.getNetworkInterfaces(); en.hasMoreElements();)
            {
                NetworkInterface intf = en.nextElement();
                for (Enumeration<InetAddress> enumIpAddr = intf.getInetAddresses(); enumIpAddr.hasMoreElements();)
                {
                    InetAddress inetAddress = enumIpAddr.nextElement();
                    addresses.add( inetAddress );
                    logger.info( "address: {}", inetAddress );
                }
            }
        }
        catch (SocketException ex)
        {
            logger.error("getLocalIpAddresses: {}", ex.toString());
        }

        return addresses;
    }

	public boolean isBusy() {
	return false;
    }

}
