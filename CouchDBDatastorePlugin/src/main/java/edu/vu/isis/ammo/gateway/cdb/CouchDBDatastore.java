package edu.vu.isis.ammo.gateway.cdb;

import com.google.common.primitives.Bytes;
import edu.vu.isis.ammo.gateway.PushData;
import org.codehaus.jackson.JsonFactory;
import org.codehaus.jackson.JsonNode;
import org.codehaus.jackson.JsonParseException;
import org.codehaus.jackson.JsonParser;
import org.codehaus.jackson.map.ObjectMapper;
import org.codehaus.jackson.node.JsonNodeFactory;
import org.codehaus.jackson.node.ObjectNode;
import org.ektorp.AttachmentInputStream;
import org.ektorp.CouchDbConnector;
import org.ektorp.CouchDbInstance;
import org.ektorp.http.HttpClient;
import org.ektorp.http.StdHttpClient;
import org.ektorp.impl.StdCouchDbInstance;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.Arrays;

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
        newNode.put("received_time", p.receivedTime);

        try {
            //find the end of the JSON data
            int jsonLength = Bytes.indexOf(p.data, (byte) 0x0);
            if(jsonLength == -1) {
                //no null terminator found, so we'll assume the whole string is valid JSON
                jsonLength = p.data.length;
            }

            JsonParser jsonParser = jsonFactory.createJsonParser(p.data, 0, jsonLength);
            JsonNode node = jsonParser.readValueAsTree();
            newNode.put("data", node);

            dbConnector.create(newNode);

            logger.debug("Item as JSON: {}", newNode);

            //parse out attachments if we need to
            if(jsonLength < p.data.length) {
                if(newNode.has("_rev") && newNode.get("_rev").isTextual()) {
                    final String revisionId = newNode.get("_rev").asText();
                    logger.debug("Revision ID: {}", revisionId);

                    parseBlobs(dbConnector, newNode, p.data, jsonLength);
                } else {
                    logger.warn("CouchDB didn't add a revision ID to this object. It may not have been stored in the DB.");
                }
            }
        } catch(JsonParseException e) {
            logger.error("Failed to parse JSON object", e);
        } catch(IOException e) {
            logger.error("IO Exception while parsing JSON object", e);
        }
    }

    private void parseBlobs(CouchDbConnector dbConnector, ObjectNode newNode, byte[] data, int jsonLength) {
        int position = jsonLength + 1; // move past the null terminator
        final ByteBuffer dataBuff = ByteBuffer.wrap(data).order(ByteOrder.BIG_ENDIAN);

        dataBuff.position(position);
        while (dataBuff.position() < data.length) {
            // get the field name
            final int nameStart = dataBuff.position();
            int nameLength;
            for (nameLength = 0; position < data.length; nameLength++, position++) {
                if (data[position] == 0x0)
                    break;
            }
            final String fieldName = new String(data, nameStart, nameLength);
            position++; // move past the null

            // get the last three bytes of the length, to be used as a simple
            // checksum
            dataBuff.position(position);
            dataBuff.get();
            final byte[] beginningPsuedoChecksum = new byte[3];
            dataBuff.get(beginningPsuedoChecksum);

            // get the blob length for real
            dataBuff.position(position);
            final int dataLength = dataBuff.getInt();

            if (dataLength > dataBuff.remaining()) {
                logger.error("payload size is wrong {} {}",
                        dataLength, data.length);
                return;
            }
            // get the blob data
            final byte[] blob = new byte[dataLength];
            final int blobStart = dataBuff.position();
            System.arraycopy(data, blobStart, blob, 0, dataLength);
            dataBuff.position(blobStart + dataLength);

            // check for storage type
            final byte storageMarker = dataBuff.get();

            // get and compare the beginning and ending checksum
            final byte[] endingPsuedoChecksum = new byte[3];
            dataBuff.get(endingPsuedoChecksum);
            if (!Arrays.equals(endingPsuedoChecksum, beginningPsuedoChecksum)) {
                logger.error("blob checksum mismatch {} {}", endingPsuedoChecksum,
                        beginningPsuedoChecksum);
                break;
            }
            BlobTypeEnum blobType;

            if (storageMarker == BLOB_MARKER_FIELD) {
                blobType = BlobTypeEnum.SMALL;
            } else {
                blobType = BlobTypeEnum.LARGE;
            }

            newNode.put("att_" + fieldName + "_blobType", blobType == BlobTypeEnum.SMALL ? 1:2);
            dbConnector.update(newNode);

            if(newNode.has("_rev") && newNode.get("_rev").isTextual()) {
                final String revisionId = newNode.get("_rev").asText();
                logger.debug("Revision ID: {}", revisionId);

                AttachmentInputStream attachment = new AttachmentInputStream(fieldName, new ByteArrayInputStream(blob), "application/octet-stream");
                dbConnector.createAttachment(newNode.get("_id").getTextValue(), revisionId, attachment);
            } else {
                logger.warn("CouchDB didn't add a revision ID to this object. It may not have been stored in the DB.");
            }
        }
    }

    /**
     * The presence of the BLOB_MARKER_FIELD as the first byte in the footer for
     * a blob data section indicates where the blob should be placed in the
     * content provider.
     */
    public static final byte BLOB_MARKER_FIELD = (byte) 0xff;

    /**
     * The blob type enum makes a distinction between blobs which are small and
     * those which are large. The basic idea is delivery and storage will be
     * performed differently depending upon the relative size of the blob.
     */
    public enum BlobTypeEnum {
        /** blob too large to send via binder */
        LARGE,
        /** blob sufficiently small to send via binder */
        SMALL;

        /**
         * The difficulty here is that the blob may have trailing null bytes.
         * e.g. fieldName = [data], fieldNameBlob= [[100, 97, 116, 97]], blob =
         * [[100, 97, 116, 97, 0]] These should result it a match.
         */
        public static BlobTypeEnum infer(String fieldName, byte[] blob) {

            final byte[] fieldNameBlob;
            try {
                fieldNameBlob = fieldName.getBytes("UTF-8");
            } catch (java.io.UnsupportedEncodingException ex) {
                return SMALL;
            }

            logger.trace(
                    "processing blob fieldName = [{}], fieldNameBlob= [{}], blob = [{}]",
                    fieldName, fieldNameBlob, blob);

            if (blob == null)
                return LARGE;
            if (blob.length < 1)
                return LARGE;

            if (fieldNameBlob.length == blob.length)
                return Arrays.equals(blob, fieldNameBlob) ? LARGE : SMALL;

            if (fieldNameBlob.length > blob.length)
                return SMALL;

            int i;
            for (i = 0; i < fieldNameBlob.length; i++) {
                if (fieldNameBlob[i] != blob[i])
                    return SMALL;
            }
            for (; i < blob.length; i++) {
                if (blob[i] != (byte) 0)
                    return SMALL;
            }
            return LARGE;
        }
    }

}
