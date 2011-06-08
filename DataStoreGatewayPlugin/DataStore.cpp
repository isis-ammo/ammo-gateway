#include <vector>
#include <stdexcept>

#include <sqlite3.h>

#include "ace/OS_NS_sys_time.h"

#include "log.h"

#include "json/reader.h"
#include "json/value.h"

#include "DataStore.h"
#include "QueryStatementBuilder.h"

using namespace ammo::gateway;

#include "EventFilter.h"
#include "MediaFilter.h"
#include "SMSFilter.h"
#include "ReportFilter.h"

DataStoreReceiver::DataStoreReceiver (void)
  : db_ (0),
    err_prefix_ ("DataStoreReceiver - data push: ")
{
}

DataStoreReceiver::~DataStoreReceiver (void)
{
  LOG_DEBUG ("Closing Data Store Service database...");
	
  sqlite3_close (db_);
}

void
DataStoreReceiver::onConnect (GatewayConnector * /* sender */)
{
}

void
DataStoreReceiver::onDisconnect (GatewayConnector * /* sender */)
{
}

void
DataStoreReceiver::onPushDataReceived (GatewayConnector * /* sender */,
                                       PushData &pushData)
{
  LOG_DEBUG ("Received " << pushData);
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
                 << "data type bind failed: "
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
DataStoreReceiver::onPullRequestReceived (GatewayConnector *sender,
                                          PullRequest &pullReq)
{
  LOG_DEBUG ("pull request received");
  LOG_DEBUG ("  Query: " << pullReq.query);
	
  if (sender == 0)
    {
      LOG_ERROR ("Sender is null");
      return;
    }
		
  // Finalizes (cleans up) the created SQL statement in the destructor.
  QueryStatementBuilder builder (pullReq.mimeType, pullReq.query, db_);
	
  if (!builder.build ())
    {
      LOG_ERROR ("DataStoreReceiver - pullrequest: "
                 "Constsruction of query statement failed");
      return;
    }
	
  sqlite3_stmt *query_stmt = builder.query ();
	
  // If the arg is 0, we want unlimited results.	
  unsigned int resultLimit =
    (pullReq.maxResults == 0 ? ACE_UINT32_MAX : pullReq.maxResults);
  unsigned int index = 0;
	
  while (sqlite3_step (query_stmt) == SQLITE_ROW
         && index < resultLimit)
    {
      if (index++ < pullReq.startFromCount)
        {
          continue;
        }

	    // For insertion, column numbers are 1-based, for extraction
	    // they're 0-based. SQLite retrieves text as const unsigned
	    // char*, reinterpret_cast<> is the only way to convert it
	    // to const char* for std::string assignment.
	    std::string uri (
		    reinterpret_cast<const char *> (sqlite3_column_text (query_stmt, 0)));
		
		  std::string dataType (
		    reinterpret_cast<const char *> (sqlite3_column_text (query_stmt, 1)));
		
	   
	    size_t len = sqlite3_column_bytes (query_stmt, 5);
      std::string data ((char *) sqlite3_column_blob (query_stmt, 5), len);
      
      if (!this->matchedData (pullReq.mimeType, pullReq.projection, data))
        {
          continue;
        }
		
      LOG_DEBUG ("Sending response to " << pullReq.pluginId);
      LOG_DEBUG ("  type: " << dataType);
      LOG_DEBUG ("   uri: " << uri);
      
      PullResponse response =
        PullResponse::createFromPullRequest (pullReq);
      response.mimeType = dataType;
      response.uri = uri;
      response.data = data;
		
      bool good_response =
		    sender->pullResponse (response);
		
      if (!good_response)
	      {
	        LOG_ERROR ("DataStoreReceiver - pullrequest: "
                     "sender->pullResponse() failed");
	      }
    }
}

void
DataStoreReceiver::db_filepath (const std::string &path)
{
  db_filepath_ = path;
}

bool
DataStoreReceiver::init (void)
{
  std::string filepath (db_filepath_);
  filepath += "DataStore_db.sql3";
  
//  LOG_DEBUG ("full path = " << filepath.c_str ());
  
  int status = sqlite3_open (filepath.c_str (), &db_);
  
  if (status != 0)
    {
      LOG_ERROR ("Data Store Service - " << sqlite3_errmsg (db_));
      return false;
    }
  
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
	    LOG_ERROR ("Data Store Service - " << db_err);
			return false;
	  }
	  
	LOG_DEBUG ("Data Store Service database opened successfully...");  
	return true;
}

bool
DataStoreReceiver::matchedData (const std::string &mimeType,
                                const std::string &projection,
                                const std::string &data)
{
  // No content-based filtering to be done, return immediately.
  if (projection.empty ())
    {
      return true;
    }

  Json::Reader reader;
  Json::Value root;

  bool goodParse = reader.parse (data, root);
        
  if (!goodParse)
    {
      LOG_ERROR ("DataStoreReceiver pullrequest"
                 " - JSON parsing error: "
                 << reader.getFormatedErrorMessages ());
                 
      return false;
    }

  LOG_DEBUG ("Data type: " << mimeType);
  LOG_DEBUG ("Parsed JSON: " << root.toStyledString ());
  
  // Incoming SMS mime types have the destination user name appended to this
  // base string, which we then pass to std::string::find instead of checking
  // for equality.
  std::string sms_mime_base ("application/vnd.edu.vu.isis.ammo.sms.message");
  
  // Some of the legacy SMS entries have 'createdDate' and 'modifiedDate'
  // fields that are (1) reals instead of long integers as required,
  // and (2) expressed as usec instead of sec, too large by a factor of 1000.
  // Since the value is out of range, Json::Value::asInt() will throw
  // std::runtime_error, with the message that we catch and output below.
  // As of this date (2011-5-18), the Json-parsed entry is output (for
  // debugging) so the offending field can be seen by inspection.
  try
    {
      if (mimeType == "application/vnd.edu.vu.isis.ammo.dash.event")
        {
          return this->matchedEvent (root, projection);
        }
      else if (mimeType == "application/vnd.edu.vu.isis.ammo.dash.media")
        {
          return this->matchedMedia (root, projection);
        }
      else if (mimeType.find (sms_mime_base) == 0)
        {
          return this->matchedSMS (root, projection);
        }
      else if (mimeType == "application/vnd.edu.vu.isis.ammo.report.report_base")
        {
          return this->matchedReport (root, projection);
        }
    }
  catch (const std::runtime_error &ex)
    {
      LOG_ERROR ("Malformed database entry - " << ex.what ());
      return false;
    }
    
  // In case we have registered interest in other mime types, but
  // do no filtering on the associated data content.  
  return true;
}

bool
DataStoreReceiver::matchedEvent (const Json::Value &root,
                                 const std::string &projection)
{
  EventFilter filter (root, projection);
  return filter.match ();
}
                   
bool
DataStoreReceiver::matchedMedia (const Json::Value &root,
                                 const std::string &projection)
{
  MediaFilter filter (root, projection);
  return filter.match ();
}
                   
bool
DataStoreReceiver::matchedSMS (const Json::Value &root,
                               const std::string &projection)
{
  SMSFilter filter (root, projection);
  return filter.match ();
}

bool
DataStoreReceiver::matchedReport(const Json::Value &root,
                                 const std::string &projection)
{
  ReportFilter filter (root, projection);
  return filter.match ();
}


