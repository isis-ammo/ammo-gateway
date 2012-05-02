#include <sqlite3.h>

#include "ace/Connector.h"
#include "ace/SOCK_Connector.h"
#include <ace/OS_NS_sys_time.h>

#include "log.h"

#include "OriginalQueryStatementBuilder.h"
#include "DataStoreUtils.h"

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

  // We append the last query parameter (the user that a message
  // was directed to) to the data type string, in accordance
  // with the way SMS messages are constructed.
  if (!parser_.directed_user_.empty ())
    {
      mime_type_ = mime_type_ + "_" + parser_.directed_user_;
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


