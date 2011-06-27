#include <sqlite3.h>
#include <algorithm>

#include "ContactsQueryStatementBuilder.h"
#include "log.h"

ContactsQueryStatementBuilder::ContactsQueryStatementBuilder (
      const std::string &params,
      sqlite3 *db)
  : QueryStatementBuilder (params, db, "SELECT * FROM ")
{
}

bool
ContactsQueryStatementBuilder::build (void)
{
  parser_.parse (params_);
  
  std::string::size_type len = query_str_.length ();
  
  // SQL table names can't contain '.', so append the chars of
  // the token we parsed for the table name to the query string,
  // while replacing '.' with '_'.
  query_str_.resize (len + parser_.contact_owner ().length ());
  std::replace_copy_if (parser_.contact_owner ().begin (),
                        parser_.contact_owner ().end (),
                        query_str_.begin () + len,
                        std::bind2nd (std::equal_to<char> (), '.'),
                        '_');
  
  query_str_ += " WHERE ";

//  LOG_TRACE ("Querying for " << mime_type_);
      
  bool good_adds =
    this->addFilter (parser_.first_name (), "first_name", false)
    && this->addFilter (parser_.middle_initial (), "middle_initial", false)
    && this->addFilter (parser_.last_name (), "last_name", false)
    && this->addFilter (parser_.rank (), "rank", false)
    && this->addFilter (parser_.call_sign (), "call_sign", false)
    && this->addFilter (parser_.branch (), "branch", false)
    && this->addFilter (parser_.unit (), "unit", false)
    && this->addFilter (parser_.email (), "email", false)
    && this->addFilter (parser_.phone (), "phone", false);
    
  LOG_TRACE ("query str: " << query_str_.c_str ());
    
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
ContactsQueryStatementBuilder::bind (void)
{
  return
    this->bindText (parser_.first_name ())
    && this->bindText (parser_.middle_initial ())
    && this->bindText (parser_.last_name ())
    && this->bindText (parser_.rank ())
    && this->bindText (parser_.call_sign ())
    && this->bindText (parser_.branch ())
    && this->bindText (parser_.unit ())
    && this->bindText (parser_.email ())
    && this->bindText (parser_.phone ());
}


