
#include <stdexcept>
#include <sqlite3.h>

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
  
  sqlite3_stmt *stmt = builder_.query ();
  
  std::string my_mime_type =
    DataStoreConfigManager::getInstance ()->getPrivateContactsMimeType ();

  while (sqlite3_step (stmt) == SQLITE_ROW
         && index < resultLimit)
    {
      if (index++ < pr_.startFromCount)
        {
          continue;
        }
        
//      LOG_TRACE ("matched on: " << pr_.query.c_str ());
        
	    // For insertion, column numbers are 1-based, for extraction
	    // they're 0-based. SQLite retrieves text as const unsigned
	    // char*, reinterpret_cast<> is the only way to convert it
	    // to const char* for std::string assignment.
	    std::string uri (
		    reinterpret_cast<const char *> (sqlite3_column_text (stmt, 0)));
		
      LOG_TRACE ("Sending response to " << pr_.pluginId);
      LOG_TRACE ("  type: " << my_mime_type);
      LOG_TRACE ("   uri: " << uri);
      
      ammo::gateway::PullResponse response =
        ammo::gateway::PullResponse::createFromPullRequest (pr_);
      response.mimeType = my_mime_type;
      response.uri = uri;
      this->encode_row (stmt, response.data);
      
      if (sender_ == 0)
        {
          // No response can be sent, but we will still see the trace
          // and debug output up to this point.  
          continue;
        }
		
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
    
  LOG_TRACE ("matched row: " << value.toStyledString ());
    
  Json::FastWriter writer;
  output = writer.write (value);
}


