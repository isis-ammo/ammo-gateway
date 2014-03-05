package edu.vu.isis.ammo.gateway.cdb;

import com.google.common.base.Splitter;
import com.google.common.primitives.Bytes;
import edu.vu.isis.ammo.gateway.*;
import org.codehaus.jackson.*;
import org.codehaus.jackson.map.ObjectMapper;
import org.codehaus.jackson.node.JsonNodeFactory;
import org.codehaus.jackson.node.ObjectNode;
import org.codehaus.jackson.node.ValueNode;
import org.ektorp.*;
import org.ektorp.http.HttpClient;
import org.ektorp.http.StdHttpClient;
import org.ektorp.impl.StdCouchDbInstance;
import org.json.JSONException;
import org.json.JSONObject;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.Arrays;
import java.util.Iterator;
import java.util.List;

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

    public void query(GatewayConnector sender, PullRequest query) {
        logger.debug("Got query for type {}: {}", query.mimeType, query.query);
        if(query.mimeType.equals("ammo/transapps.chat.message")) {
            //query is JSON-formatted, and looks like:
            /*
            {
                "groups" : [
                    "CMD",
                    "test1",
                    "sup sup",
                    "All",
                    "sup",
                    "hi sup",
                    "testgroup1"
                ],
                "SYNC_UUID" : "afd06aa7-0d5d-48ec-9a59-956c3817c079",
                "latestMessageTime" : "1375375718044",
                "latestMessageUuid" : "c95f6ce0-20dc-40f6-9637-ec5c12a9cf85",
                "username" : "brad"
            }
            */

            try {
                //Parse out query
                final JSONObject jsonQuery = new JSONObject(query.query);

                if(jsonQuery.has("latestMessageUuid")) {
                    //look up timestamp for latest message in DB
                    ViewQuery q = new ViewQuery()
                            .designDocId("_design/views")
                            .viewName("received_time_by_id")
                            .key(jsonQuery.getString("latestMessageUuid"));

                    List<JsonNode> results = dbConnector.queryView(q, JsonNode.class);
                    logger.debug("Timestamp query results: {}", results);
                } else {
                    logger.error("Malformed chat query: {}", jsonQuery);
                }
            } catch(JSONException e) {
                logger.error("Bad JSON query", e);
            }
        } else {
            //parse datastore plugin compatible query
            //uri,user,time_begin,time_end,directed_user
            //negative times are relative to current time (all times in seconds)

            List<String> splitQuery = Splitter.on(",")
                                              .splitToList(query.query);
            if(splitQuery.size() == 5) {
                String queryUri = splitQuery.get(0);
                String queryUser = splitQuery.get(1);
                try {
                    String queryTimeBegin = splitQuery.get(2);
                    String queryTimeEnd = splitQuery.get(3);
                    String queryDirectedUser = splitQuery.get(4);
                    //TODO: handle queries other than time

                    final String queryMimeType;

                    if(queryDirectedUser.equals("")) {
                        queryMimeType = query.mimeType;
                    } else {
                        queryMimeType = query.mimeType + "_" + queryDirectedUser;
                    }

                    final ComplexKey start, end;

                    if(!queryTimeBegin.equals("")) {
                        final long parsedTimeBegin = Long.parseLong(queryTimeBegin);
                        final long actualTimeBegin;
                        final long currentTime = System.currentTimeMillis();
                        if(parsedTimeBegin < 0) { //negative time means relative to current time
                            actualTimeBegin = currentTime + parsedTimeBegin * 1000L;
                        } else {
                            actualTimeBegin = parsedTimeBegin;
                        }
                        logger.debug("Parsed: {} Actual: {} Current: {}", parsedTimeBegin, actualTimeBegin, currentTime);
                        start = ComplexKey.of(queryMimeType, actualTimeBegin);
                    } else {
                        start = ComplexKey.of(queryMimeType, 0L);
                    }

                    if(!queryTimeEnd.equals("")) {
                        final long parsedTimeEnd = Long.parseLong(queryTimeEnd);
                        final long actualTimeEnd;
                        if(parsedTimeEnd < 0) { //negative time means relative to current time
                            actualTimeEnd = System.currentTimeMillis() + parsedTimeEnd * 1000L;
                        } else {
                            actualTimeEnd = parsedTimeEnd;
                        }
                        end = ComplexKey.of(queryMimeType, actualTimeEnd);
                    } else {
                        end = ComplexKey.of(queryMimeType, ComplexKey.emptyObject());
                    }

                    ViewQuery q = new ViewQuery()
                            .designDocId("_design/views")
                            .viewName("by_type_and_time")
                            .startKey(start)
                            .endKey(end);

                    List<JsonNode> results = dbConnector.queryView(q, JsonNode.class);
                    logger.debug("Query results: {}", results);

                    for(JsonNode node : results) {
                        sendResponseFromJson(node, query.requestUid, query.pluginId, sender);
                    }

                } catch(NumberFormatException e) {
                    logger.error("Invalid number in time field");
                }
            } else {
                logger.error("Malformed query ({}): not enough parameters", query);
            }
        }
    }

    private void sendResponseFromJson(JsonNode n, String requestId, String pluginId, GatewayConnector gatewayConnector) {
        final PullResponse response = new PullResponse();

        response.requestUid = requestId;
        response.pluginId = pluginId;
        response.mimeType = n.get("mime_type").asText();
        response.uri = n.get("_id").asText();
        response.encoding = n.get("encoding").asText();
        response.priority = MessagePriority.PRIORITY_CTRL.getValue();

        ByteArrayOutputStream dataStream = new ByteArrayOutputStream();

        try {
            JsonGenerator g = jsonFactory.createJsonGenerator(dataStream, JsonEncoding.UTF8);
            g.writeTree(n.get("data"));



            boolean firstAttachment = true;

            //Serialize out attachments (blobs/files)
            for(Iterator<String> fieldNameIt = n.getFieldNames(); fieldNameIt.hasNext(); ) {
                String fieldName = fieldNameIt.next();
                if(fieldName.startsWith("att_") && fieldName.endsWith("_blobType")) {
                    int firstUnderscore = fieldName.indexOf('_');
                    int lastUnderscore = fieldName.lastIndexOf('_');
                    String attachmentName = fieldName.substring(firstUnderscore + 1, lastUnderscore);
                    logger.debug("Field names: {}", attachmentName);

                    //get the attachment from the DB
                    AttachmentInputStream attachment = dbConnector.getAttachment(n.get("_id").asText(), attachmentName);
                    logger.debug("Attachment: size {}", attachment.getContentLength());

                    if(firstAttachment) {
                        //JSON component is only null terminated if there are attachments
                        dataStream.write(0x0);
                    }
                    dataStream.write(attachmentName.getBytes());
                    dataStream.write(0x0);

                    ByteBuffer fieldSizeBuffer = ByteBuffer.allocate(4);
                    fieldSizeBuffer.order(ByteOrder.BIG_ENDIAN);
                    fieldSizeBuffer.putInt((int) attachment.getContentLength());

                    dataStream.write(fieldSizeBuffer.array());
                    final byte[] attachmentData = new byte[(int)attachment.getContentLength()];
                    attachment.read(attachmentData);
                    dataStream.write(attachmentData);

                    //write storage marker
                    if(n.get(fieldName).asInt() == 1) { //BlobTypeEnum.SHORT; needs storage marker
                        dataStream.write(BLOB_MARKER_FIELD);
                    } else {
                        dataStream.write(0);
                    }
                    //write remaining three bytes of size
                    dataStream.write(fieldSizeBuffer.array(), 1, 3);
                }
            }

            response.data = dataStream.toByteArray();
            gatewayConnector.pullResponse(response);

        } catch(IOException e) {
            logger.error("Error serializing JSON data", e);
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
