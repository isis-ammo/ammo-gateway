#include <sqlite3.h>

#include "GatewayConnector.h"
#include "log.h"
#include "json/value.h"

#include "ContactsPushHandler.h"
#include "DataStoreUtils.h"

ContactsPushHandler::ContactsPushHandler (sqlite3 *db,
                                          const ammo::gateway::PushData &pd)
  : PushHandler (db, pd)
{
}

bool
ContactsPushHandler::handlePush (void)
{
  std::string tbl_name = pd_.originUsername;

  // SQL table names can't contain [.:/]
  DataStoreUtils::legalize_tbl_name (tbl_name);
                
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
	  
	bool good_table_open =
	  DataStoreUtils::createTable (db_,
	                               contacts_tbl_str.c_str (),
	                               "contacts");
	  
  if (! good_table_open)
    {
      return false;
    }

  std::string insert_str ("insert into ");
  insert_str += tbl_name;
  insert_str += " values (?,?,?,?,?,?,?,?,?,?,?,?)";
  
  //LOG_TRACE ("insert str: " << insert_str.c_str ());
	
  int status =
	  sqlite3_prepare_v2 (db_, insert_str.c_str (), -1, &stmt_, 0);
	
  if (status != SQLITE_OK)
    {
      LOG_ERROR ("Private contact push - "
                 << "prep of sqlite statement failed: "
		             << sqlite3_errmsg (db_));
		
      return false;
    }
    
  Json::Value root;
  
  if (! DataStoreUtils::parseJson (pd_.data, root))
    {
      return false;
    }
    
  bool good_binds =
    DataStoreUtils::bind_text (db_, stmt_, 1, pd_.uri, true)
    && DataStoreUtils::bind_text (db_, stmt_, 2, root["first_name"].asString (), true)
    && DataStoreUtils::bind_text (db_, stmt_, 3, root["middle_initial"].asString (), true)
    && DataStoreUtils::bind_text (db_, stmt_, 4, root["last_name"].asString (), true)
    && DataStoreUtils::bind_text (db_, stmt_, 5, root["rank"].asString (), true)
    && DataStoreUtils::bind_text (db_, stmt_, 6, root["call_sign"].asString (), true)
    && DataStoreUtils::bind_text (db_, stmt_, 7, root["branch"].asString (), true)
    && DataStoreUtils::bind_text (db_, stmt_, 8, root["unit"].asString (), true)
    && DataStoreUtils::bind_text (db_, stmt_, 9, root["email"].asString (), true)
    && DataStoreUtils::bind_text (db_, stmt_, 10, root["phone"].asString (), true);
    
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
