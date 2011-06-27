
#include "GatewayConnector.h"

#include "QueryHandlerFactory.h"

#include "DataStoreConstants.h"

#include "EventQueryHandler.h"
#include "MediaQueryHandler.h"
#include "ReportQueryHandler.h"
#include "SMSQueryHandler.h"
#include "ContactsQueryHandler.h"

QueryHandlerFactory::QueryHandlerFactory (void)
{
}

QueryHandler *
QueryHandlerFactory::createHandler (sqlite3 *db,
                                    ammo::gateway::GatewayConnector *sender,
                                    ammo::gateway::PullRequest &pr)
{
  // Incoming SMS mime types have the destination user name appended to this
  // base string, which we then pass to std::string::find instead of checking
  // for equality.
  if (pr.mimeType.find (SMS_MSG_DATA_TYPE) == 0)
    {
      return new SMSQueryHandler (db, sender, pr);
    }
  else if (pr.mimeType == REPORT_DATA_TYPE)
    {
      return new ReportQueryHandler (db, sender, pr);
    }
  else if (pr.mimeType == EVENT_DATA_TYPE)
    {
      return new EventQueryHandler (db, sender, pr);
    }
  else if (pr.mimeType == MEDIA_DATA_TYPE)
    {
      return new MediaQueryHandler (db, sender, pr);
    }
  else if (pr.mimeType == PVT_CONTACTS_DATA_TYPE)
    {
      return new ContactsQueryHandler (db, sender, pr);
    }
    
  return  0;
}
