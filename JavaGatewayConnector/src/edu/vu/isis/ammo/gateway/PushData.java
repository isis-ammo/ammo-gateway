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

import java.util.*;

public class PushData {
  
    /**
    * Creates a new data object with default parameters.
    */
    public PushData() {
        uri = "";
        mimeType = "";
        encoding = "json";
        data = null;
        originUserName = "";
        originDevice = "";
        scope = MessageScope.SCOPE_GLOBAL;

	      priority = 0;
	      
	      ackThresholds = new AcknowledgementThresholds();

        receivedTime = 0;
    }

    /**
     * The URI of this piece of data.  This URI should be a universally
     * unique identifier for the object being pushed (no two pieces of
     * data should have the same URI).
     */
    public String uri;
    
    /**
     * The MIME type of this piece of data.  This MIME type is used to
     * determine which other gateway plugins will receive this pushed
     * data.
     */
    public String mimeType;
    
    /**
     * The encoding of the data in this message (optional; defaults
     * to "json" if not specified).
     */
    public String encoding;
    
    /**
     * The data to be pushed to the gateway.  Can contain any arbitrary binary
     * data.
     */
    public byte[] data;
    
    /**
     * The username of the user who generated this data.  May be
     * overwritten by the gateway in some cases.  Optional.
     */
    public String originUserName;
    
    /**
     * The username of the user who generated this data.  May be
     * overwritten by the gateway in some cases.  Optional.
     */
    public String originDevice;
    
    /**
     * The scope of this object (determines how many gateways to send
     * this object to in a multiple gateway configuration).  Optional,
     * will default to SCOPE_GLOBAL.
     */
    public MessageScope scope;

    /** The priority of this object.  Objects with a higher priority
     *  will be pushed to the device before objects with a lower priority,
     *  if messages are queued.
     */
    public int priority;

    /**
     * The desired level of acknowledgement threshold - 
     *  - deviceDelivered : the pushed data was delivered to a device
     *  - pluginDelivered : the pushed data was delivered to a plugin
     */
    public AcknowledgementThresholds ackThresholds;

    /**
     * The time when this message entered the gateway network, in milliseconds
     * since the POSIX epoch (00:00:00 1/1/1970 UTC).  If zero, this will be
     * automatically populated when pushData() is called.
     */
    public long receivedTime;
    
    @Override public String toString() {
        return new String("Push URI: " + uri + " type: " + mimeType);
    }
}
