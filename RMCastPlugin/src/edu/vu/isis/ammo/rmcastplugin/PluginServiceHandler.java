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

import edu.vu.isis.ammo.gateway.*;
import edu.vu.isis.ammo.core.pb.AmmoMessages;
import com.google.protobuf.ByteString;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.util.HashMap;
import java.util.Iterator;

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
	if (message.getType() == AmmoMessages.MessageWrapper.MessageType.DATA_MESSAGE) { // data message convert to push message
	    PushData pushData = new PushData();
	    AmmoMessages.DataMessage dataMessage = message.getDataMessage();
	    pushData.uid =  dataMessage.getUid();
	    pushData.mimeType = dataMessage.getMimeType();
	    pushData.encoding = dataMessage.getEncoding();
	    pushData.data = dataMessage.getData().toByteArray();
	    pushData.scope = (AmmoMessages.MessageScope.GLOBAL == dataMessage.getScope()) ?
		MessageScope.SCOPE_GLOBAL :
		MessageScope.SCOPE_LOCAL;
	    mGatewayConnector.pushData(pushData);
	} else 	if (message.getType() == AmmoMessages.MessageWrapper.MessageType.SUBSCRIBE_MESSAGE) {
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
	} else 	if (message.getType() == AmmoMessages.MessageWrapper.MessageType.UNSUBSCRIBE_MESSAGE) {
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
	    mGatewayConnector.registerDataInterest( mimeType, this, MessageScope.SCOPE_GLOBAL ); // TBD - handle the scope properly
	}
	
    }

    @Override
	public void onDisconnect(GatewayConnector sender) {
    }

    @Override
	public void onAuthenticationResponse(GatewayConnector sender, boolean result) {
    }


    @Override
	public void onPushDataReceived(GatewayConnector sender, PushData pushData)
    {
	
	AmmoMessages.MessageWrapper.Builder msg =
	    AmmoMessages.MessageWrapper.newBuilder();
	AmmoMessages.DataMessage.Builder pushMsg =
	    AmmoMessages.DataMessage.newBuilder();

	pushMsg.setUid( pushData.uid );
	pushMsg.setMimeType( pushData.mimeType );
	pushMsg.setEncoding( pushData.encoding );
	pushMsg.setData( ByteString.copyFrom(pushData.data) );
	pushMsg.setScope( pushData.scope == MessageScope.SCOPE_GLOBAL ? AmmoMessages.MessageScope.GLOBAL : AmmoMessages.MessageScope.LOCAL );

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