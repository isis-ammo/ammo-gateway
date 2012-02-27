package edu.vu.isis.ammo.gateway;

public interface PullRequestReceiverListener {
    public void onPullRequestReceived(GatewayConnector sender, PullRequest pullReq);
}


