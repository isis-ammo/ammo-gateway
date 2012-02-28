package edu.vu.isis.ammo.gateway;

/**
* The delegate interface for a GatewayConnector.  Plugins implement the methods in
* this interface to implement behaviors associated with lifecycle events of the
* gateway connector, such as connection and disconnection.
*/
public interface GatewayConnectorDelegate {
    /**
     * Called when the GatewayConnector connects to the gateway core.
     * 
     * @param sender The GatewayConnector object that connected to the gateway.
     */
    public void onConnect(GatewayConnector sender);
    
    /**
      * Called when the GatewayConnector disconnects from the gateway core.
      * 
      * @param sender The GatewayConnector object that disconnect from the gateway.
      */
    public void onDisconnect(GatewayConnector sender);
    
    /**
      * Delegate method called after an authentication message is processed
      * by the gateway.
      * 
      * @param sender The GatewayConnector which received the authentication result.
      * @param result true if authentication succeeded; false if authentication
      *               failed.
      */
    public void onAuthenticationResponse(GatewayConnector sender, boolean result);
}

