#include <sstream>
#include <sqlite3.h>

#include <ace/OS_NS_sys_time.h>

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
	  "insignia BLOB,"
	  "checksum TEXT)";
	  
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
  insert_str += " values (?,?,?,?,?,?,?,?,?,?,?,?,?)";
  
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
    
  // Populate our checksum buffer with a new checksum.
  this->new_checksum ();
	
  unsigned int index = 1U;
  
  // Skip the last 2 binary columns (not used at present)
  // to bind the checksum in the last column.
  unsigned int final = 13U;
    
  bool good_binds =
    DataStoreUtils::bind_text (db_, stmt_, index, pd_.uri, true)
    && DataStoreUtils::bind_text (db_, stmt_, index, root["first_name"].asString (), true)
    && DataStoreUtils::bind_text (db_, stmt_, index, root["middle_initial"].asString (), true)
    && DataStoreUtils::bind_text (db_, stmt_, index, root["last_name"].asString (), true)
    && DataStoreUtils::bind_text (db_, stmt_, index, root["rank"].asString (), true)
    && DataStoreUtils::bind_text (db_, stmt_, index, root["call_sign"].asString (), true)
    && DataStoreUtils::bind_text (db_, stmt_, index, root["branch"].asString (), true)
    && DataStoreUtils::bind_text (db_, stmt_, index, root["unit"].asString (), true)
    && DataStoreUtils::bind_text (db_, stmt_, index, root["email"].asString (), true)
    && DataStoreUtils::bind_text (db_, stmt_, index, root["phone"].asString (), true)
    && DataStoreUtils::bind_text (db_, stmt_, final, checksum_.c_str (), true);
    
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
