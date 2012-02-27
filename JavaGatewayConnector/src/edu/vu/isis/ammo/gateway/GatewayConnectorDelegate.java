package edu.vu.isis.ammo.gateway;

public interface GatewayConnectorDelegate {
    public void onConnect(GatewayConnector sender);
    public void onDisconnect(GatewayConnector sender);
    public void onAuthenticationResponse(GatewayConnector sender, boolean result);
}

