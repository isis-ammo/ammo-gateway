#include "sqlite3.h"

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
    mime_type_ (mime_type)
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
    && this->addFilter (parser_.time_begin_, "tv_sec>=?", true)
    && this->addFilter (parser_.time_end_, "tv_sec<=?", true);
    
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

  return this->bind ();
}

bool
OriginalQueryStatementBuilder::bind (void)
{
  return
    this->bindText (parser_.uri_)
    && this->bindText (mime_type_)
    && this->bindText (parser_.user_)
    && this->bindInteger (parser_.time_begin_)
    && this->bindInteger (parser_.time_end_);
}


