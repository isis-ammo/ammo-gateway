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

public class PullRequest {
    /**
    * Creates a new pull request with default parameters.
    */
    public PullRequest() {
      requestUid = "";
      pluginId = "";
      mimeType = "";
      query = "";
      projection = "";
      maxResults = 0;
      startFromCount = 0;
      liveQuery = false;
      scope = MessageScope.SCOPE_LOCAL;
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
     * The query for this pull request.  Its format is defined by the
     * plugin handling the request.
     */
    public String query;
    
    /** 
     * An application-specific transformation to be applied to the results.  Optional.
     */
    public String projection;
    
    /**
     * The maxiumum number of results to return from this pull request.  Optional.
     */
    public int maxResults;
    
    /**
     * An offset specifying the result to begin returning from (when a
     * subset of results is returned).  Optional.
     */
    public int startFromCount;
    
    /**
     * Specifies a live query--  results are returned continously as they
     * become available.  The exact behavior of this option is defined
     * by the plugin handling the request.  Optional.
     */
    public boolean liveQuery;
    
    /**
     * The scope of this object (determines how many gateways to send
     * this object to in a multiple gateway configuration).  Optional,
     * will default to SCOPE_LOCAL.
     */
    public MessageScope scope;

    public int priority;
    
    @Override public String toString() {
        return new String("Pull: " + requestUid + " from: " + pluginId + " for type " + mimeType + " query: " + query);
    }
}
