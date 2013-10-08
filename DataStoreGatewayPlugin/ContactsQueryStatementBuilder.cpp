/* Copyright (C) 2010-2013 Vanderbilt University
 *  
 * This software was developed by the Institute for Software Integrated
 * Systems (ISIS) at Vanderbilt University, Tennessee, USA for the
 * Transformative Apps program under DARPA, Contract # HR011-10-C-0175 and
 * Contract # HR0011-12-C-0109.
 *  
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this data, including any software or models in source or binary
 * form, as well as any drawings, specifications, and documentation
 * (collectively "the Data"), to deal in the Data without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Data, and to
 * permit persons to whom the Data is furnished to do so, subject to the
 * following conditions:
 *  
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Data.
 *  
 * THE DATA IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS, SPONSORS, DEVELOPERS, CONTRIBUTORS, OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE DATA OR THE USE OR OTHER DEALINGS IN THE DATA.
 */

#include <sqlite3.h>

#include <ace/OS_NS_sys_time.h>

#include "log.h"

#include "ContactsQueryStatementBuilder.h"
#include "DataStoreUtils.h"

ContactsQueryStatementBuilder::ContactsQueryStatementBuilder (
      const std::string &params,
      sqlite3 *db)
  : QueryStatementBuilder (params, db, "SELECT * FROM "),
    bind_index_ (1)
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
  return
    DataStoreUtils::bind_text (db_, stmt_, bind_index_, parser_.uri_, false)
    && DataStoreUtils::bind_text (db_, stmt_, bind_index_, parser_.first_name_, false)
    && DataStoreUtils::bind_text (db_, stmt_, bind_index_, parser_.middle_initial_, false)
    && DataStoreUtils::bind_text (db_, stmt_, bind_index_, parser_.last_name_, false)
    && DataStoreUtils::bind_text (db_, stmt_, bind_index_, parser_.rank_, false)
    && DataStoreUtils::bind_text (db_, stmt_, bind_index_, parser_.call_sign_, false)
    && DataStoreUtils::bind_text (db_, stmt_, bind_index_, parser_.branch_, false)
    && DataStoreUtils::bind_text (db_, stmt_, bind_index_, parser_.unit_, false)
    && DataStoreUtils::bind_text (db_, stmt_, bind_index_, parser_.email_, false)
    && DataStoreUtils::bind_text (db_, stmt_, bind_index_, parser_.phone_, false);
}


