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

package edu.vu.isis.ammo.gateway;

/**
* The delegate interface for a GatewayConnector.  Plugins implement the methods in
* this interface to implement behaviors associated with lifecycle events of the
* gateway connector, such as connection and disconnection.
*/
public interface GatewayConnectorDelegate {
    /**
     * Called when the GatewayConnector connects to the gateway core.
     * 
     * @param sender The GatewayConnector object that connected to the gateway.
     */
    public void onConnect(GatewayConnector sender);
    
    /**
      * Called when the GatewayConnector disconnects from the gateway core.
      * 
      * @param sender The GatewayConnector object that disconnect from the gateway.
      */
    public void onDisconnect(GatewayConnector sender);
    
    /**
      * Delegate method called after an authentication message is processed
      * by the gateway.
      * 
      * @param sender The GatewayConnector which received the authentication result.
      * @param result true if authentication succeeded; false if authentication
      *               failed.
      */
    public void onAuthenticationResponse(GatewayConnector sender, boolean result);


    /**
      * Delegate method called after a push message acknowledgement is received
      * 
      * @param sender The GatewayConnector which received the authentication result.
      * @param ack    The acknowledgement message
      */
    public void onPushAcknowledgementReceived(GatewayConnector sender, PushAcknowledgement ack);
}

