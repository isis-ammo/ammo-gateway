
#include "GatewayConnector.h"
#include "log.h"

#include "DataStoreDispatcher.h"
#include "DataStoreConfigManager.h"

#include "EventQueryHandler.h"
#include "MediaQueryHandler.h"
#include "ReportQueryHandler.h"
#include "SMSQueryHandler.h"
#include "ContactsQueryHandler.h"

#include "OriginalPushHandler.h"
#include "ContactsPushHandler.h"

using namespace ammo::gateway;

DataStoreDispatcher::DataStoreDispatcher (void)
  : cfg_mgr_ (0)
{
}

void
DataStoreDispatcher::dispatchPushData (sqlite3 *db,
                                       PushData &pd)
{
//  LOG_TRACE ("Received " << pd);
  bool good_data_store = true;
  
  if (pd.mimeType == cfg_mgr_->getPrivateContactsMimeType ())
    {
      ContactsPushHandler handler (db, pd);
      good_data_store = handler.handlePush ();
    }
  else
    {
      OriginalPushHandler handler (db, pd);
      good_data_store = handler.handlePush ();
    }
	
	if (good_data_store)
	  {
      LOG_TRACE ("data store successful");
    }
  else
    {
      LOG_ERROR ("data store failed");
    }
}

void
DataStoreDispatcher::dispatchPullRequest (sqlite3 *db,
                                          GatewayConnector *sender,
                                          PullRequest &pr)
{
  if (sender == 0)
    {
      LOG_WARN ("Sender is null, no responses will be sent");
    }
		
  //LOG_DEBUG ("pull request data type: " << pr.mimeType);
  
  // Incoming SMS mime types have the destination user name appended to this
  // base string, which we then pass to std::string::find instead of checking
  // for equality.
  if (pr.mimeType.find (cfg_mgr_->getSMSMimeType ()) == 0)
    {
      SMSQueryHandler handler (db, sender, pr);
      handler.handleQuery ();
    }
  else if (pr.mimeType == cfg_mgr_->getReportMimeType ())
    {
      ReportQueryHandler handler (db, sender, pr);
      handler.handleQuery ();
    }
  else if (pr.mimeType == cfg_mgr_->getEventMimeType ())
    {
      EventQueryHandler handler (db, sender, pr);
      handler.handleQuery ();
    }
  else if (pr.mimeType == cfg_mgr_->getMediaMimeType ())
    {
      MediaQueryHandler handler (db, sender, pr);
      handler.handleQuery ();
    }
  else if (pr.mimeType == cfg_mgr_->getPrivateContactsMimeType ())
    {
      ContactsQueryHandler handler (db, sender, pr);
      handler.handleQuery ();
    }
}

void
DataStoreDispatcher::set_cfg_mgr (DataStoreConfigManager *cfg_mgr)
{
  cfg_mgr_ = cfg_mgr;
}
