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

#include <algorithm>

#include "ace/Connector.h"
#include "ace/SOCK_Connector.h"

#include "ace/OS_NS_stdlib.h"
#include <ace/OS_NS_stdlib.h>
#include <ace/OS_NS_sys_time.h>

#include "json/value.h"
#include "json/reader.h"
#include "log.h"

#include "DataStoreUtils.h"

bool
DataStoreUtils::parseJson (const std::string &input,
                           Json::Value& root)
{
  Json::Reader jsonReader;
  bool parseSuccess = jsonReader.parse (input, root);

  if (!parseSuccess)
    {
      LOG_ERROR ("JSON parsing error: "
                 << jsonReader.getFormatedErrorMessages ());
                 
      return false;
    }

//  LOG_DEBUG ("Parsed JSON: " << root.toStyledString ());
  return true;
}

bool
DataStoreUtils::createTable (sqlite3 *db,
                             const char *tbl_string,
                             const char *msg)
{
  char *db_err = 0;
	
  sqlite3_exec (db, tbl_string, 0, 0, &db_err);
	
  if (db_err != 0)
	  {
	    LOG_ERROR ("Data Store Service " << msg << " table - " << db_err);
	    sqlite3_free (db_err);
			return false;
	  }
	  
  return true;
}

bool
DataStoreUtils::bind_int (sqlite3 *db,
                          sqlite3_stmt *stmt,
                          unsigned int &slot,
                          int val)
{
  int status = sqlite3_bind_int (stmt, slot++, val);
	
  if (status != SQLITE_OK)
    {
      LOG_ERROR ("sqlite3_bind_int() at slot " << slot
                 << " with value " << val
                 << " failed: " << sqlite3_errmsg (db));
		
      return false;
    }
   
  return true;
}
                 
bool
DataStoreUtils::bind_int (sqlite3 *db,
                          sqlite3_stmt *stmt,
                          unsigned int &slot,
                          const std::string &val)
{
  if (val.empty ())
    {
      return true;
    }

  long conversion = 0;
  
  if (safe_atol (val, conversion))
    {
      return bind_int (db, stmt, slot, conversion);
    }
   
  return false;
}

bool
DataStoreUtils::bind_double (sqlite3 *db,
                             sqlite3_stmt *stmt,
                             unsigned int &slot,
                             double val)
{
  int status = sqlite3_bind_double (stmt, slot, val);
	
  if (status != SQLITE_OK)
    {
      LOG_ERROR ("sqlite3_bind_double() at slot " << slot
                 << " with value " << val
                 << " failed: " << sqlite3_errmsg (db));
		
		  ++slot;
      return false;
    }
    
  ++slot;  
  return true;
}
                 
bool
DataStoreUtils::bind_double (sqlite3 *db,
                             sqlite3_stmt *stmt,
                             unsigned int &slot,
                             const std::string &val)
{
  if (val.empty ())
    {
      return true;
    }

  double conversion = 0;
  
  if (safe_atof (val, conversion))
    {
      return bind_double (db, stmt, slot, conversion);
    }
    
  return false;
}

bool
DataStoreUtils::bind_text (sqlite3 *db,
                           sqlite3_stmt *stmt,
                           unsigned int &slot,
                           const std::string &text,
                           bool is_insert)
{
  // For a data push, we want to go ahead and store an empty string,
  // see comment below for more details.
  if (text.empty () && !is_insert)
    {
      return true;
    }
    
  // If the text string is empty, passing its length (0), will
  // result in a NULL char* being extracted on a match. Passing
  // a 1 instead will produce an empty string upon extraction,
  // which is what we want to put into the Json::Value response.
  // Also, insert statement values are copied right away, so
  // they don't have to hang around, but query statement values
  // take a longer path, and the original value goes out of
  // scope, so we need to specify SQLITE_STATIC in that case.
  int status =
	  sqlite3_bind_text (stmt,
					             slot++,
					             text.c_str (),
					             (text.empty () ? 1 : text.length ()),
					             (is_insert ? SQLITE_TRANSIENT : SQLITE_STATIC));
	
  if (status != SQLITE_OK)
    {
      LOG_ERROR ("sqlite3_bind_text() at slot " << slot - 1
                 << " with value " << text.c_str ()
                 << " failed: " << sqlite3_errmsg (db));
		
      return false;
    }
	
	return true;
}
                  
bool
DataStoreUtils::bind_blob (sqlite3 *db,
                           sqlite3_stmt *stmt,
                           unsigned int &slot,
                           const void *val,
                           int size,
                           bool is_insert)
{
  // Insert statement values are copied right away, so
  // they don't have to hang around, but query statement values
  // take a longer path, and the original value goes out of
  // scope, so we need to specify SQLITE_STATIC in that case.
  // Not that we're likely to query on a binary blob value,
  // but just in case....
  int status =
	  sqlite3_bind_blob (stmt,
					             slot++,
					             val,
					             size,
					             (is_insert ? SQLITE_TRANSIENT : SQLITE_STATIC));
	
  if (status != SQLITE_OK)
    {
      LOG_ERROR ("sqlite3_bind_blob() at slot " << slot - 1
                 << " failed: " << sqlite3_errmsg (db));
		
      return false;
    }
    
  return true;
}
                         
bool
DataStoreUtils::safe_atol (const std::string &val,
                           long &result)
{
  if (val.find_first_not_of ("-0123456789") != std::string::npos)
    {
      LOG_ERROR ("string '"
                 << val.c_str ()
                 << "' is malformed integer");

      return false;
    }
    
  result = ACE_OS::atol (val.c_str ());
  return true;
}

bool
DataStoreUtils::safe_atof (const std::string &val,
                           double &result)
{
  if (val.find_first_not_of ("-0123456789.") != std::string::npos)
    {
      LOG_ERROR ("string '"
                 << val.c_str ()
                 << "' is malformed double");

      return false;
    }
    
  result = ACE_OS::atof (val.c_str ());
  return true;
}

void
DataStoreUtils::legalize_tbl_name (std::string &name)
{
  std::replace (name.begin (), name.end (), '.', '_');
  std::replace (name.begin (), name.end (), ':', '_');
  std::replace (name.begin (), name.end (), '/', '_');
}


