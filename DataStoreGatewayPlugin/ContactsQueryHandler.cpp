
#include <stdexcept>
#include "sqlite3.h"

#include "json/value.h"
#include "json/writer.h"
#include "log.h"
#include "GatewayConnector.h"

#include "ContactsQueryHandler.h"
#include "DataStoreConfigManager.h"

ContactsQueryHandler::ContactsQueryHandler (
      sqlite3 *db,
      ammo::gateway::GatewayConnector *sender,
      ammo::gateway::PullRequest &pr)
  : QueryHandler (db, sender, pr),
    builder_ (pr.query, db)
{
}

void
ContactsQueryHandler::handleQuery (void)
{
  if (!builder_.build ())
    {
      LOG_ERROR ("ContactsQueryHandler - handleQuery: "
                 "construction of query statement failed");
      return;
    }
    
  // If the arg is 0, we want unlimited results.	
  unsigned int resultLimit =
    (pr_.maxResults == 0 ? ACE_UINT32_MAX : pr_.maxResults);
  unsigned int index = 0;
  unsigned int skip = 0;
  
  sqlite3_stmt *stmt = builder_.query ();

  while (sqlite3_step (stmt) == SQLITE_ROW
         && index < resultLimit)
    {
      if (skip++ < pr_.startFromCount)
        {
          continue;
        }
        
      LOG_TRACE ("matched on: " << pr_.query.c_str ());
        
	    // For insertion, column numbers are 1-based, for extraction
	    // they're 0-based. SQLite retrieves text as const unsigned
	    // char*, reinterpret_cast<> is the only way to convert it
	    // to const char* for std::string assignment.
	    std::string uri (
		    reinterpret_cast<const char *> (sqlite3_column_text (stmt, 0)));
		    
		  std::string pvt_contacts_mimetype =
		    DataStoreConfigManager::getInstance ()->getPrivateContactsMimeType ();
		
      LOG_DEBUG ("Sending response to " << pr_.pluginId);
      LOG_DEBUG ("  type: " << pvt_contacts_mimetype);
      LOG_DEBUG ("   uri: " << uri);
      
      ammo::gateway::PullResponse response =
        ammo::gateway::PullResponse::createFromPullRequest (pr_);
      response.mimeType = pvt_contacts_mimetype;
      response.uri = uri;
      this->encode_row (stmt, response.data);
      
//      LOG_TRACE ("row: " << response.data.c_str ());
		
      if (sender_ == 0)
        {
          // No response can be sent, but we will still see the trace
          // and debug output up to this point.  
          continue;
        }
        
      // Increment after all skips of good matches have been made, but
      // before checking sender_ since we may want to see the debug
      // output even if no responses are sent.
      ++index;
		
      bool good_response =
		    sender_->pullResponse (response);
		
      if (!good_response)
	      {
	        LOG_ERROR ("ContactsQueryHanerl - handleQuery: "
                     "sender->pullResponse() failed");
	      }
    }
}

void
ContactsQueryHandler::encode_row (sqlite3_stmt *stmt,
                                  std::string &output)
{
  Json::Value value;
  
  // SQLite retrieves text as const unsigned char*. Overloaded operator
  // interprets it as bool - reinterpret_cast<> is the only way to
  // convert it to const char* for recognition as text.
  
  static const Json::StaticString fn ("first_name");
  value[fn] =
    reinterpret_cast<const char *> (sqlite3_column_text (stmt, 1));
  
  static const Json::StaticString mi ("middle_initial");
  value[mi] =
    reinterpret_cast<const char *> (sqlite3_column_text (stmt, 2));
  
  static const Json::StaticString ln ("last_name");
  value[ln] =
    reinterpret_cast<const char *> (sqlite3_column_text (stmt, 3));
  
  static const Json::StaticString rk ("rank");
  value[rk] =
    reinterpret_cast<const char *> (sqlite3_column_text (stmt, 4));
  
  static const Json::StaticString cs ("call_sign");
  value[cs] =
    reinterpret_cast<const char *> (sqlite3_column_text (stmt, 5));
  
  static const Json::StaticString br ("branch");
  value[br] =
    reinterpret_cast<const char *> (sqlite3_column_text (stmt, 6));
  
  static const Json::StaticString un ("unit");
  value[un] =
    reinterpret_cast<const char *> (sqlite3_column_text (stmt, 7));
  
  static const Json::StaticString em ("email");
  value[em] =
    reinterpret_cast<const char *> (sqlite3_column_text (stmt, 8));
  
  static const Json::StaticString ph ("phone");
  value[ph] =
    reinterpret_cast<const char *> (sqlite3_column_text (stmt, 9));
    
  size_t len = sqlite3_column_bytes (stmt, 10);
  std::string fo_str ((char *) sqlite3_column_blob (stmt, 10), len);
  static const Json::StaticString fo ("photo");
  value[fo] = fo_str;
  
  len = sqlite3_column_bytes (stmt, 11);
  std::string ig_str ((char *) sqlite3_column_blob (stmt, 11), len);
  static const Json::StaticString ig ("insignia");
  value[ig] = ig_str;
  
  Json::FastWriter writer;
  output = writer.write (value);
}


