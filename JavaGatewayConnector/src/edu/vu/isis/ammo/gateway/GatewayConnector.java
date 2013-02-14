package edu.vu.isis.ammo.gateway;

import ammo.gateway.protocol.GatewayPrivateMessages;

import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.ArrayList;
import java.util.Map;
import com.google.protobuf.ByteString;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;


public class GatewayConnector {
    private static final Logger logger = LoggerFactory.getLogger(GatewayConnector.class);

    /**
     * Creates a new GatewayConnector with the given GatewayConnectorDelegate and
     * establishes a connection to the gateway core.
     *
     * @param delegate A GatewayConnectorDelegate object to be used by this
     *                 GatewayConnector instance.  May be NULL (no delegate methods
     *                 will be called).
     */
    public GatewayConnector(GatewayConnectorDelegate delegate) {
	this.delegate = delegate;
	this.connector = null;
	init(delegate, GatewayConfigurationManager.getInstance() );
    }

    /**
      * Creates a new GatewayConnector with the given GatewayConnectorDelegate and
      * establishes a connection to the gateway core.
      *
      * @param delegate A GatewayConnectorDelegate object to be used by this
      *                 GatewayConnector instance.  May be NULL (no delegate methods
      *                 will be called).
      * @param configfile A path to the gateway config file.
      */
    public GatewayConnector(GatewayConnectorDelegate delegate, String configFile) {
	this.delegate = delegate;
	this.connector = null;
	init(delegate, GatewayConfigurationManager.getInstance(configFile) );
    }
    

      /**
      * Destroys a GatewayConnector.
      */
    public void finalize() {
	connector.close();
	connector = null;
    }


      //General connection negotiation and bookkeeping
      /**
      * Associates a device with this GatewayConnector in the gateway core.  This
      * method should only be needed by device connector plugins (e.g. the Android
      * Gateway Plugin).
      * 
      * @todo Authentication is not yet implemented in the gateway--  this method
      *       will send an authentication request, but the gateway will always
      *       will always return 'success' (and this method will always return
      *       true).  We should actually perform authentication here (pending more
      *       information from the security people).
      * 
      * @param device The unique ID of the device connecting to the gateway
      * @param user The unique ID of the user associated with the device connecting
      *             to the gateway.
      * @param key The authentication key/password for this user and device. Content
      *            of this parameter is still to be determined.
      * 
      * @return true if authentication was successful; false if authentication
      *         failed.
      */
    public boolean associateDevice(String device, String user, String key) {
	GatewayPrivateMessages.GatewayWrapper.Builder msg =
	    GatewayPrivateMessages.GatewayWrapper.newBuilder();
	GatewayPrivateMessages.AssociateDevice.Builder associateMsg =
	    GatewayPrivateMessages.AssociateDevice.newBuilder();
	associateMsg.setDevice(device);
	associateMsg.setUser(user);
	associateMsg.setKey(key);

	msg.setAssociateDevice(associateMsg.build());
	msg.setType(GatewayPrivateMessages.GatewayWrapper.MessageType.ASSOCIATE_DEVICE);
	msg.setMessagePriority( (int) MessagePriority.PRIORITY_AUTH.getValue());
	if (connector.isConnected()) {
	    connector.sendMessage(msg.build());
	    return true;
	} else {
	    // plugin not connected to gateway
	    return false;
	}
    }

      //--Data-Push support methods--
    
      //Sender-side
      /**
       * Pushes a piece of data (with a particular URI and type) to the gateway.
       * 
       * Data pushed with this method can be received by listeners registered with
       * registerDataInterest.  All listeners registered for the type specified by
       * mimeType will receive this piece of data.
       * 
       * @param pushData The data to be pushed to the gateway.  uri and mimeType
       *                 must be set, or this call must fail (other parameters
       *                 are optional, and will use sane defaults).
       * 
       * @return true if the operation succeeded; false if the operation failed.
       */
    public boolean pushData(PushData pushData) {
	GatewayPrivateMessages.GatewayWrapper.Builder msg =
	    GatewayPrivateMessages.GatewayWrapper.newBuilder();
	GatewayPrivateMessages.PushData.Builder pushMsg =
	    GatewayPrivateMessages.PushData.newBuilder();
	pushMsg.setUri(pushData.uri);
	pushMsg.setMimeType(pushData.mimeType);
	pushMsg.setEncoding(pushData.encoding);
	pushMsg.setOriginUser(pushData.originUserName);
	pushMsg.setOriginDevice(pushData.originDevice);
	pushMsg.setData( ByteString.copyFrom(pushData.data) );
	pushMsg.setScope( (pushData.scope == MessageScope.SCOPE_LOCAL) ?
			  GatewayPrivateMessages.MessageScope.LOCAL :
			  GatewayPrivateMessages.MessageScope.GLOBAL );

	GatewayPrivateMessages.AcknowledgementThresholds.Builder ackThresholds =
	    GatewayPrivateMessages.AcknowledgementThresholds.newBuilder();
	ackThresholds.setDeviceDelivered(pushData.ackThresholds.deviceDelivered);
	ackThresholds.setPluginDelivered(pushData.ackThresholds.pluginDelivered);

	pushMsg.setThresholds(ackThresholds.build());

	msg.setPushData(pushMsg.build());
	msg.setType(GatewayPrivateMessages.GatewayWrapper.MessageType.PUSH_DATA);
	msg.setMessagePriority((int)pushData.priority);
	if (connector.isConnected()) {
	    connector.sendMessage(msg.build());
	    return true;
	} else {
	    logger.error("pushData: data sent while not connected to gateway, dropped ...");
	    return false;
	}
    }
    
      /**
       * Pushes an acknowledgement for a received push message
       * 
       * 
       * @param pushAckData The push acknowledgement data to be pushed via the gateway
       * 
       * @return true if the operation succeeded; false if the operation failed.
       */
    public boolean pushAcknowledgement(PushAcknowledgement pushAckData) {
	GatewayPrivateMessages.GatewayWrapper.Builder msg =
	    GatewayPrivateMessages.GatewayWrapper.newBuilder();
	GatewayPrivateMessages.PushAcknowledgement.Builder pushAck =
	    GatewayPrivateMessages.PushAcknowledgement.newBuilder();
	pushAck.setUid(pushAckData.uid);
	pushAck.setDestinationDevice(pushAckData.destinationDevice);
	pushAck.setAcknowledgingDevice(pushAckData.acknowledgingDevice);
	pushAck.setDestinationUser(pushAckData.destinationUser);
	pushAck.setAcknowledgingUser(pushAckData.acknowledgingUser);

	GatewayPrivateMessages.AcknowledgementThresholds.Builder ackThresholds =
	    GatewayPrivateMessages.AcknowledgementThresholds.newBuilder();
	ackThresholds.setDeviceDelivered(pushAckData.deviceDelivered);
	ackThresholds.setPluginDelivered(pushAckData.pluginDelivered);

	pushAck.setThreshold(ackThresholds.build());

	GatewayPrivateMessages.PushAcknowledgement.PushStatus status =
	    GatewayPrivateMessages.PushAcknowledgement.PushStatus.RECEIVED;
	switch(pushAckData.status) {
	case PUSH_RECEIVED:
	    status = GatewayPrivateMessages.PushAcknowledgement.PushStatus.RECEIVED;
	    break;
	case PUSH_SUCCESS:
	    status = GatewayPrivateMessages.PushAcknowledgement.PushStatus.SUCCESS;
	    break;
	case PUSH_FAIL:
	    status = GatewayPrivateMessages.PushAcknowledgement.PushStatus.FAIL;
	    break;
	case PUSH_REJECTED:
	    status = GatewayPrivateMessages.PushAcknowledgement.PushStatus.REJECTED;
	    break;
	}
	pushAck.setStatus(status);
	
	msg.setPushAcknowledgement(pushAck.build());
	msg.setType(GatewayPrivateMessages.GatewayWrapper.MessageType.PUSH_ACKNOWLEDGEMENT);
	msg.setMessagePriority((int)MessagePriority.PRIORITY_CTRL.getValue());
	if (connector.isConnected()) {
	    connector.sendMessage(msg.build());
	    return true;
	} else {
	    logger.error("pushAck: ack sent while not connected to gateway, dropped ...");
	    return false;
	}
    }
    
      /**
       * Requests data from a gateway plugin or device (which claims it can handle a
       * pull request of a particular type).
       *
       * @param request The pull request to send to the gateway.  This call will fail
       *                if requestUid, pluginId, and mimeType are not set; other fields are
       *                optional and will use default values if not explicitly set.
       *
       * @return true if the operation succeeded; false if the operation failed.
       */
    public boolean pullRequest(PullRequest request) {
	GatewayPrivateMessages.GatewayWrapper.Builder msg =
	    GatewayPrivateMessages.GatewayWrapper.newBuilder();
	GatewayPrivateMessages.PullRequest.Builder pullMsg =
	    GatewayPrivateMessages.PullRequest.newBuilder();

	pullMsg.setRequestUid(request.requestUid).
	    setPluginId(request.pluginId).
	    setMimeType(request.mimeType).
	    setQuery(request.query).
	    setProjection(request.projection).
	    setMaxResults(request.maxResults).
	    setStartFromCount(request.startFromCount).
	    setLiveQuery(request.liveQuery).
	    setScope( request.scope == MessageScope.SCOPE_LOCAL ?
		      GatewayPrivateMessages.MessageScope.LOCAL :
		      GatewayPrivateMessages.MessageScope.GLOBAL );

	msg.setPullRequest(pullMsg.build());
	msg.setType(GatewayPrivateMessages.GatewayWrapper.MessageType.PULL_REQUEST);
	msg.setMessagePriority((int)request.priority);
	if (connector.isConnected()) {
	    connector.sendMessage(msg.build());
	    return true;
	} else {
	    logger.error("pullRequest: request sent while not connected to gateway, dropped ...");
	    return false;
	}

    }
    
      /** 
       * Sends a response to a pull request.  Used by plugins with a registered
       * pull request handler.
       *
       * @param response The pull response to send back to the requesting plugin.
       *                 The requestUid, pluginId, and mimeType fields must be
       *                 set, and must match the corresponding fields from the
       *                 pull request which triggered this response.
       *
       * @return true if the operation succeeded; false if the operation failed.
       */
    public boolean pullResponse(PullResponse response) {
	GatewayPrivateMessages.GatewayWrapper.Builder msg =
	    GatewayPrivateMessages.GatewayWrapper.newBuilder();
	GatewayPrivateMessages.PullResponse.Builder pullResp =
	    GatewayPrivateMessages.PullResponse.newBuilder();
	pullResp.setRequestUid(response.requestUid);
	pullResp.setPluginId(response.pluginId);
	pullResp.setMimeType(response.mimeType);
	pullResp.setUri(response.uri);
	pullResp.setEncoding(response.encoding);
	pullResp.setData( ByteString.copyFrom(response.data) );

	msg.setPullResponse(pullResp.build());
	msg.setType(GatewayPrivateMessages.GatewayWrapper.MessageType.PULL_RESPONSE);
	msg.setMessagePriority((int)response.priority);
	
	if (connector.isConnected()) {
	    connector.sendMessage(msg.build() );
	    return true;
	} else {
	    logger.error("pullResponse: response sent while not connected to gateway, dropped ...");
	    return false;
	}
    }
    
    
      //Receiver-side
      /**
      * Registers interest in pushed data of type mime_type.  Data will be pushed
      * to the DataPushReceiverListener specified by listener.
      *
      * @param mime_type The type of data to listen for.
      * @param listener  The DataPushReceiverListener object which will be called
      *                  when data is available.
      * @param scope     The scope of this registration.  SCOPE_LOCAL will only
      *                  deliver data originating from the connected gateway;
      *                  SCOPE_GLOBAL will deliver data originating from all
      *                  connected gateways (subject to each data object's scope
      *                  parameter).
      *
      * @return true if the operation succeeded; false if the operation failed.
      */
    public boolean registerDataInterest(String mime_type, DataPushReceiverListener listener, MessageScope scope) {
	logger.info("registerDataInterest: mime {}, listener {}", mime_type, listener );
	
	GatewayPrivateMessages.GatewayWrapper.Builder msg =
	    GatewayPrivateMessages.GatewayWrapper.newBuilder();
	GatewayPrivateMessages.RegisterDataInterest.Builder di =
	    GatewayPrivateMessages.RegisterDataInterest.newBuilder();
	di.setMimeType(mime_type);
	di.setScope( scope == MessageScope.SCOPE_LOCAL ?
		     GatewayPrivateMessages.MessageScope.LOCAL :
		     GatewayPrivateMessages.MessageScope.GLOBAL );

	msg.setRegisterDataInterest(di.build());
	msg.setType(GatewayPrivateMessages.GatewayWrapper.MessageType.REGISTER_DATA_INTEREST);
	msg.setMessagePriority((int)MessagePriority.PRIORITY_CTRL.getValue());

	receiverListeners.put(mime_type, listener);

	if (connector.isConnected()) {
	    connector.sendMessage(msg.build() );
	    return true;
	} else {
	    logger.error("registerDataInterest: regeisterDataInterest sent while not connected to gateway, dropped ...");
	    return false;
	}
    }
      
      /**
      * Unregisters interest in pushed data of type mime_type.  Will unregister all
      * listeners associated with this plugin if this plugin has registered multiple
      * listeners for the same type.
      * 
      * @param mime_type The type of data to unregister interest in.
      * @param scope     The scope of the registration to unregister interest
      *                  in; should be the same as the original registration.
      * 
      * @return true if the operation succeeded; false if the operation failed.
      */
    public boolean unregisterDataInterest(String mime_type, MessageScope scope) {
	GatewayPrivateMessages.GatewayWrapper.Builder msg =
	    GatewayPrivateMessages.GatewayWrapper.newBuilder();
	GatewayPrivateMessages.UnregisterDataInterest.Builder di =
	    GatewayPrivateMessages.UnregisterDataInterest.newBuilder();
	di.setMimeType(mime_type);
	di.setScope( scope == MessageScope.SCOPE_LOCAL ?
		     GatewayPrivateMessages.MessageScope.LOCAL :
		     GatewayPrivateMessages.MessageScope.GLOBAL );

	msg.setUnregisterDataInterest(di.build());
	msg.setType(GatewayPrivateMessages.GatewayWrapper.MessageType.UNREGISTER_DATA_INTEREST);
	msg.setMessagePriority((int)MessagePriority.PRIORITY_CTRL.getValue());

	receiverListeners.remove(mime_type);

	if (connector.isConnected()) {
	    connector.sendMessage(msg.build());
	    return true;
	} else {
	    logger.error("unregisterDataInterest:  sent while not connected to gateway, dropped ...");
	    return false;
	}

    }
      
      /**
      * Registers a pull request handler for the specified data type.
      * 
      * @param mime_type The type of data to listen for pull requests for.
      * @param listener  The PullRequestReceiverListener object which will be called
      *                  when a new pull request is received.
      *
      * @return true if the operation succeeded; false if the operation failed.
      */
    public boolean registerPullInterest(String mime_type, PullRequestReceiverListener listener, MessageScope scope) {
	GatewayPrivateMessages.GatewayWrapper.Builder msg =
	    GatewayPrivateMessages.GatewayWrapper.newBuilder();
	GatewayPrivateMessages.RegisterPullInterest.Builder di =
	    GatewayPrivateMessages.RegisterPullInterest.newBuilder();
	di.setMimeType(mime_type);
	di.setScope( scope == MessageScope.SCOPE_LOCAL ?
		     GatewayPrivateMessages.MessageScope.LOCAL :
		     GatewayPrivateMessages.MessageScope.GLOBAL );

	msg.setRegisterPullInterest(di.build());
	msg.setType(GatewayPrivateMessages.GatewayWrapper.MessageType.REGISTER_PULL_INTEREST);
	msg.setMessagePriority((int)MessagePriority.PRIORITY_CTRL.getValue());

	pullRequestListeners.put(mime_type, listener);

	if (connector.isConnected()) {
	    connector.sendMessage(msg.build() );
	    return true;
	} else {
	    logger.error("registerPullInterest:  sent while not connected to gateway, dropped ...");
	    return false;
	}
    }
      
      /**
      * Unregisters interest in pull requests for the specified data type.  Will
      * unregister all listeners associated with this plugin if this plugin has 
      * registered multiple listeners for the same type.
      * 
      * @param mime_type The type of data to unregister interest in.
      * 
      * @return true if the operation succeeded; false if the operation failed.
      */
    public boolean unregisterPullInterest(String mime_type, MessageScope scope) {
	GatewayPrivateMessages.GatewayWrapper.Builder msg =
	    GatewayPrivateMessages.GatewayWrapper.newBuilder();
	GatewayPrivateMessages.UnregisterPullInterest.Builder di =
	    GatewayPrivateMessages.UnregisterPullInterest.newBuilder();
	di.setMimeType(mime_type);
	di.setScope( scope == MessageScope.SCOPE_LOCAL ?
		     GatewayPrivateMessages.MessageScope.LOCAL :
		     GatewayPrivateMessages.MessageScope.GLOBAL );

	msg.setUnregisterPullInterest(di.build());
	msg.setType(GatewayPrivateMessages.GatewayWrapper.MessageType.UNREGISTER_PULL_INTEREST);
	msg.setMessagePriority((int)MessagePriority.PRIORITY_CTRL.getValue());

	pullRequestListeners.remove(mime_type);

	if (connector.isConnected()) {
	    connector.sendMessage(msg.build() );
	    return true;
	} else {
	    logger.error("unregisterPullInterest:  sent while not connected to gateway, dropped ...");
	    return false;
	}
	
    }
    
      /**
      * Registers a listener to be called when data is received as a response from a
      * pull request.  Should be called in conjunction with pullRequest when
      * initiating a pull request; does not have to be called more than once if more
      * than one pull request will be made for the same mime type.
      * 
      * @param mime_type The type of data to listen for pull responses for.
      * @param listener  The PullResponseReceiverListener object which will be
      *                  called when a new pull response is received.
      * 
      * @return true if the operation succeeded; false if the operation failed.
      */
    public boolean registerPullResponseInterest(String mime_type, PullResponseReceiverListener listener) {
	pullResponseListeners.put(mime_type, listener);
	return true;
    }
      
      /**
      * Unregisters interest in pull responses for the specified data type.  Will
      * unresgister all listeners associated with this plugin if this plugin has
      * registered multiple listeners for the same data type.
      * 
      * @param mime_type The type of data to unregister interest in.
      * 
      * @return true if the operation succeeded; false if the operation failed.
      */
    public boolean unregisterPullResponseInterest(String mime_type) {
	pullResponseListeners.remove(mime_type);
	return true;
    }
      
    private void init(GatewayConnectorDelegate delegate, GatewayConfigurationManager config) {
	receiverListeners = new HashMap<String, DataPushReceiverListener>();
	pullRequestListeners = new HashMap<String, PullRequestReceiverListener>();
	pullResponseListeners = new HashMap<String, PullResponseReceiverListener>();

	connector = new NetworkConnector(this, config.getGatewayAddress(), config.getGatewayPort());
    }

    private List<DataPushReceiverListener> getListenersForType(String mimeType) {
	List<DataPushReceiverListener> match = new ArrayList<DataPushReceiverListener>();
	Iterator it = receiverListeners.entrySet().iterator();
	while( it.hasNext() ) {
	    Map.Entry pair = (Map.Entry)it.next();
	    if ( mimeType.startsWith( (String)pair.getKey() ) ) {
		match.add( (DataPushReceiverListener)pair.getValue() );
	    }
	}
	return match;
    }
    
    protected void onAssociateResultReceived(final GatewayPrivateMessages.AssociateResult msg) {
	if (delegate != null) {
	    try {
	        delegate.onAuthenticationResponse(this, msg.getResult() == GatewayPrivateMessages.AssociateResult.Status.SUCCESS );
	    } catch(Exception ex) {
	        logger.error("Authentication Response handler threw exception", ex);
	    }
	}
    }

    protected void onPushDataReceived(final GatewayPrivateMessages.PushData msg, final int messagePriority) {
	String mimeType = msg.getMimeType();
	List<DataPushReceiverListener> listeners = getListenersForType( mimeType );
	for( DataPushReceiverListener listener : listeners ) {
	    logger.info("onPushDataReceived: mime {}, listener {}", mimeType, listener);
	    PushData pushData = new PushData();
	    pushData.uri = msg.getUri();
	    pushData.mimeType = mimeType;
	    pushData.encoding = msg.getEncoding();
	    pushData.originUserName = msg.getOriginUser();
	    pushData.originDevice = msg.getOriginDevice();
	    pushData.data = msg.getData().toByteArray();
	    pushData.priority = messagePriority;
	    pushData.ackThresholds = new AcknowledgementThresholds();
	    pushData.ackThresholds.deviceDelivered = msg.getThresholds().getDeviceDelivered();
	    pushData.ackThresholds.pluginDelivered = msg.getThresholds().getPluginDelivered();
	    try {
	        listener.onPushDataReceived(this, pushData);
	    } catch(Exception ex) {
	        logger.error("Push Data handler threw exception", ex);
	    }
	    // TODO: Auto-generate Acks here ...
	}
    }

    protected void onPushAcknowledgementReceived(final GatewayPrivateMessages.PushAcknowledgement msg) {
	PushAcknowledgement pushAck = new PushAcknowledgement();
	pushAck.uid = msg.getUid();
	pushAck.destinationDevice = msg.getDestinationDevice();
	pushAck.acknowledgingDevice = msg.getAcknowledgingDevice();
	pushAck.destinationUser = msg.getDestinationUser();
	pushAck.acknowledgingUser = msg.getAcknowledgingUser();
	pushAck.deviceDelivered = msg.getThreshold().getDeviceDelivered();
	pushAck.pluginDelivered = msg.getThreshold().getPluginDelivered();
	
	switch(msg.getStatus()) {
	case RECEIVED:
	    pushAck.status = PushStatus.PUSH_RECEIVED;
	    break;

	case SUCCESS:
	    pushAck.status = PushStatus.PUSH_SUCCESS;
	    break;

	case FAIL:
	    pushAck.status = PushStatus.PUSH_FAIL;
	    break;

	case REJECTED:
	    pushAck.status = PushStatus.PUSH_REJECTED;
	    break;
	}

	if (delegate != null) {
	    try {
	        delegate.onPushAcknowledgementReceived(this, pushAck);
	    } catch(Exception ex) {
	        logger.error("Push Acknowledgement handler threw exception", ex);
	    }
	}
    }


    protected void onPullRequestReceived(final GatewayPrivateMessages.PullRequest msg, final int messagePriority) {
	String mimeType = msg.getMimeType();
	PullRequestReceiverListener listener = pullRequestListeners.get( mimeType );
	logger.info("onPullRequestReceived: mime {}, listener {}", mimeType, listener);
	if (listener != null) {
	    PullRequest req = new PullRequest();
	    req.requestUid = msg.getRequestUid();
	    req.pluginId = msg.getPluginId();
	    req.mimeType = mimeType;
	    req.query = msg.getQuery();
	    req.projection = msg.getProjection();
	    req.startFromCount = msg.getStartFromCount();
	    req.liveQuery = msg.getLiveQuery();
	    req.priority = messagePriority;
	    try {
	        listener.onPullRequestReceived(this, req);
	    } catch(Exception ex) {
	        logger.error("Pull Request handler threw exception", ex);
	    }
	}
    }

    protected void onPullResponseReceived(final GatewayPrivateMessages.PullResponse msg, final int messagePriority) {
	String mimeType = msg.getMimeType();
	PullResponseReceiverListener listener = pullResponseListeners.get( mimeType );
	logger.info("onPullResponseReceived: mime {}, listener {}", mimeType, listener);
	if (listener != null) {
	    PullResponse resp = new PullResponse();
	    resp.requestUid = msg.getRequestUid();
	    resp.pluginId = msg.getPluginId();
	    resp.mimeType = msg.getMimeType();
	    resp.uri = msg.getUri();
	    resp.encoding = msg.getEncoding();
	    resp.data = msg.getData().toByteArray();
	    resp.priority = messagePriority;
	    try {
	        listener.onPullResponseReceived(this, resp);
	    } catch(Exception ex) {
	        logger.error("Pull Response handler threw exception", ex);
	    }
	}
    }

    void onConnect() {
	if (delegate != null) {
	    try {
	        delegate.onConnect(this);
	    } catch(Exception ex) {
	        logger.error("On Connect handler threw exception", ex);
	    }
	}
    }

    void onDisconnect() {
	if (delegate != null) {
	    try {
	        delegate.onDisconnect(this);
	    } catch(Exception ex) {
	        logger.error("On Disconnect handler threw exception", ex);
	    }
	}
    }
      
    private GatewayConnectorDelegate delegate;

    private HashMap<String, DataPushReceiverListener> receiverListeners;
    private HashMap<String, PullRequestReceiverListener> pullRequestListeners;
    private HashMap<String, PullResponseReceiverListener> pullResponseListeners;
    
    private NetworkConnector connector;
}
