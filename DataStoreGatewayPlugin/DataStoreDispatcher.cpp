
#include <sstream>
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
//  LOG_DEBUG ("point-to-point message type: " << msg.mimeType);
  LOG_TRACE("DataStoreDispatcher::dispatchPointToPointMessage");
  if (msg.mimeType == cfg_mgr_->getReqCsumsMimeType ())
    {
      LOG_TRACE("Got request for checksums...");
      PointToPointMessage reply;
      reply.destinationGateway = msg.sourceGateway;
      reply.destinationPluginId = msg.sourcePluginId;
      reply.uid = this->gen_uuid ();
      reply.mimeType =
        DataStoreConfigManager::getInstance ()->getSendCsumsMimeType ();
  
      requestChecksumsMessageData decoder;
      decoder.decodeJson (msg.data);
      
      if (decoder.tv_sec_ > 0)
        {
	        LOG_ERROR ("Checksum request error: "
	                   << "time offset must be non-positive");
	        return;
        }
        
      ACE_Time_Value tv (ACE_OS::gettimeofday ());
      tv.sec (tv.sec () + decoder.tv_sec_);
      
      if (!this->collect_recent_checksums (db, tv))
        {
	        LOG_ERROR ("Checksum request error: "
	                   << "fetching recent checksums from db failed");
	        return;
        }
      
      sendChecksumsMessageData encoder;
      encoder.checksums_ = checksums_;
      reply.data = encoder.encodeJson ();
      reply.encoding = "json";
      
      LOG_TRACE("Sending checksums...");
      LOG_TRACE("  " << reply.data);
      sender->pointToPointMessage (reply);
    }
  else if (msg.mimeType == cfg_mgr_->getSendCsumsMimeType ())
    {
      LOG_TRACE("Got checksums...");
      PointToPointMessage reply;
      reply.destinationGateway = msg.sourceGateway;
      reply.destinationPluginId = msg.sourcePluginId;
      reply.uid = this->gen_uuid ();
      reply.mimeType =
        DataStoreConfigManager::getInstance ()->getReqObjsMimeType ();
      
      sendChecksumsMessageData decoder;
      decoder.decodeJson (msg.data);

      // Stores missing checksums in member checksums_.
      if (!this->collect_missing_checksums (db, decoder.checksums_))
        {
	        LOG_ERROR ("Checksum send error: "
	                   << "fetching missing checksums from db failed");
	        return;
        }
        
      requestObjectsMessageData encoder;
      encoder.checksums_ = checksums_;

      reply.data = encoder.encodeJson ();
      reply.encoding = "json";
      
      LOG_TRACE("Requesting objects...");
      LOG_TRACE("  " << reply.data);
      sender->pointToPointMessage (reply);
    }
  else if (msg.mimeType == cfg_mgr_->getReqObjsMimeType ())
    {
      LOG_TRACE("Got request for objects...");
      PointToPointMessage reply;
      reply.destinationGateway = msg.sourceGateway;
      reply.destinationPluginId = msg.sourcePluginId;
      reply.uid = this->gen_uuid ();
      reply.mimeType =
        DataStoreConfigManager::getInstance ()->getSendObjsMimeType ();
        
      requestObjectsMessageData decoder;
      decoder.decodeJson (msg.data);
      sendObjectsMessageData encoder;
      
      if (!this->match_requested_checksums (db, encoder, decoder.checksums_))
        {
	        LOG_ERROR ("Object request error: "
	                   << "fetching objects from db failed");
	        return;
        }
      
      reply.data = encoder.encodeJson ();
      reply.encoding = "json";
      
      LOG_TRACE("Sending objects...");
      LOG_TRACE("  " << reply.data);
      sender->pointToPointMessage (reply);
    }
  else if (msg.mimeType == cfg_mgr_->getSendObjsMimeType ())
    {
      LOG_TRACE("Got objects");
      sendObjectsMessageData decoder;
      decoder.decodeJson (msg.data);
      DataStoreConfigManager *cfg_mgr =
        DataStoreConfigManager::getInstance ();
        
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
          
          LOG_TRACE("Storing object " << pd.uri);
          LOG_TRACE("  " << pd.data);
          
          ACE_Time_Value tv (i->tv_sec_, i->tv_usec_);
          
          // Use the alternate constructor to set the handler's
          // ACE_Time_Value and checksum members, so it will
          // store the received values instead of generating new values.
          
          if (pd.mimeType == cfg_mgr->getPrivateContactsMimeType ())
            {
              ContactsPushHandler handler (db, pd, tv, i->checksum_);
              
              if (handler.handlePush ())
                {
                  LOG_TRACE ("Data store succeeded on private "
                             << "contacts object from reconnect");
                }
              else
                {
                  LOG_ERROR ("Data store failed on private "
                             << "contacts object from reconnect");
                }
            }
          else
            {
              OriginalPushHandler handler (db, pd, tv, i->checksum_);
              
              if (handler.handlePush ())
                {
                  LOG_TRACE ("Data store succeeded on object from reconnect");
                }
              else
                {
                  LOG_ERROR ("Data store failed on object from reconnect");
                }
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
DataStoreDispatcher::collect_recent_checksums (sqlite3 *db,
                                               const ACE_Time_Value &tv)
{
  checksums_.clear ();
  std::string qry ("SELECT checksum FROM data_table WHERE "
                   "tv_sec>? OR tv_sec=? AND tv_usec>=?"
                   "UNION SELECT checksum from contacts_table "
                   "WHERE tv_sec>? OR tv_sec=? AND tv_usec>=?");

  sqlite3_stmt *stmt = 0;
  
  int status = sqlite3_prepare_v2 (db,
                                   qry.c_str (),
                                   qry.length (),
                                   &stmt,
                                   0);

  if (status != SQLITE_OK)
    {
      LOG_ERROR ("Preparation of recent checksum query failed: "
                 << sqlite3_errmsg (db));

      return false;
    }

  unsigned int slot = 1U;
  
  bool good_binds =
    DataStoreUtils::bind_int (db, stmt, slot, tv.sec ())
    && DataStoreUtils::bind_int (db, stmt, slot, tv.sec ())
    && DataStoreUtils::bind_int (db, stmt, slot, tv.usec ())
    && DataStoreUtils::bind_int (db, stmt, slot, tv.sec ())
    && DataStoreUtils::bind_int (db, stmt, slot, tv.sec ())
    && DataStoreUtils::bind_int (db, stmt, slot, tv.usec ());
  
  if (!good_binds)
    {
      // Error msg already output in bind_int().
      return false;
    }
    
  while (sqlite3_step (stmt) == SQLITE_ROW)
    {
      std::string tmp (
        reinterpret_cast<const char *> (sqlite3_column_text (stmt, 0)));
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
  sqlite3_stmt *stmt = 0;
  
  if (!this->prepare_match_statement (db, stmt, "data_table", checksums))
    {
      LOG_ERROR ("Preparation of data_table query failed: "
                 << sqlite3_errmsg (db));

      return false;
    }

  this->extract_original_row (stmt, holder);
  
  stmt = 0;
  
  if (!this->prepare_match_statement (db, stmt, "contacts_table", checksums))
    {
      LOG_ERROR ("Preparation of contacts_table query failed: "
                 << sqlite3_errmsg (db));

      return false;
    }
    
  this->extract_contacts_row (stmt, holder);

  return true;
}

bool
DataStoreDispatcher::prepare_match_statement (
  sqlite3 *db,
  sqlite3_stmt *&stmt,
  const char *tbl_name,
  const std::vector<std::string> &checksums)
{
  std::string query_str ("SELECT * from ");
  query_str += tbl_name;
  query_str += " WHERE checksum IN (";
    
  for (unsigned long i = 0; i < checksums.size (); ++i)
    {
      query_str.append (i == 0 ? "?" : ",?");
    }
    
  query_str.append (")");
  
  int status = sqlite3_prepare_v2 (db,
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
      bool good_bind = DataStoreUtils::bind_text (db,
                                                  stmt,
                                                  slot,
                                                  *i,
                                                  false);
                                                  
      if (!good_bind)
        {
          // Other useful info already output by bind_text().
          LOG_ERROR (" - in match_requested_checksums()");
		
          return false;
        }
    }
    
  return true;
}

void
DataStoreDispatcher::extract_original_row (
  sqlite3_stmt *stmt,
  sendObjectsMessageData &holder)
{
  while (sqlite3_step (stmt) == SQLITE_ROW)
    {
      sendObjectsMessageData::dbRow row;
      unsigned int slot = holder.objects_.size ();
      holder.objects_.push_back (row);
      
      holder.objects_[slot].uri_ =
        reinterpret_cast<const char *> (sqlite3_column_text (stmt, 0));
        
      holder.objects_[slot].mime_type_ =
        reinterpret_cast<const char *> (sqlite3_column_text (stmt, 1));
        
      holder.objects_[slot].origin_user_ =
        reinterpret_cast<const char *> (sqlite3_column_text (stmt, 2));
        
      holder.objects_[slot].tv_sec_ = sqlite3_column_int (stmt, 3);
      
      holder.objects_[slot].tv_usec_ = sqlite3_column_int (stmt, 4);
      
      holder.objects_[slot].data_ =
        reinterpret_cast<const char *> (sqlite3_column_blob (stmt, 5));
      
      holder.objects_[slot].checksum_ =
        reinterpret_cast<const char *> (sqlite3_column_text (stmt, 6));
    }
    
  sqlite3_finalize (stmt);
}

void
DataStoreDispatcher::extract_contacts_row (
  sqlite3_stmt *stmt,
  sendObjectsMessageData &holder)
{
  while (sqlite3_step (stmt) == SQLITE_ROW)
    {
      sendObjectsMessageData::dbRow row;
      unsigned int slot = holder.objects_.size ();
      holder.objects_.push_back (row);
      
      holder.objects_[slot].uri_ =
        reinterpret_cast<const char *> (sqlite3_column_text (stmt, 0));
        
      holder.objects_[slot].mime_type_ =
        DataStoreConfigManager::getInstance ()->getPrivateContactsMimeType ();
        
      holder.objects_[slot].origin_user_ =
        reinterpret_cast<const char *> (sqlite3_column_text (stmt, 1));
        
      holder.objects_[slot].tv_sec_ = sqlite3_column_int (stmt, 2);
      
      holder.objects_[slot].tv_usec_ = sqlite3_column_int (stmt, 3);
      
      std::ostringstream o;
      o << "{"
        << "\"first_name\": \"" << sqlite3_column_text (stmt, 4) << "\", "
        << "\"middle_initial\": \"" << sqlite3_column_text (stmt, 5) << "\", "
        << "\"last_name\": \"" << sqlite3_column_text (stmt, 6) << "\", "
        << "\"rank\": \"" << sqlite3_column_text (stmt, 7) << "\", "
        << "\"call_sign\": \"" << sqlite3_column_text (stmt, 8) << "\", "
        << "\"branch\": \"" << sqlite3_column_text (stmt, 9) << "\", "
        << "\"unit\": \"" << sqlite3_column_text (stmt, 10) << "\", "
        << "\"email\": \"" << sqlite3_column_text (stmt, 11) << "\", "
        << "\"phone\": \"" << sqlite3_column_text (stmt, 12) << "\""
        << "}";
      
      holder.objects_[slot].data_ = o.str ();
      
      holder.objects_[slot].checksum_ =
        reinterpret_cast<const char *> (sqlite3_column_text (stmt, 15));
    }
    
  sqlite3_finalize (stmt);
}

bool
DataStoreDispatcher::collect_missing_checksums (
  sqlite3 *db,
  const std::vector<std::string> &checksums)
{
  checksums_.clear ();
  sqlite3_stmt *stmt = 0;
  
  // 'NULL' because we are interested only in the case where
  // no rows are returned, so there will be nothing to display.
  const char *qry =
    "SELECT NULL FROM data_table WHERE checksum = ? UNION "
    "SELECT NULL FROM contacts_table WHERE checksum = ?";
  
  int status = sqlite3_prepare_v2 (db, qry, -1, &stmt, 0);

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
      unsigned int n = 1U;
      
      bool good_binds =
        DataStoreUtils::bind_text (db, stmt, n, *i, false)
        && DataStoreUtils::bind_text (db, stmt, n, *i, false);

      if (!good_binds)
        {
          // Error diagnostic is already output.
          return false;
        }
        
      status = sqlite3_step (stmt);
      
      if (status == SQLITE_DONE)
        {
          // Above return code means checksum not found in db,
          // so we add it to the 'missing' list.
          checksums_.push_back (*i);
        }
        
      sqlite3_reset (stmt);
    }

  sqlite3_finalize (stmt);  
  return true;
}

std::string
DataStoreDispatcher::gen_uuid (void)
{
  ACE_Utils::UUID uuid;
  ACE_Utils::UUID_GENERATOR::instance ()->generate_UUID (uuid);
  new_uuid_ = uuid.to_string ();
  return std::string(new_uuid_->c_str()); //uuid.to_string returns a pointer, which won't be valid any more after uuid goes out of scope, so we must copy
}

