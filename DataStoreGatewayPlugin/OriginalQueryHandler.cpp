
#include <stdexcept>
#include <sqlite3.h>

#include "json/value.h"
#include "log.h"
#include "GatewayConnector.h"

#include "OriginalQueryHandler.h"
#include "DataStoreUtils.h"

OriginalQueryHandler::OriginalQueryHandler (
      sqlite3 *db,
      ammo::gateway::GatewayConnector *sender,
      ammo::gateway::PullRequest &pr)
  : QueryHandler (db, sender, pr),
    builder_ (pr.mimeType, pr.query, db)
{
}

OriginalQueryHandler::~OriginalQueryHandler (void)
{
}

void
OriginalQueryHandler::handleQuery (void)
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
  unsigned int skip = 0;
  
  sqlite3_stmt *stmt = builder_.query ();
  
  while (sqlite3_step (stmt) == SQLITE_ROW
         && index < resultLimit)
    {
	    size_t len = sqlite3_column_bytes (stmt, 5);
      std::string data ((char *) sqlite3_column_blob (stmt, 5), len);
      
//      LOG_TRACE ("Data type: " << pr_.mimeType);
  
      if (!this->matchedData (pr_.projection, data))
        {
          continue;
        }
        
      // Skip 'startFromCount' # of good matches.  
      if (skip++ < pr_.startFromCount)
        {
          continue;
        }
        
      // Increment after all skips of good matches have been made, but
      // before checking sender_ since we may want to see the debug
      // output even if no responses are sent.
      ++index;
        
//      LOG_TRACE ("matched on: " << pr_.projection);
//      LOG_TRACE (data);
        
      if (sender_ == 0)
        {
          // No response can be sent, but we will still see the trace
          // and debug output up to this point.  
          continue;
        }
		
	    // For insertion, column numbers are 1-based, for extraction
	    // they're 0-based. SQLite retrieves text as const unsigned
	    // char*, reinterpret_cast<> is the only way to convert it
	    // to const char* for std::string assignment.
	    std::string uri (
		    reinterpret_cast<const char *> (sqlite3_column_text (stmt, 0)));
		
      LOG_DEBUG ("Sending response to " << pr_.pluginId);
      LOG_DEBUG ("  type: " << pr_.mimeType);
      LOG_DEBUG ("   uri: " << uri);
      
      ammo::gateway::PullResponse response =
        ammo::gateway::PullResponse::createFromPullRequest (pr_);
      response.uri = uri;
      response.data = data;
		
      bool good_response =
		    sender_->pullResponse (response);
		
      if (!good_response)
	      {
	        LOG_ERROR ("DataStoreReceiver - pullrequest: "
                     "sender->pullResponse() failed");
	      }
    }
}

bool
OriginalQueryHandler::matchedData (const std::string &projection,
                                   const std::string &data)
{
  if (projection.empty ())
    {
      return true;
    }
    
  Json::Value root;
  
  if (! DataStoreUtils::parseJson (data, root))
    {
      return false;
    }
   
  // Some of the legacy SMS entries have 'createdDate' and 'modifiedDate'
  // fields that are (1) reals instead of long integers as required,
  // and (2) expressed as msec instead of sec, too large by a factor of 1000.
  // Since the value is out of range, Json::Value::asInt() will throw
  // std::runtime_error, with the message that we catch and output below.
  // As of this date (2011-5-18), the Json-parsed entry is output (for
  // debugging) so the offending field can be seen by inspection.
  try
    { 
      return this->matchedProjection (root, projection);
    }
  catch (const std::runtime_error &ex)
    {
      LOG_ERROR ("Malformed database entry - " << ex.what ());
      return false;
    }
}
