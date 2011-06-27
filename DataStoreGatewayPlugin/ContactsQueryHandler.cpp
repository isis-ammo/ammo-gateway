
#include <stdexcept>
#include <sqlite3.h>

#include "json/value.h"
#include "log.h"
#include "GatewayConnector.h"

#include "ContactsQueryHandler.h"

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
      LOG_ERROR ("DataStoreReceiver - pullrequest: "
                 "Construction of query statement failed");
      return;
    }
    
  // If the arg is 0, we want unlimited results.	
  unsigned int resultLimit =
    (pr_.maxResults == 0 ? ACE_UINT32_MAX : pr_.maxResults);
  unsigned int index = 0;
  
  sqlite3_stmt *stmt = builder_.query ();
  
  while (sqlite3_step (stmt) == SQLITE_ROW
         && index < resultLimit)
    {LOG_TRACE ("got match");
      if (index++ < pr_.startFromCount)
        {
          continue;
        }
        
//	    size_t len = sqlite3_column_bytes (stmt, 5);
//      std::string data ((char *) sqlite3_column_blob (stmt, 5), len);
      
      LOG_TRACE ("matched on: " << pr_.query.c_str ());
        
      if (sender_ == 0)
        {
          // No response can be sent, but we will still see the trace
          // and debug output up to this point.  
          continue;
        }
/*		
	    // For insertion, column numbers are 1-based, for extraction
	    // they're 0-based. SQLite retrieves text as const unsigned
	    // char*, reinterpret_cast<> is the only way to convert it
	    // to const char* for std::string assignment.
	    std::string uri (
		    reinterpret_cast<const char *> (sqlite3_column_text (stmt, 0)));
		
		  std::string dataType (
		    reinterpret_cast<const char *> (sqlite3_column_text (stmt, 1)));
			   
      LOG_DEBUG ("Sending response to " << pr_.pluginId);
      LOG_DEBUG ("  type: " << dataType);
      LOG_DEBUG ("   uri: " << uri);
      
      ammo::gateway::PullResponse response =
        ammo::gateway::PullResponse::createFromPullRequest (pr_);
      response.mimeType = dataType;
      response.uri = uri;
      response.data = data;
		
      bool good_response =
		    sender_->pullResponse (response);
		
      if (!good_response)
	      {
	        LOG_ERROR ("DataStoreReceiver - pullrequest: "
                     "sender->pullResponse() failed");
	      }
*/
    }
}


