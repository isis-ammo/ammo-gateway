package edu.vu.isis.ammo.gateway.cdb;

import edu.vu.isis.ammo.gateway.GatewayConnector;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

/**
 * Created by jwilliams on 2/6/14.
 */
public class Main {
    private static final Logger logger = LoggerFactory.getLogger(Main.class);

    public static void main(String[] args) {
        logger.info("Couch DB Datastore Plugin");

        GatewayHandler handler = new GatewayHandler();
        GatewayConnector connector = new GatewayConnector(handler);

        while(true) {
            try {
                Thread.sleep(5);	// idle wait ...
            } catch (InterruptedException ex) {
                logger.info("Main sleep interrupted ... exitting ... ");
                break;
            }
        }
    }
}
