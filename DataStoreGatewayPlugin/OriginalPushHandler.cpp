#include <sqlite3.h>

#include <ace/OS_NS_sys_time.h>

#include "GatewayConnector.h"
#include "log.h"

#include "OriginalPushHandler.h"
#include "DataStoreUtils.h"

OriginalPushHandler::OriginalPushHandler (sqlite3 *db,
                                          const ammo::gateway::PushData &pd)
  : PushHandler (db, pd)
{
}

OriginalPushHandler::OriginalPushHandler (sqlite3 *db,
                                          const ammo::gateway::PushData &pd,
                                          const ACE_Time_Value &tv,
                                          const std::string &checksum)
  : PushHandler (db, pd, tv, checksum)
{
}

bool
OriginalPushHandler::handlePush (void)
{
  int status =
	  sqlite3_prepare_v2 (db_,
			                  "insert into data_table values (?,?,?,?,?,?,?)",
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
    
  // No-op if checksum and timestamp were passed to our constructor.  
  this->create_checksum ();
    
  unsigned int index = 1U;
  
  bool good_binds =
    DataStoreUtils::bind_text (db_, stmt_, index, pd_.uri, true)
    && DataStoreUtils::bind_text (db_, stmt_, index, pd_.mimeType, true)
    && DataStoreUtils::bind_text (db_, stmt_, index, pd_.originUsername, true)
    && DataStoreUtils::bind_int (db_, stmt_, index, tv_.sec ())
    && DataStoreUtils::bind_int (db_, stmt_, index, tv_.usec ())
    && DataStoreUtils::bind_blob (db_, stmt_, index, pd_.data.data (), pd_.data.length (), true)
    && DataStoreUtils::bind_text (db_, stmt_, index, checksum_.c_str (), true);

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

  return good_binds;
}
