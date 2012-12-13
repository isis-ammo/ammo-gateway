#include <sqlite3.h>

#include <ace/OS_NS_sys_time.h>

#include "log.h"

#include "OriginalQueryStatementBuilder.h"
#include "DataStoreUtils.h"
#include "DataStoreConfigManager.h"

OriginalQueryStatementBuilder::OriginalQueryStatementBuilder (
      const std::string &mime_type,
      const std::string &params,
      sqlite3 *db)
  : QueryStatementBuilder (params, db, "SELECT * FROM data_table WHERE "),
    mime_type_ (mime_type),
    bind_index_ (1)
{
}

bool
OriginalQueryStatementBuilder::build (void)
{
  parser_.parse (params_);
  std::string ext_mime_type = mime_type_;

  // We append the last query parameter (the user that a message
  // was directed to) to the data type string. There are two
  // ways to do this. For SMS mime type, we simply append the
  // recipient field, and for chat mime type, we appent the
  // string "user" first. The chat mime type is intended to
  // replace the SMS mime type, but we handle both case here
  // for backward compatibility, or in case it's decided in
  // the future to support both mime types.
  if (!parser_.recipient_.empty ())
    {
      ext_mime_type += "_";
      
      if (mime_type_ == DataStoreConfigManager::getInstance ()->getChatMimeType ())
        {
          ext_mime_type += "user";
        }
        
      ext_mime_type += parser_.recipient_;
    }

//  LOG_TRACE ("Querying for " << mime_type_);
      
  bool good_adds =
    this->addFilter (parser_.uri_, "uri", false)
    && this->addFilter (mime_type_, "mime_type", false)
    && this->addFilter (parser_.user_, "origin_user", false)
    && this->addFilter (parser_.time_begin_, "tv_sec", true, true)
    && this->addFilter (parser_.time_end_, "tv_sec", true, false);
    
    LOG_TRACE("OriginalQueryStatmentBuilder: Statement built:");
    LOG_TRACE("  URI: " << parser_.uri_);
    LOG_TRACE("  type: " << mime_type_);
    LOG_TRACE("  user: " << parser_.user_);
    LOG_TRACE("  time_begin: " << parser_.time_begin_);
    LOG_TRACE("  time_end: " << parser_.time_end_);
    
  if (!good_adds)
    {
      return false;
    }

  int status = sqlite3_prepare (db_,
                                query_str_.c_str (),
                                query_str_.length (),
                                &stmt_,
                                0);

  if (status != SQLITE_OK)
    {
      LOG_ERROR ("Preparation of query statement failed: "
                 << sqlite3_errmsg (db_));

      return false;
    }

  bool good_bind = this->bind ();
  
  if (!good_bind)
    {
      LOG_ERROR ("Bind of values to query statement failed: "
                 << sqlite3_errmsg (db_));
      return false;
    }
    
  LOG_TRACE("Query String: " << query_str_);
  return true;
}

bool
OriginalQueryStatementBuilder::bind (void)
{
  return
    DataStoreUtils::bind_text (db_, stmt_, bind_index_, parser_.uri_, false)
    && DataStoreUtils::bind_text (db_, stmt_, bind_index_, mime_type_, false)
    && DataStoreUtils::bind_text (db_, stmt_, bind_index_, parser_.user_, false)
    && this->bindInteger (parser_.time_begin_)
    && this->bindInteger (parser_.time_end_);
}

bool
OriginalQueryStatementBuilder::bindInteger (const std::string &token)
{
  if (!token.empty ())
    {
	    long val = 0;
	    
	    if (! DataStoreUtils::safe_atol (token, val))
	      {
          LOG_ERROR ("Conversion of string "
                     << token.c_str ()
                     << " to integer failed");

          return false;
        }
		
	    if (val < 0)
	      {
		      // A negative time value indicates that it is to be
		      // used as an offset from the current time.
		      val += static_cast<long> (ACE_OS::gettimeofday ().sec ());
		    }
		    
		  unsigned int uval = static_cast<unsigned int> (val);
		    
		  return DataStoreUtils::bind_int (db_, stmt_, bind_index_, uval);
    }

  return true;
}


