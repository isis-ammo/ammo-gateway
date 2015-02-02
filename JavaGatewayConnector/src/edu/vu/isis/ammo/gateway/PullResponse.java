/* Copyright (c) 2010-2015 Vanderbilt University
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
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
