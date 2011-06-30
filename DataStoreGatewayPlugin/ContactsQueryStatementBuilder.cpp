#include <sqlite3.h>
#include <algorithm>

#include "log.h"

#include "ContactsQueryStatementBuilder.h"
#include "DataStoreUtils.h"

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
  query_str_.resize (len + parser_.contact_owner_.length ());
  std::replace_copy_if (parser_.contact_owner_.begin (),
                        parser_.contact_owner_.end (),
                        query_str_.begin () + len,
                        std::bind2nd (std::equal_to<char> (), '.'),
                        '_');
  
  query_str_ += " WHERE ";

//  LOG_TRACE ("Querying for " << mime_type_);
      
  bool good_adds =
    this->addFilter (parser_.uri_, "uri", false)
    && this->addFilter (parser_.first_name_, "first_name", false)
    && this->addFilter (parser_.middle_initial_, "middle_initial", false)
    && this->addFilter (parser_.last_name_, "last_name", false)
    && this->addFilter (parser_.rank_, "rank", false)
    && this->addFilter (parser_.call_sign_, "call_sign", false)
    && this->addFilter (parser_.branch_, "branch", false)
    && this->addFilter (parser_.unit_, "unit", false)
    && this->addFilter (parser_.email_, "email", false)
    && this->addFilter (parser_.phone_, "phone", false);
    
//  LOG_TRACE ("query str: " << query_str_.c_str ());
    
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
    DataStoreUtils::bind_text (db_, stmt_, 1, parser_.uri_, false)
    && DataStoreUtils::bind_text (db_, stmt_, 2, parser_.first_name_, false)
    && DataStoreUtils::bind_text (db_, stmt_, 3, parser_.middle_initial_, false)
    && DataStoreUtils::bind_text (db_, stmt_, 4, parser_.last_name_, false)
    && DataStoreUtils::bind_text (db_, stmt_, 5, parser_.rank_, false)
    && DataStoreUtils::bind_text (db_, stmt_, 6, parser_.call_sign_, false)
    && DataStoreUtils::bind_text (db_, stmt_, 7, parser_.branch_, false)
    && DataStoreUtils::bind_text (db_, stmt_, 8, parser_.unit_, false)
    && DataStoreUtils::bind_text (db_, stmt_, 9, parser_.email_, false)
    && DataStoreUtils::bind_text (db_, stmt_, 10, parser_.phone_, false);
}


