
#include <sqlite3.h>

#include <ace/UUID.h>

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
#include "GatewaySyncSerialization.h"

using namespace ammo::gateway;

DataStoreDispatcher::DataStoreDispatcher (void)
  : cfg_mgr_ (0),
    new_uuid_ (0)
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
DataStoreDispatcher::dispatchPointToPointMessage (
  sqlite3 *db,
  GatewayConnector *sender,
  const PointToPointMessage &msg)
{
  //LOG_DEBUG ("point-to-point message type: " << msg.mimeType);
  
  if (msg.mimeType == cfg_mgr_->getReqCsumsMimeType ())
    {
      PointToPointMessage reply;
      reply.destinationGateway = msg.sourceGateway;
      reply.destinationPluginId = msg.sourcePluginId;
      reply.uid = this->gen_uuid ();
      reply.mimeType =
        DataStoreConfigManager::getInstance ()->getSendCsumsMimeType ();
  
      requestChecksumsMessageData decoder;
      decoder.decodeJson (msg.data);
      ACE_Time_Value tv (ACE_OS::gettimeofday ());
      
      // Value in decoder is expected to be a negative offset.
      tv.sec (tv.sec () + decoder.tv_sec_);
      
      this->fetch_recent_checksums (db, tv);
      
      sendChecksumsMessageData encoder;
      encoder.checksums_ = checksums_;
      
      reply.data = encoder.encodeJson ();
      reply.encoding = "json";
      
      if (sender != 0)
        {
          sender->pointToPointMessage (reply);
        }
    }
  else if (msg.mimeType == cfg_mgr_->getSendCsumsMimeType ())
    {
      PointToPointMessage reply;
      reply.destinationGateway = msg.sourceGateway;
      reply.destinationPluginId = msg.sourcePluginId;
      reply.uid = this->gen_uuid ();
      reply.mimeType =
        DataStoreConfigManager::getInstance ()->getReqObjsMimeType ();
      
      sendChecksumsMessageData decoder;
      decoder.decodeJson (msg.data);
      this->collect_missing_checksums (db, decoder.checksums_);
      
      requestObjectsMessageData encoder;
      encoder.checksums_ = decoder.checksums_;
      reply.data = encoder.encodeJson ();
      reply.encoding = "json";
      
      if (sender != 0)
        {
          sender->pointToPointMessage (reply);
        }
    }
  else if (msg.mimeType == cfg_mgr_->getReqObjsMimeType ())
    {
      PointToPointMessage reply;
      reply.destinationGateway = msg.sourceGateway;
      reply.destinationPluginId = msg.sourcePluginId;
      reply.uid = this->gen_uuid ();
      reply.mimeType =
        DataStoreConfigManager::getInstance ()->getSendObjsMimeType ();
        
      sendChecksumsMessageData decoder;
      decoder.decodeJson (msg.data);
      sendObjectsMessageData encoder;
      this->match_requested_checksums (db, encoder, decoder.checksums_);
      reply.data = encoder.encodeJson ();
      reply.encoding = "json";
      
      if (sender != 0)
        {
          sender->pointToPointMessage (reply);
        }
    }
  else if (msg.mimeType == cfg_mgr_->getSendObjsMimeType ())
    {
      sendObjectsMessageData decoder;
      decoder.decodeJson (msg.data);
      
      for (std::vector<sendObjectsMessageData::dbRow>::const_iterator i =
             decoder.objects_.begin ();
           i != decoder.objects_.end ();
           ++i)
        {
          PushData pd;
          pd.uri = i->uri_;
          pd.mimeType = i->mime_type_;
          pd.originUsername = i->origin_user_;
          pd.data = i->data_;
          pd.encoding = msg.encoding;
          pd.priority = msg.priority;
          
          ACE_Time_Value tv (i->tv_sec_, i->tv_usec_);
          OriginalPushHandler handler (db, pd, &tv, i->checksum_);
          bool good_data_store = handler.handlePush ();
          
          if (!good_data_store)
            {
              LOG_ERROR ("Data store failed on received object");
            }
        }
    }
}

void
DataStoreDispatcher::set_cfg_mgr (DataStoreConfigManager *cfg_mgr)
{
  cfg_mgr_ = cfg_mgr;
}

bool
DataStoreDispatcher::fetch_recent_checksums (sqlite3 *db,
                                             const ACE_Time_Value &tv)
{
  checksums_.clear ();
  
  // TODO - the private contacts tables.
  const char * query_str =
    "SELECT checksum FROM data_table WHERE "
    "tv_sec>? OR tv_sec=? AND tv_usec>=?";

  sqlite3_stmt *stmt = 0;
  
  int status = sqlite3_prepare (db,
                                query_str,
                                ACE_OS::strlen (query_str),
                                &stmt,
                                0);

  if (status != SQLITE_OK)
    {
      LOG_ERROR ("Preparation of recent checksum query failed: "
                 << sqlite3_errmsg (db));

      return false;
    }

  status = sqlite3_bind_int (stmt, 1, tv.sec ());
  
  if (status != SQLITE_OK)
    {
      LOG_ERROR ("Bind of integer at index 1 failed: "
                 << sqlite3_errmsg (db));

      return false;
    }

  status = sqlite3_bind_int (stmt, 2, tv.sec ());
  
  if (status != SQLITE_OK)
    {
      LOG_ERROR ("Bind of integer at index 2 failed: "
                 << sqlite3_errmsg (db));

      return false;
    }

  status = sqlite3_bind_int (stmt, 3, tv.usec ());

  if (status != SQLITE_OK)
    {
      LOG_ERROR ("Bind of integer at index 3 failed: "
                 << sqlite3_errmsg (db));

      return false;
    }

  while (sqlite3_step (stmt) == SQLITE_ROW)
    {
      std::string tmp ((char *) sqlite3_column_blob (stmt, 0),
                       DataStoreUtils::CS_SIZE);
      checksums_.push_back (tmp);
    }
    
  sqlite3_finalize (stmt);
  return true;
}

bool
DataStoreDispatcher::match_requested_checksums (
  sqlite3 *db,
  sendObjectsMessageData &holder,
  const std::vector<std::string> &checksums)
{
  // TODO - private contacts tables.
  std::string query_str (
    "SELECT * from data_table WHERE checksum IN (");
    
  for (unsigned long i = 0; i < checksums.size (); ++i)
    {
      query_str.append (i == 0 ? "?" : ",?");
    }
    
  query_str.append (")");
    
  sqlite3_stmt *stmt = 0;
  
  int status = sqlite3_prepare (db,
                                query_str.c_str (),
                                query_str.length (),
                                &stmt,
                                0);

  if (status != SQLITE_OK)
    {
      LOG_ERROR ("Preparation of checksum match query failed: "
                 << sqlite3_errmsg (db));

      return false;
    }
    
  unsigned int slot = 1U;
    
  for (std::vector<std::string>::const_iterator i = checksums.begin ();
       i != checksums.end ();
       ++i)
    {
      bool good_bind = DataStoreUtils::bind_blob (db,
                                                  stmt,
                                                  slot,
                                                  i->c_str (),
                                                  i->length (),
                                                  false);
                                                  
      if (!good_bind)
        {
          // Other useful info already output by bind_blob().
          LOG_ERROR (" - in match_requested_checksums()");
		
          return false;
        }
    }

  while (sqlite3_step (stmt) == SQLITE_ROW)
    {
      sendObjectsMessageData::dbRow row;
      
      row.uri_ =
        reinterpret_cast<const char *> (sqlite3_column_text (stmt, 0));
        
      row.mime_type_ =
        reinterpret_cast<const char *> (sqlite3_column_text (stmt, 1));
        
      row.origin_user_ =
        reinterpret_cast<const char *> (sqlite3_column_text (stmt, 2));
        
      row.tv_sec_ = sqlite3_column_int (stmt, 3);
      
      row.tv_usec_ = sqlite3_column_int (stmt, 4);
      
	    size_t len = sqlite3_column_bytes (stmt, 5);
      row.data_ = ((char *) sqlite3_column_blob (stmt, 5), len);
      
      row.checksum_ =
        ((char *) sqlite3_column_blob (stmt, DataStoreUtils::CS_SIZE), len);
        
      holder.objects_.push_back (row);
    }
    
  sqlite3_finalize (stmt);
  return true;
}

bool
DataStoreDispatcher::collect_missing_checksums (
  sqlite3 *db,
  const std::vector<std::string> &checksums)
{
  checksums_.clear ();
  sqlite3_stmt *stmt = 0;
  const char *qry =
    "SELECT * FROM data_table WHERE checksum = ?";
  
  int status = sqlite3_prepare (db, qry, -1, &stmt, 0);

  if (status != SQLITE_OK)
    {
      LOG_ERROR ("Preparation of checksums-missing query failed: "
                 << sqlite3_errmsg (db));

      return false;
    }
    
  for (std::vector<std::string>::const_iterator i = checksums.begin ();
       i != checksums.end ();
       ++i)
    {
      status = sqlite3_bind_blob (stmt,
                                  1,
                                  i->c_str (),
                                  DataStoreUtils::CS_SIZE,
                                  SQLITE_STATIC);

      if (status != SQLITE_OK)
        {
          LOG_ERROR ("Bind to checksums-missing query failed: "
                     << sqlite3_errmsg (db));

          return false;
        }
    
      status = sqlite3_step (stmt);
      
      if (status == SQLITE_DONE)
        {
          // Above return code means checksum not found in db.
          checksums_.push_back (*i);
        }
        
      sqlite3_reset (stmt);
    }
    
  sqlite3_finalize (stmt);  
  return true;
}

void
update_db (sqlite3 *db,
           const sendObjectsMessageData &data)

{
}

const char *
DataStoreDispatcher::gen_uuid (void)
{
  ACE_Utils::UUID uuid;
  ACE_Utils::UUID_GENERATOR::instance ()->generate_UUID (uuid);
  new_uuid_ = uuid.to_string ();
  return new_uuid_->c_str ();
}

