#include <sqlite3.h>
#include <algorithm>

#include "ace/OS_NS_sys_time.h"

#include "log.h"

#include "json/reader.h"
#include "json/value.h"

#include "DataStore.h"
#include "QueryHandlerFactory.h"
#include "QueryHandler.h"

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
  LOG_TRACE ("Received " << pushData);
  bool good_data_store = true;
  
  if (pushData.mimeType.find (PVT_CONTACTS_DATA_TYPE) == 0)
    {
      good_data_store = this->pushContactData (pushData);
    }
  else
    {
      good_data_store = this->pushGeneralData (pushData);
    }
	
	if (good_data_store)
	  {
      LOG_DEBUG ("data store successful");
    }
}

void
DataStoreReceiver::onPullRequestReceived (GatewayConnector *sender,
                                          PullRequest &pullReq)
{
  LOG_TRACE ("pull request received");

  if (sender == 0)
    {
      LOG_WARN ("Sender is null, no responses will be sent");
    }
		
  LOG_DEBUG ("Data type: " << pullReq.mimeType);
  LOG_DEBUG ("query string: " << pullReq.query);
  
  QueryHandlerFactory factory;
  QueryHandler *handler = factory.createHandler (db_, sender, pullReq);
  
  handler->handleQuery ();
  
  delete handler;
  handler = 0;
}

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
	
  return this->createTable (data_tbl_str, "data");
}

bool
DataStoreReceiver::pushContactData (const ammo::gateway::PushData &pushData)
{
  Json::Value root;
  
  if (!this->parseJson (pushData.data, root))
    {
      return false;
    }
    
  PrivateContact info;
  this->contactFromJson (root, info);
  
  // SQL table names can't contain '.', so we append the chars of
  // the arg we use for the table name to the query string,
  // while replacing '.' with '_'.
  std::string tbl_name = pushData.originUsername;
  std::replace_if (tbl_name.begin (),
                   tbl_name.end (),
                   std::bind2nd (std::equal_to<char> (), '.'),
                   '_');
  
  // 2011-06-21 - not using the last 2 columns at this point.
	std::string contacts_tbl_str ("CREATE TABLE IF NOT EXISTS ");
	contacts_tbl_str += tbl_name;
  contacts_tbl_str += " ("
    "uri TEXT,"
	  "first_name TEXT,"
	  "middle_initial TEXT,"
	  "last_name TEXT,"
	  "rank TEXT,"
	  "call_sign TEXT,"
	  "branch TEXT,"
	  "unit TEXT,"
	  "email TEXT,"
	  "phone TEXT,"
	  "photo BLOB,"
	  "insignia BLOB)";
	  
  if (!this->createTable (contacts_tbl_str.c_str (), "contacts"))
    {
      return false;
    }
    
  std::string insert_str ("insert into ");
  insert_str += tbl_name;
  insert_str += " values (?,?,?,?,?,?,?,?,?,?,?,?)";
	
  int status =
	  sqlite3_prepare (db_, insert_str.c_str (), -1, &stmt_, 0);
	
  if (status != SQLITE_OK)
    {
      LOG_ERROR ("Private contact push - "
                 << "prep of sqlite statement failed: "
		             << sqlite3_errmsg (db_));
		
      return false;
    }
    
  const char *insert_type = "Private contact";
  
  bool good_binds =
    this->bind_text (1, pushData.uri, insert_type, "uri")
    && this->bind_text (2, info.first_name, insert_type, "first name")
    && this->bind_text (3, info.middle_initial, insert_type, "middle initial")
    && this->bind_text (4, info.last_name, insert_type, "last name")
    && this->bind_text (5, info.rank, insert_type, "rank")
    && this->bind_text (6, info.call_sign, insert_type, "call sign")
    && this->bind_text (7, info.branch, insert_type, "branch")
    && this->bind_text (8, info.unit, insert_type, "unit")
    && this->bind_text (9, info.email, insert_type, "email")
    && this->bind_text (10, info.phone, insert_type, "phone");
	
	if (good_binds)
	  {
      status = sqlite3_step (stmt_);
	
      if (status != SQLITE_DONE)
        {
          LOG_ERROR ("Private contact push - "
                     << "insert operation failed: "
                     << sqlite3_errmsg (db_));
		
          return false;
        }
    }

  return good_binds;
}

bool
DataStoreReceiver::pushGeneralData (const ammo::gateway::PushData &pushData)
{
  ACE_Time_Value tv (ACE_OS::gettimeofday ());
	
  int status =
	  sqlite3_prepare (db_,
			               "insert into data_table values (?,?,?,?,?,?)",
			               -1,
			               &stmt_,
			               0);
	
  if (status != SQLITE_OK)
    {
      LOG_ERROR ("Data push - "
                 << "prep of sqlite statement failed: "
		             << sqlite3_errmsg (db_));
		
      return false;
    }
    
  const char *insert_type = "Data";
    
  bool good_binds =
    this->bind_text (1, pushData.uri, insert_type, "URI")
    && this->bind_text (2, pushData.mimeType, insert_type, "data type")
    && this->bind_text (3, pushData.originUsername, insert_type, "origin user")
    && this->bind_int (4, tv.sec (), insert_type, "timestamp sec")
    && this->bind_int (5, tv.usec (), insert_type, "timestamp usec")
    && this->bind_blob (6,
                        pushData.data.data (),
                        pushData.data.length (),
                        insert_type,
                        "data");

  if (good_binds)
    {	
      status = sqlite3_step (stmt_);
	
      if (status != SQLITE_DONE)
        {
          LOG_ERROR ("Data push - "
                     << "insert operation failed: "
                     << sqlite3_errmsg (db_));
		
          return false;
        }
    }

  return true;
}

bool
DataStoreReceiver::parseJson (const std::string &input,
                              Json::Value& root)
{
  Json::Reader jsonReader;
  bool parseSuccess = jsonReader.parse (input, root);

  if (!parseSuccess)
    {
      LOG_ERROR ("JSON parsing error:"
                 << jsonReader.getFormatedErrorMessages ());
                 
      return parseSuccess;
    }

  LOG_DEBUG ("Parsed JSON: " << root.toStyledString ());
  return parseSuccess;
}

void
DataStoreReceiver::contactFromJson (const Json::Value &jsonRoot,
                                    PrivateContact &info)
{
  info.first_name = jsonRoot["first_name"].asString ();
  info.middle_initial = jsonRoot["middle_initial"].asString ();
  info.last_name = jsonRoot["last_name"].asString ();
  info.rank = jsonRoot["rank"].asString ();
  info.call_sign = jsonRoot["call_sign"].asString ();
  info.branch = jsonRoot["branch"].asString ();
  info.unit = jsonRoot["unit"].asString ();
  info.email = jsonRoot["email"].asString ();
  info.phone = jsonRoot["phone"].asString ();
}

bool
DataStoreReceiver::createTable (const char *tbl_string,
                                const char *msg)
{
  char *db_err = 0;
	
  sqlite3_exec (db_, tbl_string, 0, 0, &db_err);
	
  if (db_err != 0)
	  {
	    LOG_ERROR ("Data Store Service " << msg << " table - " << db_err);
			return false;
	  }
	  
	LOG_DEBUG ("Data Store Service " << msg << " table opened successfully...");
	
  return true;
}

bool
DataStoreReceiver::bind_text (int column,
                              const std::string &text,
                              const char *push_type,
                              const char *column_name)
{
  // Must make sure that stmt_ is prepared before this call.
  int status =
	  sqlite3_bind_text (stmt_,
					             column,
					             text.c_str (),
					             text.length (),
					             SQLITE_STATIC);
	
  if (status != SQLITE_OK)
    {
      LOG_ERROR (push_type << " push - "
		             << column_name << " bind failed: "
		             << sqlite3_errmsg (db_));
		
      return false;
    }
	
	return true;
}

bool
DataStoreReceiver::bind_int (int column,
                             int val,
                             const char *push_type,
                             const char *column_name)
{
  // Must make sure that stmt_ is prepared before this call.
  int status = sqlite3_bind_int (stmt_, column, val);
	
  if (status != SQLITE_OK)
    {
      LOG_ERROR (push_type << " push - "
                 << column_name << " bind failed: "
                 << sqlite3_errmsg (db_));
		
      return false;
    }
    
  return true;
}

bool
DataStoreReceiver::bind_blob (int column,
                              const void *val,
                              int size,
                              const char *push_type,
                              const char *column_name)
{
  int status =
	  sqlite3_bind_blob (stmt_,
					             column,
					             val,
					             size,
					             SQLITE_STATIC);
	
  if (status != SQLITE_OK)
    {
      LOG_ERROR (push_type << " push - "
                 << column_name << " bind failed: "
                 << sqlite3_errmsg (db_));
		
      return false;
    }
    
  return true;
}


