package edu.vu.isis.ammo.gateway;

import java.util.*;

public class PushAcknowledgement {
  
    /**
    * Creates a new data object with default parameters.
    */
    public PushAcknowledgement() {
        uri = "";
        destinationDevice = "";
	acknowledgingDevice = "";
	destinationUser = "";
        acknowledgingUser = "";
	deviceDelivered = false;
	pluginDelivered = false;
	status = PushStatus.PUSH_RECEIVED;
    }

    public String uri;

    public String destinationDevice;

    public String acknowledgingDevice;

    public String destinationUser;

    public String acknowledgingUser;

    public boolean deviceDelivered;

    public boolean pluginDelivered;

    public PushStatus status;

    
    @Override public String toString() {
        return new String("Push Acknowledment: " + uri );
    }
}
