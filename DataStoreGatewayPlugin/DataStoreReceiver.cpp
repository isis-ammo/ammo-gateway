#include <sqlite3.h>

#include "ace/Connector.h"
#include "ace/SOCK_Connector.h"

#include <ace/OS_NS_sys_stat.h>
#include <ace/OS_NS_unistd.h>

#include "log.h"

#include "DataStoreReceiver.h"
#include "DataStoreDispatcher.h"
#include "DataStoreConfigManager.h"
#include "DataStoreUtils.h"

using namespace ammo::gateway;

DataStoreReceiver::DataStoreReceiver (void)
  : db_ (0)
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
  dispatcher_.dispatchPushData (db_, pushData);
}

void
DataStoreReceiver::onPullRequestReceived (GatewayConnector *sender,
                                          PullRequest &pullReq)
{
  dispatcher_.dispatchPullRequest (db_, sender, pullReq);
}

void
DataStoreReceiver::db_filepath (const std::string &path)
{
  db_filepath_ = path;
}

bool
DataStoreReceiver::init (void)
{
  // The config manager's receiver and connectro references have been set,
  // so we can initialize the dispatcher's member with a call using the
  // default null arguments.
  dispatcher_.set_cfg_mgr (DataStoreConfigManager::getInstance ());

  if (!check_path ())
    {
      // check_path() will also output error info.
      LOG_ERROR ("DataStoreReceiver::init() failed");
      return false;
    }
    
  std::string fullpath (db_filepath_);
  fullpath += "DataStore_db.sql3";
  
//  LOG_DEBUG ("full path = " << fullpath.c_str ());
  
  int status = sqlite3_open (fullpath.c_str (), &db_);
  
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
	  "data BLOB,"
	  "checksum BLOB)";
	
  char *db_err = 0;
	
  sqlite3_exec (db_, data_tbl_str, 0, 0, &db_err);
	
  if (db_err != 0)
	  {
	    LOG_ERROR ("Data Store Service data table - " << db_err);
			return false;
	  }
	  
  return true;
}

bool
DataStoreReceiver::check_path (void)
{
  char delimiter = '/';
  
  std::string::size_type lastPos =
    db_filepath_.find_first_not_of (delimiter, 0);
  std::string::size_type pos =
    db_filepath_.find_first_of (delimiter, lastPos);
  
  std::string seg = db_filepath_.substr (lastPos, pos - lastPos);
  bool top_level = true;
  
  while (std::string::npos != pos || std::string::npos != lastPos)
    {
      //LOG_DEBUG ("segment: " << seg);
      int result = 0;
      
      switch (db_filepath_[0])
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
    
  // Make sure it ends with a slash so we can just append the db filename.  
  if (db_filepath_[db_filepath_.size () - 1] != delimiter)
    {
      db_filepath_ += delimiter;
    }
    
  return true;
}

bool
DataStoreReceiver::fetch_recent_checksums (const ACE_Time_Value &tv)
{
  checksums_.clear ();
  
  // TODO - the private contacts tables.
  const char * query_str =
    "SELECT checksum FROM data_table WHERE "
    "tv_sec>? OR tv_sec=? AND tv_usec>=?";

  sqlite3_stmt *stmt = 0;
  
  int status = sqlite3_prepare (db_,
                                query_str,
                                ACE_OS::strlen (query_str),
                                &stmt,
                                0);

  if (status != SQLITE_OK)
    {
      LOG_ERROR ("Preparation of recent checksum query failed: "
                 << sqlite3_errmsg (db_));

      return false;
    }

  status = sqlite3_bind_int (stmt, 1, tv.sec ());
  
  if (status != SQLITE_OK)
    {
      LOG_ERROR ("Bind of integer at index 1 failed: "
                 << sqlite3_errmsg (db_));

      return false;
    }

  status = sqlite3_bind_int (stmt, 2, tv.sec ());
  
  if (status != SQLITE_OK)
    {
      LOG_ERROR ("Bind of integer at index 2 failed: "
                 << sqlite3_errmsg (db_));

      return false;
    }

  status = sqlite3_bind_int (stmt, 3, tv.usec ());

  if (status != SQLITE_OK)
    {
      LOG_ERROR ("Bind of integer at index 3 failed: "
                 << sqlite3_errmsg (db_));

      return false;
    }

  while (sqlite3_step (stmt) == SQLITE_ROW)
    {
      std::string tmp ((char *) sqlite3_column_blob (stmt, 0),
                       DataStoreUtils::CS_SIZE);
      checksums_.push_back (tmp);
    }
    
  sqlite3_finalize (stmt);
  return true;
}

bool
DataStoreReceiver::match_requested_checksums (
  const std::vector<std::string> &checksums)
{
  // TODO - private contacts tables.
  std::string query_str (
    "SELECT * from data_table WHERE checksum IN (");
    
  for (unsigned long i = 0; i < checksums.size (); ++i)
    {
      query_str.append (i == 0 ? "?" : ",?");
    }
    
  query_str.append (")");
    
  sqlite3_stmt *stmt = 0;
  
  int status = sqlite3_prepare (db_,
                                query_str.c_str (),
                                query_str.length (),
                                &stmt,
                                0);

  if (status != SQLITE_OK)
    {
      LOG_ERROR ("Preparation of checksum match query failed: "
                 << sqlite3_errmsg (db_));

      return false;
    }
    
  unsigned int slot = 1U;
    
  for (std::vector<std::string>::const_iterator i = checksums.begin ();
       i != checksums.end ();
       ++i)
    {
      bool good_bind = DataStoreUtils::bind_blob (db_,
                                                  stmt,
                                                  slot,
                                                  i->c_str (),
                                                  i->length (),
                                                  false);
                                                  
      if (!good_bind)
        {
          // Other useful info already output by bind_blob().
          LOG_ERROR (" - in match_requested_checksums()");
		
          return false;
        }
    }

  while (sqlite3_step (stmt) == SQLITE_ROW)
    {
      // TODO - Prep selected object for remote reply.
    }
    
  sqlite3_finalize (stmt);
  return true;
}

bool
DataStoreReceiver::collect_missing_checksums (
  const std::vector<std::string> &checksums)
{
  checksums_.clear ();
  sqlite3_stmt *stmt = 0;
  const char *qry =
    "SELECT * FROM data_table WHERE checksum = ?";
  
  int status = sqlite3_prepare (db_, qry, -1, &stmt, 0);

  if (status != SQLITE_OK)
    {
      LOG_ERROR ("Preparation of checksums-missing query failed: "
                 << sqlite3_errmsg (db_));

      return false;
    }
    
  for (std::vector<std::string>::const_iterator i = checksums.begin ();
       i != checksums.end ();
       ++i)
    {
      status = sqlite3_bind_blob (stmt,
                                  1,
                                  i->c_str (),
                                  DataStoreUtils::CS_SIZE,
                                  SQLITE_STATIC);

      if (status != SQLITE_OK)
        {
          LOG_ERROR ("Bind to checksums-missing query failed: "
                     << sqlite3_errmsg (db_));

          return false;
        }
    
      status = sqlite3_step (stmt);
      
      if (status == SQLITE_DONE)
        {
          // Above return code means checksum not found in db.
          checksums_.push_back (*i);
        }
        
      sqlite3_reset (stmt);
    }
    
  sqlite3_finalize (stmt);  
  return true;
}

