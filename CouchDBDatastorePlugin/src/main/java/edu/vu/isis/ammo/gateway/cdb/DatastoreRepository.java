package edu.vu.isis.ammo.gateway.cdb;

import com.google.common.base.Splitter;
import org.codehaus.jackson.JsonNode;
import org.ektorp.ComplexKey;
import org.ektorp.CouchDbConnector;
import org.ektorp.ViewQuery;
import org.ektorp.support.CouchDbRepositorySupport;
import org.ektorp.support.View;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.util.ArrayList;
import java.util.List;

/**
 * Created by jwilliams on 3/13/14.
 */
@View( name = "all", map = "function(doc) {emit( null, doc._id )}")
public class DatastoreRepository extends CouchDbRepositorySupport<JsonNode> {
    private static final Logger logger = LoggerFactory.getLogger(DatastoreRepository.class);

    protected DatastoreRepository(Class<JsonNode> type, CouchDbConnector db) {
        super(type, db, "views");
        this.initStandardDesignDocument();
    }

    @View( name = "by_type_and_time",
            map = "function(doc) {\n" +
                  "  if(doc.mime_type && doc.received_time) {\n" +
                  "    emit([doc.mime_type, doc.received_time], doc);\n" +
                  "  }\n" +
                  "}")
    public List<JsonNode> getByDatastoreQuery(String mimeType, String query) {
        //parse datastore plugin compatible query
        //uri,user,time_begin,time_end,directed_user
        //negative times are relative to current time (all times in seconds)

        List<String> splitQuery = Splitter.on(",")
                .splitToList(query);
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
                    queryMimeType = mimeType;
                } else {
                    queryMimeType = mimeType + "_" + queryDirectedUser;
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

                return db.queryView(q, JsonNode.class);
            } catch(NumberFormatException e) {
                logger.error("Invalid number in time field");
                return new ArrayList<JsonNode>();
            }
        } else {
            logger.error("Malformed query ({}): not enough parameters", query);
            return new ArrayList<JsonNode>();
        }
    }
}
