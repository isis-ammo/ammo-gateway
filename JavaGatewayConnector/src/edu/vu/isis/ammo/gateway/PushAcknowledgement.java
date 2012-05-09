package edu.vu.isis.ammo.gateway;

import java.util.*;

public class PushAcknowledgement {
  
    /**
    * Creates a new data object with default parameters.
    */
    public PushAcknowledgement() {
        uid = null;
        destinationDevice = null;
	acknowledgingDevice = null;
	destinationUser = null;
        acknowledgingUser = null;
	deviceDelivered = false;
	pluginDelivered = false;
	status = PushStatus.PUSH_RECEIVED;
    }

    String uid;

    String destinationDevice;

    String acknowledgingDevice;

    String destinationUser;

    String acknowledgingUser;

    boolean deviceDelivered;

    boolean pluginDelivered;

    PushStatus status;

    
    @Override public String toString() {
        return new String("Push Acknowledment: " + uid );
    }
}
