package edu.vu.isis.ammo.gateway;

import java.util.*;

public class PullResponse {
    public PullResponse() {
	requestUid = null;
	pluginId = null;
	mimeType = null;
	uri = null;
	encoding = new String("json");
	data = null;
    }


    public String requestUid;      ///< A unique identifier for this pull request.  It will be returned
                                   ///  with each item returned by the pull request.
    public String pluginId;        ///< The unique identifier for this plugin or connected device.  For
                                   ///  devices, this should be the same as the device ID used by
                                   ///  associateDevice.
    public String mimeType;        ///< The data type to request.  Used to determine which plugin or
                                   ///  plugins a request is routed to.
    public String uri;             ///< The URI of the data in this response.
    public String encoding;        ///< The encoding of the data in this response (optional; defaults
                                   ///  to "json" if not specified).
    public byte[] data;            ///< The data to be sent to the requestor.

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
