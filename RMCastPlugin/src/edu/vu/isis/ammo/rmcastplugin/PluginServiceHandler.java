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

import java.util.HashMap;
import java.util.Iterator;
import java.util.List;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.google.protobuf.ByteString;

import edu.vu.isis.ammo.core.pb.AmmoMessages;
import edu.vu.isis.ammo.gateway.AcknowledgementThresholds;
import edu.vu.isis.ammo.gateway.DataPushReceiverListener;
import edu.vu.isis.ammo.gateway.GatewayConnector;
import edu.vu.isis.ammo.gateway.GatewayConnectorDelegate;
import edu.vu.isis.ammo.gateway.MessageScope;
import edu.vu.isis.ammo.gateway.PullRequest;
import edu.vu.isis.ammo.gateway.PullRequestReceiverListener;
import edu.vu.isis.ammo.gateway.PullResponse;
import edu.vu.isis.ammo.gateway.PullResponseReceiverListener;
import edu.vu.isis.ammo.gateway.PushAcknowledgement;
import edu.vu.isis.ammo.gateway.PushData;

/**
 * class PluginServiceHandler
 * - transcode AmmoMessages received from JGroups into GatewayMessages
 * - transcode GatewayMessages received from Gateway into AmmoMessages
 **/ 


class PluginServiceHandler implements
			       GatewayConnectorDelegate,
			       DataPushReceiverListener,
			       PullRequestReceiverListener,
			       PullResponseReceiverListener
{
    private ReliableMulticastConnector mRmcastConnector = null;
    private GatewayConnector mGatewayConnector = null;

    private HashMap<String,Integer> subscriptions = null;

    private static final Logger logger = LoggerFactory.getLogger(PluginServiceHandler.class);

    public PluginServiceHandler()
    {
	subscriptions = new HashMap<String,Integer>();
	// populate the subscriptions map with mimetypes read from config file
	PluginConfigurationManager pConfig = PluginConfigurationManager.getInstance();
	List<String> mimeTypes = pConfig.getMimeTypes();
	for( String mimeType : mimeTypes ) {
	    subscriptions.put(mimeType, 1);
	}
    }

    public void setGatewayConnector(GatewayConnector gatewayConnector)
    {
	mGatewayConnector = gatewayConnector;
    }

    public void setRmcastConnector(ReliableMulticastConnector rmcastConnector)
    {
	mRmcastConnector = rmcastConnector;
    }


    public void onConnect() { }	// from RMCastConnector

    public void onDisconnect() { } // from RMCastConnector

    public void onAmmoMessageReceived( AmmoMessages.MessageWrapper message ) // from RMCastConnector
    {
    	logger.info("received a message: {}", message.getType());
	if (message.getType() == AmmoMessages.MessageWrapper.MessageType.DATA_MESSAGE) { // data message convert to push message
	    PushData pushData = new PushData();
	    AmmoMessages.DataMessage dataMessage = message.getDataMessage();
	    pushData.uri =  dataMessage.getUri();
	    pushData.mimeType = dataMessage.getMimeType();
	    pushData.encoding = dataMessage.getEncoding();
	    pushData.originUserName = dataMessage.getUserId();
	    pushData.originDevice = dataMessage.getOriginDevice();
		pushData.ackThresholds = new AcknowledgementThresholds();
		pushData.ackThresholds.deviceDelivered = dataMessage.getThresholds().getDeviceDelivered();
		pushData.ackThresholds.pluginDelivered = dataMessage.getThresholds().getPluginDelivered();
	    pushData.data = dataMessage.getData().toByteArray();
	    pushData.scope = (AmmoMessages.MessageScope.GLOBAL == dataMessage.getScope()) ?
		MessageScope.SCOPE_GLOBAL :
		MessageScope.SCOPE_LOCAL;
	    mGatewayConnector.pushData(pushData);
	    logger.info("received push message from: {} {}", pushData.originUserName, pushData);
	} else if(message.getType() == AmmoMessages.MessageWrapper.MessageType.PUSH_ACKNOWLEDGEMENT) {
		PushAcknowledgement out = new PushAcknowledgement();
		AmmoMessages.PushAcknowledgement in = message.getPushAcknowledgement();
		out.acknowledgingDevice = in.getAcknowledgingDevice();
		out.acknowledgingUser = in.getAcknowledgingUser();
		out.destinationDevice = in.getDestinationDevice();
		out.destinationUser = in.getDestinationUser();
		out.deviceDelivered = in.getThreshold().getDeviceDelivered();
		out.pluginDelivered = in.getThreshold().getPluginDelivered();
		switch (in.getStatus()) {
		case FAIL: out.status = edu.vu.isis.ammo.gateway.PushStatus.PUSH_FAIL; break;
		case RECEIVED: out.status = edu.vu.isis.ammo.gateway.PushStatus.PUSH_RECEIVED; break;
		case REJECTED: out.status = edu.vu.isis.ammo.gateway.PushStatus.PUSH_RECEIVED; break;
		case SUCCESS: out.status = edu.vu.isis.ammo.gateway.PushStatus.PUSH_SUCCESS; break;
		}
		out.uid = in.getUri();
		mGatewayConnector.pushAcknowledgement(out);
		logger.info("received push ack from: {} {}", out.acknowledgingUser, out);
	} else 	if (false) { // TBD SKN message.getType() == AmmoMessages.MessageWrapper.MessageType.SUBSCRIBE_MESSAGE) {
	    // subscribe message check the sub map to see if we are not already subscribed to this type
	    AmmoMessages.SubscribeMessage subscribeMessage = message.getSubscribeMessage();
	    
	    String mimeType = subscribeMessage.getMimeType();
	    // @todo: handle queries
	    // 	    subData.query = subscribeMessage.getQuery();
	    // @todo: handle scope changes across different subscriptions of the same type
	    MessageScope scope = (AmmoMessages.MessageScope.GLOBAL == subscribeMessage.getScope()) ?
	    	MessageScope.SCOPE_GLOBAL :
	    	MessageScope.SCOPE_LOCAL;

	    if ( subscriptions.containsKey(mimeType) ) {
		int count = subscriptions.get(mimeType);
		subscriptions.put(mimeType, count+1); // increment refcount
	    } else {
		subscriptions.put(mimeType, 1);
		mGatewayConnector.registerDataInterest( mimeType, this, scope); // create a subscription with Gateway
	    }
	} else 	if (false) { // TBD SKN message.getType() == AmmoMessages.MessageWrapper.MessageType.UNSUBSCRIBE_MESSAGE) {
	    // subscribe message check the sub map to see if we are not already subscribed to this type
	    AmmoMessages.UnsubscribeMessage unsubscribeMessage = message.getUnsubscribeMessage();
	    
	    String mimeType = unsubscribeMessage.getMimeType();
	    // @todo: handle scope changes across different subscriptions of the same type
	    MessageScope scope = (AmmoMessages.MessageScope.GLOBAL == unsubscribeMessage.getScope()) ?
	    	MessageScope.SCOPE_GLOBAL :
	    	MessageScope.SCOPE_LOCAL;

	    if ( subscriptions.containsKey(mimeType) ) {
		int count = subscriptions.get(mimeType);
		if (count == 1) {
		    subscriptions.remove(mimeType);
		    mGatewayConnector.unregisterDataInterest(mimeType, scope);
		} else
		    subscriptions.put(mimeType, count-1); // decrement refcount
	    }

	    
	}

    }

    private class SubscribeData
    {
	public MessageScope scope;
	public int refCount;
    }


    @Override
	public void onConnect(GatewayConnector sender) {
	// resend all the subscriptions
	Iterator ki = subscriptions.keySet().iterator();
	while( ki.hasNext() ) {
	    String mimeType = (String)ki.next();
	    sender.registerDataInterest( mimeType, this, MessageScope.SCOPE_GLOBAL ); // TBD - handle the scope properly
	}
	
    }

    @Override
	public void onDisconnect(GatewayConnector sender) {
    }

    @Override
	public void onAuthenticationResponse(GatewayConnector sender, boolean result) {
    }

    @Override
    public void onPushAcknowledgementReceived(GatewayConnector sender, PushAcknowledgement ack) {
		AmmoMessages.PushAcknowledgement.Builder ackMsg =
			AmmoMessages.PushAcknowledgement.newBuilder();
		ackMsg.setUri(ack.uid);
		ackMsg.setDestinationDevice(ack.destinationDevice);
		ackMsg.setDestinationUser(ack.destinationUser);
		ackMsg.setAcknowledgingDevice(ack.acknowledgingDevice);
		ackMsg.setAcknowledgingUser(ack.acknowledgingUser);
		
		AmmoMessages.AcknowledgementThresholds.Builder thresholds = 
			AmmoMessages.AcknowledgementThresholds.newBuilder();
		thresholds.setDeviceDelivered(ack.deviceDelivered);
		thresholds.setPluginDelivered(ack.pluginDelivered);
		ackMsg.setThreshold(thresholds.build());
		
		AmmoMessages.MessageWrapper.Builder msg =
			    AmmoMessages.MessageWrapper.newBuilder();
		msg.setPushAcknowledgement(ackMsg.build());
		msg.setType(AmmoMessages.MessageWrapper.MessageType.PUSH_ACKNOWLEDGEMENT);
		
		mRmcastConnector.sendMessage(msg.build());
    }

    @Override
	public void onPushDataReceived(GatewayConnector sender, PushData pushData)
    {
	
	AmmoMessages.MessageWrapper.Builder msg =
	    AmmoMessages.MessageWrapper.newBuilder();
	AmmoMessages.DataMessage.Builder pushMsg =
	    AmmoMessages.DataMessage.newBuilder();

	pushMsg.setUri( pushData.uri );
	pushMsg.setMimeType( pushData.mimeType );
	pushMsg.setEncoding( pushData.encoding );
	pushMsg.setUserId( pushData.originUserName );
	pushMsg.setOriginDevice( pushData.originDevice );
	pushMsg.setData( ByteString.copyFrom(pushData.data) );
	AmmoMessages.AcknowledgementThresholds.Builder thresholds = 
			AmmoMessages.AcknowledgementThresholds.newBuilder();
	thresholds.setDeviceDelivered(pushData.ackThresholds.deviceDelivered);
	thresholds.setPluginDelivered(pushData.ackThresholds.pluginDelivered);
	pushMsg.setThresholds(thresholds.build());
	pushMsg.setScope( pushData.scope == MessageScope.SCOPE_GLOBAL ? AmmoMessages.MessageScope.GLOBAL : AmmoMessages.MessageScope.LOCAL );

	msg.setType( AmmoMessages.MessageWrapper.MessageType.DATA_MESSAGE );
	msg.setDataMessage( pushMsg.build() );
	// send to rmcastConnector
	mRmcastConnector.sendMessage( msg.build() );
	
    }

    @Override
	public void onPullRequestReceived(GatewayConnector sender, PullRequest pullReq)
    {
	// currently unsupported ...
	
    }

    @Override
	public void onPullResponseReceived(GatewayConnector sender, PullResponse pullResp)
    {
	// currently unsupported ...
    }


}
