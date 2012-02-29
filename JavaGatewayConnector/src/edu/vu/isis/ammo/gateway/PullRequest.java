package edu.vu.isis.ammo.gateway;

import java.util.*;

public class PullRequest {
    /**
    * Creates a new pull request with default parameters.
    */
    public PullRequest() {
      requestUid = null;
      pluginId = null;
      mimeType = null;
      query = null;
      projection = null;
      maxResults = 0;
      startFromCount = 0;
      liveQuery = false;
      scope = MessageScope.SCOPE_LOCAL;
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

    
    @Override public String toString() {
        return new String("Pull: " + requestUid + " from: " + pluginId + " for type " + mimeType + " query: " + query);
    }
}
