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

ContactsPushHandler::ContactsPushHandler (sqlite3 *db,
                                          const ammo::gateway::PushData &pd,
                                          const ACE_Time_Value &tv,
                                          const std::string &checksum)
  : PushHandler (db, pd, tv, checksum)
{
}

bool
ContactsPushHandler::handlePush (void)
{
  std::string insert_str (
    "insert into contacts_table values (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)");
  
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
    
  // No-op if checksum and timestamp were passed to our constructor.  
  this->create_checksum ();
	
  unsigned int index = 1U;
  
  bool good_binds =
    DataStoreUtils::bind_text (db_, stmt_, index, pd_.uri, true)
    && DataStoreUtils::bind_text (db_, stmt_, index, pd_.originUsername, true)
    && DataStoreUtils::bind_int (db_, stmt_, index, tv_.sec ())
    && DataStoreUtils::bind_int (db_, stmt_, index, tv_.usec ())
    && DataStoreUtils::bind_text (db_, stmt_, index, root["first_name"].asString (), true)
    && DataStoreUtils::bind_text (db_, stmt_, index, root["middle_initial"].asString (), true)
    && DataStoreUtils::bind_text (db_, stmt_, index, root["last_name"].asString (), true)
    && DataStoreUtils::bind_text (db_, stmt_, index, root["rank"].asString (), true)
    && DataStoreUtils::bind_text (db_, stmt_, index, root["call_sign"].asString (), true)
    && DataStoreUtils::bind_text (db_, stmt_, index, root["branch"].asString (), true)
    && DataStoreUtils::bind_text (db_, stmt_, index, root["unit"].asString (), true)
    && DataStoreUtils::bind_text (db_, stmt_, index, root["email"].asString (), true)
    && DataStoreUtils::bind_text (db_, stmt_, index, root["phone"].asString (), true)
    
    // Skip these binary fields until we're sure they can be encoded as JSON strings.
    && DataStoreUtils::bind_blob (db_, stmt_, index, 0, 0, true)
    && DataStoreUtils::bind_blob (db_, stmt_, index, 0, 0, true)
    
    && DataStoreUtils::bind_text (db_, stmt_, index, checksum_.c_str (), true);
    
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
