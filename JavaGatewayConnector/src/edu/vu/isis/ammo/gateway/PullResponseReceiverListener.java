package edu.vu.isis.ammo.gateway;

public interface PullResponseReceiverListener {
    public void onPullResponseReceived(GatewayConnector sender, PullResponse response);
}
