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


class RMCastPlugin 
{
    static ReliableMulticastConnector mRmcastConnector = null;
    static GatewayConnector mGatewayConnector = null;
    static PluginServiceHandler mServiceHandler = null;

    private static final Logger logger = LoggerFactory.getLogger(PluginServiceHandler.class);

    public static void main(String[] args)
    {
	mServiceHandler = new PluginServiceHandler();

	mGatewayConnector = new GatewayConnector( mServiceHandler );
    mServiceHandler.setGatewayConnector( mGatewayConnector );

    mRmcastConnector = new ReliableMulticastConnector(mServiceHandler, "", 1234);
	mServiceHandler.setRmcastConnector( mRmcastConnector );
	
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