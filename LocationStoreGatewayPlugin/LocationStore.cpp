#include <vector>

#include <sqlite3.h>

#include "ace/OS_NS_sys_time.h"

#include "log.h"

#include "LocationStore.h"
#include "QueryStatementBuilder.h"

using namespace ammo::gateway;

LocationStoreReceiver::LocationStoreReceiver (void)
  : db_ (0),
    err_prefix_ ("LocationStoreReceiver - data push: ")
{
  sqlite3_open ("LocationStore_db.sql3", &db_);
	
  LOG_DEBUG ("Opening location store database...");

  const char *create_tbl_str =
	"CREATE TABLE IF NOT EXISTS the_table ("
	"uri TEXT,"
	"mime_type TEXT,"
	"origin_user TEXT,"
	"tv_sec INTEGER NOT NULL,"
	"tv_usec INTEGER,"
	"data BLOB)";
	
  char *db_err = 0;
	
  sqlite3_exec (db_, create_tbl_str, 0, 0, &db_err);
	
  if (db_err != 0)
	{
	  LOG_ERROR ("Error creating location store database table - "
				 << db_err);
	}
}

LocationStoreReceiver::~LocationStoreReceiver (void)
{
  LOG_DEBUG ("Closing location store database...");
	
  sqlite3_close (db_);
}

void LocationStoreReceiver::onConnect (GatewayConnector * /* sender */)
{
}

void LocationStoreReceiver::onDisconnect (GatewayConnector * /* sender */)
{
}

void LocationStoreReceiver::onPushDataReceived (GatewayConnector * /* sender */,
										    ammo::gateway::PushData &pushData)
{
  LOG_DEBUG("Received " << pushData);
  ACE_Time_Value tv (ACE_OS::gettimeofday ());
  sqlite3_stmt *stmt;
	
  int status =
	sqlite3_prepare (db_,
					 "insert into the_table values (?,?,?,?,?,?)",
					 -1,
					 &stmt,
					 0);
	
  if (status != SQLITE_OK)
    {
	  LOG_ERROR (err_prefix_
				 << "prep of sqlite statement failed: "
		         << sqlite3_errmsg (db_));
		
	  return;
	}
	
  status =
	sqlite3_bind_text (stmt,
					   1,
					   pushData.uri.c_str (),
					   pushData.uri.length (),
					   SQLITE_STATIC);
	
  if (status != SQLITE_OK)
    {
	  LOG_ERROR (err_prefix_
				 << "URI bind failed: "
		         << sqlite3_errmsg (db_));
		
	  return;
	}
	
  status =
	sqlite3_bind_text (stmt,
					   2,
					   pushData.mimeType.c_str (),
					   pushData.mimeType.length (),
					   SQLITE_STATIC);
	
  if (status != SQLITE_OK)
    {
	  LOG_ERROR (err_prefix_
				 << "MIME type bind failed: "
		         << sqlite3_errmsg (db_));
		
	  return;
	}
	
  status =
	sqlite3_bind_text (stmt,
					   3,
					   pushData.originUsername.c_str (),
					   pushData.originUsername.length (),
					   SQLITE_STATIC);
	
  if (status != SQLITE_OK)
    {
	  LOG_ERROR (err_prefix_
				 << "origin user bind failed: "
		         << sqlite3_errmsg (db_));
		
		return;
	}
	
  status =
	sqlite3_bind_int (stmt,
					  4,
					  tv.sec ());
	
  if (status != SQLITE_OK)
    {
      LOG_ERROR (err_prefix_
				 << "timestamp sec bind failed: "
		         << sqlite3_errmsg (db_));
		
	  return;
	}
	
  status =
	sqlite3_bind_int (stmt,
					  5,
					  tv.usec ());
	
  if (status != SQLITE_OK)
    {
	  LOG_ERROR (err_prefix_
				 << "timestamp usec bind failed: "
		         << sqlite3_errmsg (db_));
		
	  return;
	}
	
  status =
	sqlite3_bind_blob (stmt,
					   6,
					   pushData.data.get_allocator ().address (*(pushData.data.begin ())),
					   pushData.data.size (),
					   SQLITE_STATIC);
	
  if (status != SQLITE_OK)
    {
	  LOG_ERROR (err_prefix_
				 << "data bind failed: "
		         << sqlite3_errmsg (db_));
		
	  return;
	}
	
  status = sqlite3_step (stmt);
	
  if (status != SQLITE_DONE)
    {
	  LOG_ERROR (err_prefix_
				 << "insert operation failed: "
		         << sqlite3_errmsg (db_));
		
	  return;
    }
	
  LOG_DEBUG ("data store successful");
}

void
LocationStoreReceiver::onPullRequestReceived (GatewayConnector *sender, 
					                   std::string requestUid,
					                   std::string pluginId,
					                   std::string mimeType,
					                   std::string query,
					                   std::string /* projection */,
					                   unsigned int maxResults,
					                   unsigned int startFromCount,
					                   bool /* liveQuery */)
{
  LOG_DEBUG ("pull request received");
  LOG_DEBUG ("  Query: " << query);
	
  // Finalizes (cleans up) the created SQL statement in the destructor.
  QueryStatementBuilder builder (mimeType, query, db_);
	
  if (!builder.build ())
    {
      LOG_ERROR ("Constsruction of query statement failed");
    }
	
  sqlite3_stmt *query_stmt = builder.query ();
	
  // If the arg is 0, we want unlimited results.	
  unsigned int resultLimit =
    (maxResults == 0 ? ACE_UINT32_MAX : maxResults);
  unsigned int index = 0;
	
  while (sqlite3_step (query_stmt) == SQLITE_ROW
	     && index < resultLimit)
    {
      if (index++ < startFromCount)
        {
	      continue;
        }
		
	  // For insertion, column numbers are 1-based, for extraction, 0-based.
		
	  // SQLite retrieves text as const unsigned char*, reinterpret_cast<>
	  // is the only way to convert it to const char* for std::string assignment.
	  std::string uri (
		reinterpret_cast<const char *> (sqlite3_column_text (query_stmt, 0)));
		
		std::string dataType (
		reinterpret_cast<const char *> (sqlite3_column_text (query_stmt, 1)));
		
	  std::vector<char> data;
	  size_t len = sqlite3_column_bytes (query_stmt, 5);
	  data.resize (len);
		
	  // This trick seems to work for assigning to the vector in one shot.	
	  ACE_OS::memcpy (data.get_allocator ().address (*data.begin ()),
		              sqlite3_column_blob (query_stmt, 5),
		              len);
		
      LOG_DEBUG("Sending response to " << pluginId);
      LOG_DEBUG("  type: " << dataType);
      LOG_DEBUG("   uri: " << uri);
		
      bool good_response =
		sender->pullResponse (requestUid,
							  pluginId,
					          dataType,
							  uri,
					          data);
		
	  if (!good_response)
		{
		  LOG_ERROR ("LocationStoreReceiver - pullrequest: "
					 "sender->pullResponse() failed");
		}
    }
}

