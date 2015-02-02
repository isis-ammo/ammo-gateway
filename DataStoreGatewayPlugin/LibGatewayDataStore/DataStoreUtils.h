/* Copyright (c) 2010-2015 Vanderbilt University
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
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
