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

#ifndef DATA_STORE_UTILS_H
#define DATA_STORE_UTILS_H

#include <string>
#include <ace/Reactor.h>

#include "LibGatewayDataStore_Export.h"

namespace Json
{
  class Value;
}

struct sqlite3;
struct sqlite3_stmt;

// Collection of common low-level methods that can't be
// located in a single base class.
struct LibGatewayDataStore_Export DataStoreUtils
{
  static bool parseJson (const std::string &input,
                         Json::Value& root);

  static bool createTable (sqlite3 *db,
                           const char *tbl_string,
                           const char *msg);
  
  static bool bind_int (sqlite3 *db,
                        sqlite3_stmt *stmt,
                        unsigned int &slot,
                        int val);
      
  // Converts or reports error if string is malformed.               
  static bool bind_int (sqlite3 *db,
                        sqlite3_stmt *stmt,
                        unsigned int &slot,
                        const std::string &val);
                 
  static bool bind_double (sqlite3 *db,
                           sqlite3_stmt *stmt,
                           unsigned int &slot,
                           double val);
                 
  // Converts or reports error if string is malformed.               
  static bool bind_double (sqlite3 *db,
                           sqlite3_stmt *stmt,
                           unsigned int &slot,
                           const std::string &val);
                 
  static bool bind_text (sqlite3 *db,
                         sqlite3_stmt *stmt,
                         unsigned int &slot,
                         const std::string &text,
                         bool is_insert);
                  
  static bool bind_blob (sqlite3 *db,
                         sqlite3_stmt *stmt,
                         unsigned int &slot,
                         const void *val,
                         int size,
                         bool is_insert);
                         
  static bool safe_atol (const std::string &val,
                         long &result);
                         
  static bool safe_atof (const std::string &val,
                         double &result);
                         
  static void legalize_tbl_name (std::string &name);
};

#endif // #ifndef DATA_STORE_UTILS_H
