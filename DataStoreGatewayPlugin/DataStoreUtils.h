#ifndef DATA_STORE_UTILS_H
#define DATA_STORE_UTILS_H

#include <string>

namespace Json
{
  class Value;
}

class sqlite3;
class sqlite3_stmt;

// Collection of common low-level methods that can't be
// located in a single base class.
struct DataStoreUtils
{
  static bool parseJson (const std::string &input,
                         Json::Value& root);

  static bool createTable (sqlite3 *db,
                           const char *tbl_string,
                           const char *msg);
  
  static bool bind_int (sqlite3 *db,
                        sqlite3_stmt *stmt,
                        int slot,
                        int val);
      
  // Converts or reports error if string is malformed.               
  static bool bind_int (sqlite3 *db,
                        sqlite3_stmt *stmt,
                        int slot,
                        const std::string &val);
                 
  static bool bind_double (sqlite3 *db,
                           sqlite3_stmt *stmt,
                           int slot,
                           double val);
                 
  // Converts or reports error if string is malformed.               
  static bool bind_double (sqlite3 *db,
                           sqlite3_stmt *stmt,
                           int slot,
                           const std::string &val);
                 
  static bool bind_text (sqlite3 *db,
                         sqlite3_stmt *stmt,
                         int slot,
                         const std::string &text,
                         bool is_insert);
                  
  static bool bind_blob (sqlite3 *db,
                         sqlite3_stmt *stmt,
                         int slot,
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
