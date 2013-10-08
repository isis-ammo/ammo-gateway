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

public class PullResponse {
    /**
    * Creates a new pull response with default parameters.
    */
    public PullResponse() {
        requestUid = "";
        pluginId = "";
        mimeType = "";
        uri = "";
        encoding = "json";
        data = null;
	priority = 0;
    }

    /**
     * A unique identifier for this pull request.  It will be returned
     * with each item returned by the pull request.
     */
    public String requestUid;
    
    /**
     * The unique identifier for this plugin or connected device.  For
     * devices, this should be the same as the device ID used by
     * associateDevice.
     */
    public String pluginId;
    
    /**
     * The data type to request.  Used to determine which plugin or
     * plugins a request is routed to.
     */
    public String mimeType;
    
    /**
     * The URI of the data in this response.
     */
    public String uri;
    
    /**
     * The encoding of the data in this response (optional; defaults
     * to "json" if not specified).
     */
    public String encoding;
    
    /**
     * The data to be sent to the requestor.
     */
    public byte[] data;


    public int priority;

    /**
     * Convenience method which creates a PullResponse, prepopulating elements
     * which must match the PullRequest which generated the response.  
     * Specifically, this method sets the requestUid, pluginId, and mimeType 
     * to match the values from request.
     * 
     * @param request The PullRequest which the PullResponse should be
     *                generated from.
     * @return A PullResponse, with mandatory fields preset to the correct
     *         values.
     */
    public static PullResponse createFromPullRequest(PullRequest request) {
        PullResponse newResponse = new PullResponse();
        newResponse.requestUid = request.requestUid;
        newResponse.pluginId = request.pluginId;
        newResponse.mimeType = request.mimeType;
        return newResponse;
    }
    
    @Override public String toString() {
        return new String("Response to: " + pluginId + " for request: " + requestUid + " for type " + mimeType);
    }
}
