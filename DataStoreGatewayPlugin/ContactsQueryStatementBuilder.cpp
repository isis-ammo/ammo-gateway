#include <sqlite3.h>
#include <algorithm>

#include <ace/OS_NS_sys_time.h>

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
  
  std::string tbl_name (parser_.contact_owner_);
  
  // SQL table names can't contain [.:/]
  DataStoreUtils::legalize_tbl_name (tbl_name);
  
  query_str_ += tbl_name;
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
  unsigned int index = 1;

  return
    DataStoreUtils::bind_text (db_, stmt_, index, parser_.uri_, false)
    && DataStoreUtils::bind_text (db_, stmt_, index, parser_.first_name_, false)
    && DataStoreUtils::bind_text (db_, stmt_, index, parser_.middle_initial_, false)
    && DataStoreUtils::bind_text (db_, stmt_, index, parser_.last_name_, false)
    && DataStoreUtils::bind_text (db_, stmt_, index, parser_.rank_, false)
    && DataStoreUtils::bind_text (db_, stmt_, index, parser_.call_sign_, false)
    && DataStoreUtils::bind_text (db_, stmt_, index, parser_.branch_, false)
    && DataStoreUtils::bind_text (db_, stmt_, index, parser_.unit_, false)
    && DataStoreUtils::bind_text (db_, stmt_, index, parser_.email_, false)
    && DataStoreUtils::bind_text (db_, stmt_, index, parser_.phone_, false);
}


