package edu.vu.isis.ammo.gateway;

import java.util.*;

public class AcknowledgementThresholds {
  
    /**
    * Creates a new data object with default parameters.
    */
    public AcknowledgementThresholds() {
        deviceDelivered = false;
        pluginDelivered = false;
    }

    /**
     * The pushed object was delivered to a remote device
     */
    public boolean deviceDelivered;
    
    /**
     * The pushed object was delivered to another plugin
     */
    public boolean pluginDelivered;
    
}
