#include <sqlite3.h>
#include <algorithm>

#include "ace/Connector.h"
#include "ace/SOCK_Connector.h"
#include <ace/OS_NS_sys_time.h>

#include "log.h"

#include "ContactsQueryStatementBuilder.h"
#include "DataStoreUtils.h"

ContactsQueryStatementBuilder::ContactsQueryStatementBuilder (
      const std::string &params,
      sqlite3 *db)
  : QueryStatementBuilder (params, db, "SELECT * FROM contacts_table")
{
}

bool
ContactsQueryStatementBuilder::build (void)
{
  parser_.parse (params_);
  
  query_str_ += " WHERE ";

//  LOG_TRACE ("Querying for " << mime_type_);
      
  bool good_adds =
    this->addFilter (parser_.uri_, "uri", false)
    && this->addFilter (parser_.contact_owner_, "origin_user", false)
    && this->addFilter (parser_.time_begin_, "tv_sec>=?", true)
    && this->addFilter (parser_.time_end_, "tv_sec<=?", true)
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

  int status = sqlite3_prepare_v2 (db_,
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
    this->bindText (parser_.uri_)
    && this->bindText (parser_.contact_owner_)
    && this->bindInteger (parser_.time_begin_)
    && this->bindInteger (parser_.time_end_)
    && this->bindText (parser_.first_name_)
    && this->bindText (parser_.middle_initial_)
    && this->bindText (parser_.last_name_)
    && this->bindText (parser_.rank_)
    && this->bindText (parser_.call_sign_)
    && this->bindText (parser_.branch_)
    && this->bindText (parser_.unit_)
    && this->bindText (parser_.email_)
    && this->bindText (parser_.phone_);
}


