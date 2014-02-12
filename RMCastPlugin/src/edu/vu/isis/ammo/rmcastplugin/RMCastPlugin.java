/* Copyright (C) 2010-2013 Vanderbilt University
 *  
 * This software was developed by the Institute for Software Integrated
 * Systems (ISIS) at Vanderbilt University, Tennessee, USA for the
 * Transformative Apps program under DARPA, Contract # HR011-10-C-0175 and
 * Contract # HR0011-12-C-0109.
 *  
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this data, including any software or models in source or binary
 * form, as well as any drawings, specifications, and documentation
 * (collectively "the Data"), to deal in the Data without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Data, and to
 * permit persons to whom the Data is furnished to do so, subject to the
 * following conditions:
 *  
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Data.
 *  
 * THE DATA IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS, SPONSORS, DEVELOPERS, CONTRIBUTORS, OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE DATA OR THE USE OR OTHER DEALINGS IN THE DATA.
 */

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
