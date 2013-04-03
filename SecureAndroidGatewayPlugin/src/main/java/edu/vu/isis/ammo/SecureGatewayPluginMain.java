
package edu.vu.isis.ammo;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;


import edu.vu.isis.ammo.core.pb.AmmoMessages.MessageWrapper;
import edu.vu.isis.ammo.gateway.GatewayConnector;
import edu.vu.isis.ammo.gateway.GatewayConnectorDelegate;

public class SecureGatewayPluginMain implements GatewayConnectorDelegate {
    private static final Logger logger = LoggerFactory.getLogger(SecureGatewayPluginMain.class);

    public static void main(String[] args) {
        logger.error("Hello world");
        
        MessageWrapper.Builder msg = MessageWrapper.newBuilder();
        
        GatewayConnector g = new GatewayConnector(new SecureGatewayPluginMain());
    }
}
