#include "sqlite3.h"

#include "ace/OS_NS_stdlib.h"

#include "QueryStatementBuilder.h"
#include "log.h"

QueryStatementBuilder::QueryStatementBuilder (
      const std::string &mime_type,
      const std::string &params,
      sqlite3 *db)
  : mime_type_ (mime_type),
    params_ (params),
    db_ (db),
    stmt_ (0),
    has_term_ (false),
    query_str_ ("SELECT * FROM the_table WHERE "),
    digits_ ("0123456789"),
    bind_index_ (1)
{
}

QueryStatementBuilder::~QueryStatementBuilder (void)
{
  // OK if stmt_ is 0.
  sqlite3_finalize (stmt_);
}

bool
QueryStatementBuilder::build (void)
{
  parser_.parse (params_);

  bool good_add =
    addFilter (parser_.uri (), "uri", false);

  if (!good_add)
    {
      LOG_ERROR ("Addition of URI filter failed");
      return false;
    }
  
  //this appends the last query parameter (the user that a message was directed
  //to) to the MIME type, in accordance with the way SMS messages are constructed.
  if (parser_.directed_user () != "") {
    std::string queryType = mime_type_ + "_" + parser_.directed_user();
    LOG_TRACE("Querying for " << queryType);
    good_add =
      addFilter (queryType, "mime_type", false);
  } else {
    LOG_TRACE("Querying for " << mime_type_);
    good_add =
      addFilter (mime_type_, "mime_type", false);
  }

  if (!good_add)
    {
      LOG_ERROR ("Addition of mime_type filter failed");
      return false;
    }

  good_add =
    addFilter (parser_.user (), "origin_user", false);

  if (!good_add)
    {
      LOG_ERROR ("Addition of origin user filter failed");
      return false;
    }

  good_add =
    addFilter (parser_.time_begin (), "tv_sec>=?", true);

  if (!good_add)
    {
      LOG_ERROR ("Addition of timestamp lower bound filter failed");
      return false;
    }

  good_add =
    addFilter (parser_.time_end (), "tv_sec<=?", true);

  if (!good_add)
    {
      LOG_ERROR ("Addition of timestamp upper bound filter failed");
      return false;
    }

  if (!has_term_)
    {
      LOG_ERROR ("SELECT must have at least one condition");
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

  return bind ();
}

sqlite3_stmt *
QueryStatementBuilder::query (void) const
{
  return stmt_;
}

bool
QueryStatementBuilder::bind (void)
{
  if (!bindText (parser_.uri ()))
    {
      return false;
    }

  if (!bindText (mime_type_))
    {
      return false;
    }

  if (!bindText (parser_.user ()))
    {
      return false;
    }

  if (!bindInteger (parser_.time_begin ()))
    {
      return false;
    }

  if (!bindInteger (parser_.time_end ()))
    {
      return false;
    }

  return true;
}

bool
QueryStatementBuilder::addFilter (const std::string &token,
                                  const char *stub,
                                  bool is_int)
{
  if (!token.empty ())
    {
      if (is_int && token.find_first_not_of (digits_) != std::string::npos)
        {
          LOG_ERROR (token.c_str () << " is malformed");

          return false;
        }

      if (has_term_)
        {
          query_str_ += " AND ";
        }
      else
        {
          has_term_ = true;
        }

      query_str_ += stub;

      if (!is_int)
        {
          if (token.find ('%') == std::string::npos)
            {
              query_str_ += "=?";
            }
          else
            {
              query_str_ += " LIKE ?";
            }
        }
    }

  return true;
}

bool
QueryStatementBuilder::bindText (const std::string &token)
{
  if (!token.empty ())
    {
      int status = sqlite3_bind_text (stmt_,
                                      bind_index_++,
                                      token.c_str (),
                                      token.length (),
                                      SQLITE_STATIC);

      if (status != SQLITE_OK)
        {
          LOG_ERROR ("Bind of text \""
                     << token.c_str ()
                     << "\" failed: "
                     << sqlite3_errmsg (db_));

          return false;
        }
    }

  return true;
}

bool
QueryStatementBuilder::bindInteger (const std::string &token)
{
  if (!token.empty ())
    {
      int status = sqlite3_bind_int (stmt_,
                                     bind_index_++,
                                     ACE_OS::atol (token.c_str ()));

      if (status != SQLITE_OK)
        {
          LOG_ERROR ("Bind of integer "
                     << token.c_str ()
                     << " failed: "
                     << sqlite3_errmsg (db_));

          return false;
        }
    }

  return true;
}

