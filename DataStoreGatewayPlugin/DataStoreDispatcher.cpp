
#include "GatewayConnector.h"
#include "log.h"

#include "DataStoreDispatcher.h"
#include "DataStoreConstants.h"

#include "EventQueryHandler.h"
#include "MediaQueryHandler.h"
#include "ReportQueryHandler.h"
#include "SMSQueryHandler.h"
#include "ContactsQueryHandler.h"

#include "OriginalPushHandler.h"
#include "ContactsPushHandler.h"

using namespace ammo::gateway;

DataStoreDispatcher::DataStoreDispatcher (void)
{
}

void
DataStoreDispatcher::dispatchPushData (sqlite3 *db,
                                       PushData &pd)
{
  LOG_TRACE ("Received " << pd);
  bool good_data_store = true;
  
  if (pd.mimeType == PVT_CONTACTS_DATA_TYPE)
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
      LOG_DEBUG ("data store successful");
    }
}

void
DataStoreDispatcher::dispatchPullRequest (sqlite3 *db,
                                          GatewayConnector *sender,
                                          PullRequest &pr)
{
  LOG_TRACE ("pull request received");

  if (sender == 0)
    {
      LOG_WARN ("Sender is null, no responses will be sent");
    }
		
  LOG_DEBUG ("Data type: " << pr.mimeType);
  
  // Incoming SMS mime types have the destination user name appended to this
  // base string, which we then pass to std::string::find instead of checking
  // for equality.
  if (pr.mimeType.find (SMS_MSG_DATA_TYPE) == 0)
    {
      SMSQueryHandler handler (db, sender, pr);
      handler.handleQuery ();
    }
  else if (pr.mimeType == REPORT_DATA_TYPE)
    {
      ReportQueryHandler handler (db, sender, pr);
      handler.handleQuery ();
    }
  else if (pr.mimeType == EVENT_DATA_TYPE)
    {
      EventQueryHandler handler (db, sender, pr);
      handler.handleQuery ();
    }
  else if (pr.mimeType == MEDIA_DATA_TYPE)
    {
      MediaQueryHandler handler (db, sender, pr);
      handler.handleQuery ();
    }
  else if (pr.mimeType == PVT_CONTACTS_DATA_TYPE)
    {
      ContactsQueryHandler handler (db, sender, pr);
      handler.handleQuery ();
    }
}
