#include <sqlite3.h>

#include "OriginalQueryStatementBuilder.h"
#include "log.h"

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
  if (!parser_.directed_user ().empty ())
    {
      mime_type_ = mime_type_ + "_" + parser_.directed_user ();
    }

//  LOG_TRACE ("Querying for " << mime_type_);
      
  bool good_adds =
    this->addFilter (parser_.uri (), "uri", false)
    && this->addFilter (mime_type_, "mime_type", false)
    && this->addFilter (parser_.user (), "origin_user", false)
    && this->addFilter (parser_.time_begin (), "tv_sec>=?", true)
    && this->addFilter (parser_.time_end (), "tv_sec<=?", true);
    
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
    this->bindText (parser_.uri ())
    && this->bindText (mime_type_)
    && this->bindText (parser_.user ())
    && this->bindInteger (parser_.time_begin ())
    && this->bindInteger (parser_.time_end ());
}


