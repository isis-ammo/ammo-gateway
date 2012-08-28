
#include "GatewayConnector.h"
#include "log.h"

#include "DataStoreDispatcher.h"
#include "DataStoreConfigManager.h"
#include "DataStore_API.h"

#include "EventQueryHandler.h"
#include "MediaQueryHandler.h"
#include "ReportQueryHandler.h"
#include "SMSQueryHandler.h"
#include "ContactsQueryHandler.h"
#include "ChatQueryHandler.h"
#include "ChatMediaQueryHandler.h"

#include "OriginalPushHandler.h"
#include "ContactsPushHandler.h"

using namespace ammo::gateway;

DataStoreDispatcher::DataStoreDispatcher (void)
  : cfg_mgr_ (0)
{
}

void
DataStoreDispatcher::init (void)
{
  cfg_mgr_ = DataStoreConfigManager::getInstance ();
}

void
DataStoreDispatcher::dispatchPushData (sqlite3 *db,
                                       GatewayConnector *sender,
                                       PushData &pd)
{
//  LOG_TRACE ("Received " << pd);
  appsPushData (sender, pd);

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
		
  // LOG_DEBUG ("pull request data type: " << pr.mimeType);
  
  // Incoming SMS and chat mime types have the destination
  // user name appended to the base string, so instead of
  // checking for equality, we attempt to match the substring
  // at position 0.
  if (pr.mimeType.find (cfg_mgr_->getSMSMimeType ()) == 0)
    {
      SMSQueryHandler handler (db, sender, pr);
      handler.handleQuery ();
    }
  else if (pr.mimeType.find (cfg_mgr_->getChatMimeType ()) == 0)
    {
      ChatQueryHandler handler (db, sender, pr);
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
  else if (pr.mimeType == cfg_mgr_->getChatMediaMimeType ())
    {
      ChatMediaQueryHandler handler (db, sender, pr);
      handler.handleQuery ();
    }
  else if (pr.mimeType == cfg_mgr_->getPrivateContactsMimeType ())
    {
      ContactsQueryHandler handler (db, sender, pr);
      handler.handleQuery ();
    }
  else
    {
      LOG_TRACE ("query mime type not matched.");
    }
}

void
DataStoreDispatcher::appsPushData (GatewayConnector *sender,
                                   PushData &pd)
{
  DataStoreConfigManager::OBJ_MAP::const_iterator i =
    this->findObjList (pd.mimeType);
    
  if (i != cfg_mgr_->obj_map ().end ())
    {
      for (DataStoreConfigManager::OBJ_LIST::const_iterator j =
             (*i).second.begin ();
           j != (*i).second.end ();
           ++j)
        {
          (*j)->insert (sender, pd);
        }
    }
}
                 
void
DataStoreDispatcher::appsPullRequest (GatewayConnector *sender,
                                      PullRequest &pr)
{
  DataStoreConfigManager::OBJ_MAP::const_iterator i =
    this->findObjList (pr.mimeType);
    
  if (i != cfg_mgr_->obj_map ().end ())
    {
      for (DataStoreConfigManager::OBJ_LIST::const_iterator j =
             (*i).second.begin ();
           j != (*i).second.end ();
           ++j)
        {
          (*j)->query (sender, pr);
        }
    }
}

DataStoreConfigManager::OBJ_MAP::const_iterator
DataStoreDispatcher::findObjList (std::string const &mime_type)
{
  std::string mt (mime_type);

  // SMS mime types get the user name appended, so we must
  // check for a match with the beginning substring. If a 
  // match is found, use the substring as the map search key.
  if (mime_type.find (cfg_mgr_->getSMSMimeType ()) == 0)
    {
      mt = cfg_mgr_->getSMSMimeType ();
    } 
  
  return cfg_mgr_->obj_map ().find (mt);
}


