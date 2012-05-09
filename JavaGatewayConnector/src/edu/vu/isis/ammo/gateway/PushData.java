package edu.vu.isis.ammo.gateway;

import java.util.*;

public class PushData {
  
    /**
    * Creates a new data object with default parameters.
    */
    public PushData() {
        uid = null;
        mimeType = null;
        encoding = new String("json");
        data = null;
        originUserName = null;
        originDevice = null;
        scope = MessageScope.SCOPE_GLOBAL;

	priority = 0;
    }

    /**
     * The UID of this piece of data.  This UID should be a universally
     * unique identifier for the object being pushed (no two pieces of
     * data should have the same UID).
     */
    public String uid;
    
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
    
    @Override public String toString() {
        return new String("Push UID: " + uid + " type: " + mimeType);
    }
}
