#include <sqlite3.h>
#include <algorithm>

#include "ace/OS_NS_sys_time.h"

#include "log.h"

#include "json/reader.h"
#include "json/value.h"

#include "DataStore.h"
#include "DataStoreConstants.h"

using namespace ammo::gateway;

DataStoreReceiver::DataStoreReceiver (void)
  : db_ (0),
    stmt_ (0)
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
  LOG_TRACE ("data push received");
  dispatcher_.dispatchPushData (db_, pushData);
}

void
DataStoreReceiver::onPullRequestReceived (GatewayConnector *sender,
                                          PullRequest &pullReq)
{
  LOG_TRACE ("pull request received");
  dispatcher_.dispatchPullRequest (db_, sender, pullReq);
}

// Called by the config manager.
void
DataStoreReceiver::db_filepath (const std::string &path)
{
  db_filepath_ = path;
  
  if (db_filepath_[db_filepath_.size () - 1] != '/')
    {
      db_filepath_.push_back ('/');
      LOG_TRACE ("Added trailing forward slash to db file path name");
    }
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
  
  const char *data_tbl_str =
	  "CREATE TABLE IF NOT EXISTS data_table ("
	  "uri TEXT,"
	  "mime_type TEXT,"
	  "origin_user TEXT,"
	  "tv_sec INTEGER NOT NULL,"
	  "tv_usec INTEGER,"
	  "data BLOB)";
	
  char *db_err = 0;
	
  sqlite3_exec (db_, data_tbl_str, 0, 0, &db_err);
	
  if (db_err != 0)
	  {
	    LOG_ERROR ("Data Store Service data table - " << db_err);
			return false;
	  }
	  
	LOG_DEBUG ("Data Store Service data table opened successfully...");
	
  return true;
}


