#include <sqlite3.h>
#include <algorithm>

#include "ace/OS_NS_sys_time.h"
#include "ace/OS_NS_sys_stat.h"
#include "ace/OS_NS_stdlib.h"

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
bool
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
  if (!check_path ())
    {
      // check_path() will also output error info.
      LOG_ERROR ("DataStoreReceiver::init() failed");
      return false;
    }
    
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

bool
DataStoreReceiver::check_path (void)
{
  char delimiter = '/';
  
  std::string::size_type lastPos = db_filepath_.find_first_not_of (delimiter, 0);
  std::string::size_type pos = db_filepath_.find_first_of (delimiter, lastPos);
  
  std::string seg = db_filepath_.substr (lastPos, pos - lastPos);
  bool top_level = true;
  
  while (std::string::npos != pos || std::string::npos != lastPos)
    {
      //LOG_DEBUG ("segment: " << seg);
      int result = 0;
      
      switch (path[0])
        {
          case '/':
            result =
              ACE_OS::chdir (top_level
                             ? (std::string ("/") + seg).c_str ()
                             : seg.c_str ());
            break;
          case '$':
            result =
              ACE_OS::chdir (top_level
                             ? ACE_OS::getenv (seg.c_str () + 1)
                             : seg.c_str ());
            break;
          default:
            result = ACE_OS::chdir (seg.c_str ());
            break;
        }

      if (result == -1)
        {
          LOG_ERROR ("check_path(); error changing current directory to "
                     << seg);
          return false;
        }
  
      lastPos = db_filepath_.find_first_not_of (delimiter, pos);
      pos = db_filepath_.find_first_of (delimiter, lastPos);
      
      // This would get caught in the next iteration but
      // we need to check and possibly exit here.
      if (std::string::npos == lastPos)
        {
          return true;
        }
        
      seg = db_filepath_.substr (lastPos, pos - lastPos);
      
      top_level = false;
      result = ACE_OS::mkdir (seg.c_str (), S_IRWXU | S_IRWXG | S_IRWXO);
      
      // EEXIST is ok - directory already existed, ignore the return value.
      if (result != 0 && errno != EEXIST)
        {
          LOG_ERROR ("check_path(): error creating db filepath directory "
                     << seg);
          return false;
        }
    }
}


