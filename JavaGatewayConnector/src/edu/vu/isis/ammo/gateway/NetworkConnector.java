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
package edu.vu.isis.ammo.gateway;

import java.io.IOException;
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
import java.util.concurrent.atomic.AtomicLong;
import java.util.concurrent.atomic.AtomicReference;
import java.util.zip.CRC32;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import ammo.gateway.protocol.GatewayPrivateMessages;


/**
 * Two long running threads and one short.
 * The long threads are for sending and receiving messages.
 * The short thread is to connect the socket.
 * The sent messages are placed into a queue if the socket is connected.
 *
 */
class NetworkConnector {
    private static final Logger logger = LoggerFactory.getLogger(NetworkConnector.class);

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
    private static final int TCP_RECV_BUFF_SIZE = 0x15554; // the maximum receive buffer size
    private static final int MAX_MESSAGE_SIZE = 0x100000;  // arbitrary max size

    private SocketChannel mSocketChannel;

    // Three threads
    private ConnectorThread connectorThread; // long running
    private SenderThread mSender;	     // created after connection
    private ReceiverThread mReceiver;	     // created after connection

    private String gatewayHost = null;
    private int gatewayPort = -1;

    private ByteOrder endian = ByteOrder.LITTLE_ENDIAN;

    private final SenderQueue mSenderQueue;
    private final GatewayConnector mGatewayConnector;

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
    static final byte[] GATEWAY_MESSAGE_MAGICB = { (byte)0xde, (byte)0xad, (byte)0xbe, (byte)0xef };
    

    public NetworkConnector(GatewayConnector gatewayConnector, String gatewayHost, int gatewayPort) {
	logger.info("Thread <{}>NetworkConnector::<constructor>", Thread.currentThread().getId());
	this.mGatewayConnector = gatewayConnector;
	this.gatewayHost = gatewayHost;
	this.gatewayPort = gatewayPort;

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
            .append("socket: host[").append(this.gatewayHost).append("] ")
            .append("port[").append(this.gatewayPort).append("]").toString();
    }

    /**
     * forces a reconnection.
     */
    // Called by ReceiverThread to send an incoming message to the
    // appropriate destination.
    private boolean deliverMessage( GatewayPrivateMessages.GatewayWrapper message )
    {
	logger.info( "deliverMessage() {} ", message );
	// hand it off to parentconnector
	if (message.getType() == GatewayPrivateMessages.GatewayWrapper.MessageType.ASSOCIATE_RESULT) {
	    logger.debug("Received Associate Result ...");
	    mGatewayConnector.onAssociateResultReceived(message.getAssociateResult() );
	} else if (message.getType() == GatewayPrivateMessages.GatewayWrapper.MessageType.PUSH_DATA) {
	    logger.debug("Received Push Data ...");
	    mGatewayConnector.onPushDataReceived(message.getPushData() );
	} else if (message.getType() == GatewayPrivateMessages.GatewayWrapper.MessageType.PULL_REQUEST) {
	    logger.debug("Received Pull Request ...");
	    mGatewayConnector.onPullRequestReceived(message.getPullRequest() );
	} else if (message.getType() == GatewayPrivateMessages.GatewayWrapper.MessageType.PULL_RESPONSE) {
	    logger.debug("Received Pull Response ...");
	    mGatewayConnector.onPullResponseReceived(message.getPullResponse() );
	}

	return true;
    }

    /**
     * do your best to send the message.
     * This makes use of the blocking "put" call.
     * A proper producer-consumer should use put or add and not offer.
     * "put" is blocking call.
     * If this were on the UI thread then offer would be used.
     *
     * @param agm GatewayPrivateMessages.GatewayWrapper
     * @return
     */
    public boolean sendMessage( GatewayPrivateMessages.GatewayWrapper agm )
    {
        return mSenderQueue.put( agm );
    }

    ///////////////////////////////////////////////////////////////////////////
    //
    class SenderQueue
    {
	class PriorityComparator implements Comparator<GatewayPrivateMessages.GatewayWrapper> {
	    @Override
		public int  compare(GatewayPrivateMessages.GatewayWrapper x, GatewayPrivateMessages.GatewayWrapper y)
	    {
		if (x.getMessagePriority() > y.getMessagePriority() )
		    return 1;
		else if (x.getMessagePriority() == y.getMessagePriority() )
		    return 0;

		return -1;
	    }
	    
	}

        public SenderQueue( NetworkConnector iChannel )
        {
            mChannel = iChannel;
            mDistQueue = new PriorityBlockingQueue<GatewayPrivateMessages.GatewayWrapper>( 20, new PriorityComparator() );
        }

        public boolean put(GatewayPrivateMessages.GatewayWrapper iMessage )
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

        public GatewayPrivateMessages.GatewayWrapper take() throws InterruptedException
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
            // GatewayPrivateMessages.GatewayWrapper msg = mDistQueue.poll();
            // while ( msg != null )
	    // 	{
	    // 	    if ( msg.handler != null )
	    // 		mChannel.ackToHandler( msg.handler, ChannelDisposal.PENDING );
	    // 	    msg = mDistQueue.poll();
	    // 	}
        }

        private BlockingQueue<GatewayPrivateMessages.GatewayWrapper> mDistQueue;
        private NetworkConnector mChannel;
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
	private final Logger logger = LoggerFactory.getLogger( ConnectorThread.class );

	private final String DEFAULT_HOST = "127.0.0.1";
	private final int DEFAULT_PORT = 12475;
	private final int GATEWAY_RETRY_TIME = 20 * 1000; // 20 seconds

	private NetworkConnector parent;
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


	private ConnectorThread(NetworkConnector parent) {
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
                    case NetworkConnector.DISCONNECTED:
			if ( !this.connect() ) {
			    try {
				Thread.sleep(GATEWAY_RETRY_TIME);
			    } catch (InterruptedException ex) {
				logger.info("sleep interrupted - intentional disable, exiting thread ...");
			    }
			}
			break;

                    case NetworkConnector.CONNECTED:
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

            // Resolve the hostname to an IP address.
            String host = (parent.gatewayHost != null) ? parent.gatewayHost : DEFAULT_HOST;
            int port =  (parent.gatewayPort > 10) ? parent.gatewayPort : DEFAULT_PORT;
            InetAddress ipaddr = null;
            try	{
		ipaddr = InetAddress.getByName( host );
	    } catch ( UnknownHostException e ) {
		logger.warn( "could not resolve host name" );
		return false;
	    }

            // Create the SocketChannel.
            InetSocketAddress sockAddr = new InetSocketAddress( ipaddr, port );
            try {
		if ( parent.mSocketChannel != null )
		    logger.error( "Tried to create mSocketChannel when we already had one." );
		parent.mSocketChannel = SocketChannel.open( sockAddr );
		@SuppressWarnings("unused")
		    boolean result = parent.mSocketChannel.finishConnect();
	    } catch ( AsynchronousCloseException ex ) {
                logger.warn( "connection to {}:{} {} async close failure: {}",
                             new Object[]{ipaddr, port, 
                                          ex.getLocalizedMessage()});
                parent.mSocketChannel = null;
                return false;
            } catch ( ClosedChannelException ex ) {
                logger.warn( "connection to {}:{} {} closed channel failure: {}",
                             new Object[]{ipaddr, port, 
                                          ex.getLocalizedMessage()});
                parent.mSocketChannel = null;
                return false;
            } catch ( Exception e ) {
		logger.warn( "connection to {}:{} {} failed: {}",
			     new Object[]{ipaddr, port, 
					  e.getClass().getName(),
					  e.getLocalizedMessage()});
		parent.mSocketChannel = null;
		return false;
	    }

            logger.info( "connection to {}:{} established ", ipaddr, port );

            mIsConnected.set( true );
	    state.set(NetworkConnector.CONNECTED);
	    parent.mGatewayConnector.onConnect(); // send on connect message

            // Create the sending thread.
            if ( parent.mSender != null )
                logger.error( "Tried to create Sender when we already had one." );
            parent.mSender = new SenderThread( this,
                                               parent,
                                               parent.mSenderQueue,
                                               parent.mSocketChannel );
            parent.mSender.start();

            // Create the receiving thread.
            if ( parent.mReceiver != null )
                logger.error( "Tried to create Receiver when we already had one." );
            parent.mReceiver = new ReceiverThread( this, parent, parent.mSocketChannel );
            parent.mReceiver.start();

            return true;
        }


        private boolean disconnect()
        {
	    // disconnect should only be called from sender/receiver or app thread 
            logger.info( "Thread <{}>ConnectorThread::disconnect",
                         Thread.currentThread().getId() );
	    boolean ret = true;
            try {
		mIsConnected.set( false );
		parent.mGatewayConnector.onDisconnect();

		if ( mSender != null ) {
		    logger.debug( "interrupting SenderThread" );
		    mSender.interrupt();
		}
		if ( mReceiver != null ) {
		    logger.debug( "interrupting ReceiverThread" );
		    mReceiver.interrupt();
		}

		mSenderQueue.reset();

		if ( parent.mSocketChannel != null ) {
		    Socket s = parent.mSocketChannel.socket();
		    if ( s != null ) {
			logger.debug( "Closing underlying socket." );
			s.close();
			logger.debug( "Done" );
		    } else {
			logger.debug( "SocketChannel had no underlying socket!" );
		    }
		    logger.info( "Closing SocketChannel..." );
		    parent.mSocketChannel.close();
		    parent.mSocketChannel = null;
		}

		parent.mSender = null;
		parent.mReceiver = null;
	    } catch ( IOException e ) {
		logger.error( "Caught IOException" );
		// Do this here, too, since if we exited early because
		// of an exception, we want to make sure that we're in
		// an unauthorized state.
		ret = false;
	    }
            logger.debug( "returning after successful disconnect()." );

	    // we need to pend on the sender / receiver thread before we attempt a reconnect
	    try {
		if (mSender != null && Thread.currentThread().getId() != mSender.getId() )
		    mSender.join();
	    } catch (java.lang.InterruptedException ex) {
		logger.warn("disconnect: interrupted exception while waiting for sender thread to die");
	    }
	    try {
		if (mReceiver != null && Thread.currentThread().getId() != mReceiver.getId() )
		    mReceiver.join();
	    } catch (java.lang.InterruptedException ex) {
		logger.warn("disconnect: interrupted exception while waiting for receiver thread to die");
	    }

	    // setting the state to disconnected will cause the connector thread to attempt a reconnect
	    state.set(NetworkConnector.DISCONNECTED);
            return ret;
        }
    }


    ///////////////////////////////////////////////////////////////////////////
    //
    class SenderThread extends Thread
    {
        public SenderThread( ConnectorThread iParent,
                             NetworkConnector iChannel,
                             SenderQueue iQueue,
                             SocketChannel iSocketChannel )
        {
            mParent = iParent;
            mChannel = iChannel;
            mQueue = iQueue;
            mSocketChannel = iSocketChannel;
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
	    ByteBuffer hbuf = ByteBuffer.allocate( GATEWAY_HEADER_SIZE );
	    if (hbuf == null) {
		logger.error("failed to allocate memory for header byte buffer: {}", GATEWAY_HEADER_SIZE); 
	    }
	    hbuf.order(endian);

            while ( mState != NetworkConnector.INTERRUPTED ) {
		GatewayPrivateMessages.GatewayWrapper msg = null;
		try	{
		    setSenderState( NetworkConnector.TAKING );
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
		    ByteBuffer pbuf = ByteBuffer.wrap(payload);
		    pbuf.position( payloadSize );
		    pbuf.flip();

		    hbuf.clear(); // prepare buffer for writing
		    hbuf.putInt( GATEWAY_MESSAGE_MAGIC );
		    hbuf.putInt(payloadSize);
		    hbuf.put( (byte)msg.getMessagePriority() );
		    hbuf.put( (byte)0);
		    hbuf.put( (byte)0);
		    hbuf.put( (byte)0);
		    // payload checksum
		    CRC32 crc32Payload = new CRC32();
		    crc32Payload.update(payload);
		    hbuf.putInt( (int)crc32Payload.getValue() );
		    // header checksum
		    CRC32 crc32Header = new CRC32();
		    crc32Header.update( hbuf.array(), 0, GATEWAY_HEADER_SIZE - 4 );
		    hbuf.putInt( (int)crc32Header.getValue() );
		    hbuf.flip();

		    setSenderState( NetworkConnector.SENDING );
		    @SuppressWarnings("unused")
			long bytesWritten = mSocketChannel.write( new ByteBuffer[]{hbuf, pbuf} );
		    logger.info( "Wrote {} bytes to SocketChannel", bytesWritten );

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

        private int mState = NetworkConnector.TAKING;
        private ConnectorThread mParent;
        private NetworkConnector mChannel;
        private SenderQueue mQueue;
        private SocketChannel mSocketChannel;
        private final Logger logger = LoggerFactory.getLogger( "net.tcp.sender" );
    }


    ///////////////////////////////////////////////////////////////////////////
    //
    class ReceiverThread extends Thread
    {
        public ReceiverThread( ConnectorThread iParent,
                               NetworkConnector iDestination,
                               SocketChannel iSocketChannel )
        {
            mParent = iParent;
            mDestination = iDestination;
            mSocketChannel = iSocketChannel;
        }

	class Message {
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
		    if (bbuf.get() != GATEWAY_MESSAGE_MAGICB[1]) continue;
		    if (bbuf.get() != GATEWAY_MESSAGE_MAGICB[0]) continue;
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
		logger.error("extractHeader: {} \n {}", ex.getMessage(), ex.getStackTrace() );
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

        /**
         * Block on reading from the SocketChannel until we get some data.
         * Then examine the buffer to see if we have any complete packets.
         * If we have an error, notify our parent and go into an error state.
         */
        @Override
	    public void run()
        {
			long headerSuccess = 0;
            logger.info( "Thread <{}>::run()", Thread.currentThread().getId() );


            ByteBuffer bbuf = ByteBuffer.allocate( TCP_RECV_BUFF_SIZE );
            bbuf.order( endian ); // mParent.endian
	    bbuf.clear();
	    int readState = 0;	// HEADER, 1 = PAYLOAD
	    Message message = null;
	    
            while ( mState != NetworkConnector.INTERRUPTED ) {
                try {
                    int bytesRead =  mSocketChannel.read( bbuf );
                    logger.info( "SocketChannel read bytes={}", bytesRead );
		    
		    if (bytesRead < 0)
			break;

                    if (bytesRead == 0 && bbuf.remaining() == 0)
			continue; // no bytes to process

                    // prepare to drain buffer
                    bbuf.flip();

		    eatloop:while (bbuf.remaining() > 0) { // while there is data - eat it from buffer, before reading more
			switch(readState) {
			case 0:	// HEADER
			    message = extractHeader(bbuf);
				if (message == null)
					break eatloop; // I'm still hungry!  Gimme more bytes from the network!
			    readState = 1; // found a good header continue reading

			case 1:	// PAYLOAD
			    readState = extractMessage(bbuf, message);
			    if (readState == 0) { // done with message - dispatch it
				GatewayPrivateMessages.GatewayWrapper agm =
				    GatewayPrivateMessages.GatewayWrapper.parseFrom(message.payload);
				logger.info( "received a message {}", agm.getType() );
				mDestination.deliverMessage( agm );
			    }			    
			    break;
			}
		    }

                    bbuf.compact();
                } catch ( Exception ex ) {
                    logger.warn("receiver threw exception {}", ex.getStackTrace());
		    break;
                }
            }
	    logger.error("exiting receiver thread ...");
	    mParent.socketOperationFailed();
        }

        private void setReceiverState( int iState )
        {
            synchronized ( this ) {
		mState = iState;
	    }
        }

        public synchronized int getReceiverState() { return mState; }

        private int mState = NetworkConnector.TAKING; // FIXME
        private ConnectorThread mParent;
        private NetworkConnector mDestination;
        private SocketChannel mSocketChannel;
        private final Logger logger
	    = LoggerFactory.getLogger( "net.tcp.receiver" );
    }


    // ********** UTILITY METHODS ****************

    /**
     * A routine to get the local ip address
     * TODO use this someplace
     *
     * @return
     */
    public String getLocalIpAddress() {
        logger.trace("Thread <{}>::getLocalIpAddress", Thread.currentThread().getId());
        try {
            for (Enumeration<NetworkInterface> en = NetworkInterface.getNetworkInterfaces(); en.hasMoreElements();) {
                NetworkInterface intf = en.nextElement();
                for (Enumeration<InetAddress> enumIpAddr = intf.getInetAddresses(); enumIpAddr.hasMoreElements();) {
                    InetAddress inetAddress = enumIpAddr.nextElement();
                    if (!inetAddress.isLoopbackAddress()) {
                        return inetAddress.getHostAddress().toString();
                    }
                }
            }
        } catch (SocketException ex) {
            logger.error( ex.toString());
        }
        return null;
    }

	public boolean isBusy() {
	return false;
    }

}
