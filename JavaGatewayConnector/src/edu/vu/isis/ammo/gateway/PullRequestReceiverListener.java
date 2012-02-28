package edu.vu.isis.ammo.gateway;

/**
* Listener interface for pull requests.  Plugins implement this interface to receive
* pull requests sent with pullRequest.
*/
public interface PullRequestReceiverListener {
    public void onPullRequestReceived(GatewayConnector sender, PullRequest pullReq);
}


