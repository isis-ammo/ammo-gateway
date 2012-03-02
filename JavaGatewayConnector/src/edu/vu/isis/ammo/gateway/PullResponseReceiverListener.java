package edu.vu.isis.ammo.gateway;

/**
* Listener interface for pull responses.  Plugins implement this interface to receive
* responses from pull requests.
*/     
public interface PullResponseReceiverListener {
    public void onPullResponseReceived(GatewayConnector sender, PullResponse response);
}
