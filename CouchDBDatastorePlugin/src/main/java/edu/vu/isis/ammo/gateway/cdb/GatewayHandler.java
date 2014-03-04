package edu.vu.isis.ammo.gateway.cdb;

import edu.vu.isis.ammo.gateway.*;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.util.List;

/**
 * Created by jwilliams on 2/6/14.
 */
public class GatewayHandler implements GatewayConnectorDelegate, DataPushReceiverListener, PullRequestReceiverListener {
    private static final Logger logger = LoggerFactory.getLogger(GatewayHandler.class);

    private final CouchDBDatastore datastore;

    public GatewayHandler() {
        datastore = new CouchDBDatastore();
    }

    @Override
    public void onPushDataReceived(GatewayConnector sender, PushData pushData) {
        logger.debug("Received item {}", pushData.uri);
        datastore.addItem(pushData);
    }

    /**
     * Called when the GatewayConnector connects to the gateway core.
     *
     * @param sender The GatewayConnector object that connected to the gateway.
     */
    @Override
    public void onConnect(GatewayConnector sender) {
        logger.debug("Connected to gateway; subscribing");
        List<String> mimeTypes = PluginConfigurationManager.getInstance().getMimeTypes();

        for(String type : mimeTypes) {
            sender.registerDataInterest(type, this, MessageScope.SCOPE_GLOBAL);
            sender.registerPullInterest(type, this, MessageScope.SCOPE_LOCAL);
        }
    }

    /**
     * Called when the GatewayConnector disconnects from the gateway core.
     *
     * @param sender The GatewayConnector object that disconnect from the gateway.
     */
    @Override
    public void onDisconnect(GatewayConnector sender) {
        logger.debug("Lost connection to gateway");
    }

    /**
     * Delegate method called after an authentication message is processed
     * by the gateway.
     *
     * @param sender The GatewayConnector which received the authentication result.
     * @param result true if authentication succeeded; false if authentication
     */
    @Override
    public void onAuthenticationResponse(GatewayConnector sender, boolean result) {

    }

    /**
     * Delegate method called after a push message acknowledgement is received
     *
     * @param sender The GatewayConnector which received the authentication result.
     * @param ack    The acknowledgement message
     */
    @Override
    public void onPushAcknowledgementReceived(GatewayConnector sender, PushAcknowledgement ack) {

    }

    @Override
    public void onPullRequestReceived(GatewayConnector sender, PullRequest pullReq) {
        datastore.query(sender, pullReq);
    }
}
