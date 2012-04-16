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
                                          const ACE_Time_Value *tv,
                                          const std::string &checksum)
  : PushHandler (db, pd)
{
}

bool
OriginalPushHandler::handlePush (void)
{
  ACE_Time_Value tv =
    (tv_ == 0 ? ACE_OS::gettimeofday () : *tv_);

  int status =
	  sqlite3_prepare (db_,
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
    
  // Populate our checksum buffer with a new checksum, but only if
  // a time value and checksum were not passed to the constructor.
  if (tv_ == 0)
    {
      this->new_checksum (tv);
    }
    
  unsigned int index = 1U;
  
  bool good_binds =
    DataStoreUtils::bind_text (db_, stmt_, index, pd_.uri, true)
    && DataStoreUtils::bind_text (db_, stmt_, index, pd_.mimeType, true)
    && DataStoreUtils::bind_text (db_, stmt_, index, pd_.originUsername, true)
    && DataStoreUtils::bind_int (db_, stmt_, index, tv.sec ())
    && DataStoreUtils::bind_int (db_, stmt_, index, tv.usec ())
    && DataStoreUtils::bind_blob (db_, stmt_, index, pd_.data.data (), pd_.data.length (), true)
    && DataStoreUtils::bind_blob (db_, stmt_, index, checksum_.c_str (), DataStoreUtils::CS_SIZE, true);

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
