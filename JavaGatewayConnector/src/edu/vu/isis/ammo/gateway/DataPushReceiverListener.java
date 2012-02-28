package edu.vu.isis.ammo.gateway;

/**
* Listener interface for pushed data.  Plugins subclass this class to receive data
* pushed with pushData.
*/
public interface DataPushReceiverListener {
    public void onPushDataReceived(GatewayConnector sender, PushData pushData);
}

