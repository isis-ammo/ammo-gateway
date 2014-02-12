package edu.vu.isis.ammo.gateway.cdb;

import edu.vu.isis.ammo.gateway.PushData;
import org.codehaus.jackson.JsonFactory;
import org.codehaus.jackson.JsonNode;
import org.codehaus.jackson.JsonParseException;
import org.codehaus.jackson.JsonParser;
import org.codehaus.jackson.map.ObjectMapper;
import org.codehaus.jackson.node.JsonNodeFactory;
import org.codehaus.jackson.node.ObjectNode;
import org.ektorp.CouchDbConnector;
import org.ektorp.CouchDbInstance;
import org.ektorp.http.HttpClient;
import org.ektorp.http.StdHttpClient;
import org.ektorp.impl.StdCouchDbInstance;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.IOException;

/**
 * Created by jwilliams on 2/6/14.
 */
public class CouchDBDatastore {
    private static final Logger logger = LoggerFactory.getLogger(CouchDBDatastore.class);

    private static String DATABASE_NAME = "ammo_datastore";

    private final HttpClient httpClient;
    private final CouchDbInstance dbInstance;
    private final CouchDbConnector dbConnector;

    private final JsonFactory jsonFactory;

    public CouchDBDatastore() {
        logger.info("CouchDBDatastore created; creating DB connector");
        httpClient = new StdHttpClient.Builder().build();
        dbInstance = new StdCouchDbInstance(httpClient);
        dbConnector = dbInstance.createConnector(DATABASE_NAME, true);
        logger.info("DB connector created");

        jsonFactory = new JsonFactory(new ObjectMapper());
    }

    public void addItem(PushData p) {
        logger.info("Adding item {} to datastore", p.uri);
        ObjectNode newNode = JsonNodeFactory.instance.objectNode();

        newNode.put("_id", p.uri);
        newNode.put("mime_type", p.mimeType);
        newNode.put("encoding", p.encoding);
        newNode.put("origin_user", p.originUserName);
        newNode.put("origin_device", p.originDevice);

        try {
            //assume we don't have a blob or file attachment
            JsonParser jsonParser = jsonFactory.createJsonParser(p.data);
            JsonNode node = jsonParser.readValueAsTree();
            newNode.put("data", node);

            dbConnector.create(newNode);

            logger.debug("Item as JSON: {}", newNode);
        } catch(JsonParseException e) {
            logger.error("Failed to parse JSON object", e);
        } catch(IOException e) {
            logger.error("IO Exception while parsing JSON object", e);
        }
    }

}
