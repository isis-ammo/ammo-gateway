package edu.vu.isis.ammo.gateway;

public interface DataPushReceiverListener {
    public void onPushDataReceived(GatewayConnector sender, PushData pushData);
}

