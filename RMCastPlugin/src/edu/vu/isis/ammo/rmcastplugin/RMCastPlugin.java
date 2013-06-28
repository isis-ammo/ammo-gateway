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

/**
 * class RMCastPlugin
 **/ 


public class RMCastPlugin 
{
    static ReliableMulticastConnector mMsgRmcastConnector = null;
    static GatewayConnector mMsgGatewayConnector = null;
    static PluginServiceHandler mMsgServiceHandler = null;

    static ReliableMulticastConnector mMediaRmcastConnector = null;
    static GatewayConnector mMediaGatewayConnector = null;
    static PluginServiceHandler mMediaServiceHandler = null;

    private static final Logger logger = LoggerFactory.getLogger(PluginServiceHandler.class);

    public static void main(String[] args)
    {
	// message handler ....
	mMsgServiceHandler = new PluginServiceHandler(false);

	mMsgGatewayConnector = new GatewayConnector( mMsgServiceHandler );
	mMsgServiceHandler.setGatewayConnector( mMsgGatewayConnector );

	mMsgRmcastConnector = new ReliableMulticastConnector(mMsgServiceHandler, "", 1234, "rmcast", "udp.xml", "AmmoGroup");
	mMsgServiceHandler.setRmcastConnector( mMsgRmcastConnector );
	
	// media handler 
	mMediaServiceHandler = new PluginServiceHandler(true);

	mMediaGatewayConnector = new GatewayConnector( mMediaServiceHandler );
	mMediaServiceHandler.setGatewayConnector( mMediaGatewayConnector );

	mMediaRmcastConnector = new ReliableMulticastConnector(mMediaServiceHandler, "", 1234, "mediaRmcast", "udpMedia.xml", "AmmoMedia");
	mMediaServiceHandler.setRmcastConnector( mMediaRmcastConnector );


	while(true) {
	    try {
		Thread.sleep(5);	// idle wait ...
	    } catch (InterruptedException ex) {
		logger.info("Main sleep interrupted ... exitting ... ");
		break;
	    }
	}
    }

}
