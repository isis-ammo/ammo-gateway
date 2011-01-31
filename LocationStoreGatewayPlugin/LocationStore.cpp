#include "LocationStore.h"

#include <iostream>

#include <sqlite3.h>

using namespace std;

LocationStoreReceiver::LocationStoreReceiver (void)
  : db_ (0),
    err_prefix_ ("LocationStoreReceiver::onDataReceived - ")
{
  sqlite3_open ("LocationStore_db.sql3", &this->db_);
	
  cout << "Opening location store database..." << endl << flush;

  const char *create_tbl_str =
	"CREATE TABLE IF NOT EXISTS the_table ("
	"uri TEXT,"
	"mime_type TEXT,"
	"origin_user TEXT,"
	"tv_sec INTEGER NOT NULL,"
	"tv_usec INTEGER,"
	"data BLOB)";
	
  char *db_err = 0;
	
  sqlite3_exec (this->db_, create_tbl_str, 0, 0, &db_err);
	
  if (db_err != 0)
	{
	  cerr << "Error creating location store database "
		   << "table - " << db_err  << endl << flush;
	}
}

LocationStoreReceiver::~LocationStoreReceiver (void)
{
  cout << "Closing location store database..." << endl << flush;
	
  sqlite3_close (this->db_);
}

void LocationStoreReceiver::onConnect (GatewayConnector * /* sender */)
{
}

void LocationStoreReceiver::onDisconnect (GatewayConnector * /* sender */)
{
}

void LocationStoreReceiver::onDataReceived (GatewayConnector * /* sender */,
										    std::string uri,
										    std::string mimeType,
										    std::vector<char> & data,
										    std::string originUser)
{
  ACE_Time_Value tv (ACE_OS::gettimeofday ());
  sqlite3_stmt *stmt;
	
  int status =
	sqlite3_prepare (this->db_,
					 "insert into the_table values (?,?,?,?,?,?)",
					 -1,
					 &stmt,
					 0);
	
  if (status != SQLITE_OK)
    {
	  cerr << this->err_prefix_ << "prep of sqlite statement failed: "
		   << this->ec_to_string (status) << endl << flush;
		
	  return;
	}
	
  status =
	sqlite3_bind_text (stmt,
					   1,
					   uri.c_str (),
					   uri.length (),
					   SQLITE_STATIC);
	
  if (status != SQLITE_OK)
    {
	  cerr << this->err_prefix_ << "URI bind failed: "
		   << this->ec_to_string (status) << endl << flush;
		
	  return;
	}
	
  status =
	sqlite3_bind_text (stmt,
					   2,
					   mimeType.c_str (),
					   mimeType.length (),
					   SQLITE_STATIC);
	
  if (status != SQLITE_OK)
    {
	  cerr << this->err_prefix_ << "MIME type bind failed: "
		   << this->ec_to_string (status) << endl << flush;
		
	  return;
	}
	
  status =
	sqlite3_bind_text (stmt,
					   3,
					   originUser.c_str (),
					   originUser.length (),
					   SQLITE_STATIC);
	
  if (status != SQLITE_OK)
    {
		cerr << this->err_prefix_ << "origin user bind failed: "
		<< this->ec_to_string (status) << endl << flush;
		
		return;
	}
	
  status =
	sqlite3_bind_int (stmt,
					  4,
					  tv.sec ());
	
  if (status != SQLITE_OK)
    {
      cerr << this->err_prefix_ << "timestamp sec bind failed: "
		   << this->ec_to_string (status) << endl << flush;
		
	  return;
	}
	
  status =
	sqlite3_bind_int (stmt,
					  5,
					  tv.usec ());
	
  if (status != SQLITE_OK)
    {
	  cerr << this->err_prefix_ << "timestamp usec bind failed: "
		   << this->ec_to_string (status) << endl << flush;
		
	  return;
	}
	
  status =
	sqlite3_bind_blob (stmt,
					   6,
					   data.get_allocator ().address (*data.begin ()),
					   data.size (),
					   SQLITE_STATIC);
	
  if (status != SQLITE_OK)
    {
	  cerr << this->err_prefix_ << "data bind failed: "
		   << this->ec_to_string (status) << endl << flush;
		
	  return;
	}
	
  status = sqlite3_step (stmt);
	
  if (status != SQLITE_DONE)
    {
	  cerr << this->err_prefix_ << "insert operation failed: "
		   << this->ec_to_string (status) << endl << flush;
		
	  return;
    }
}

const char * const
LocationStoreReceiver::ec_to_string (int error_code) const
{
  switch (error_code)
	{
	  case SQLITE_OK:
		return "Successful result";
	  case SQLITE_ERROR:
		return "SQL error or missing database";
	  case SQLITE_INTERNAL:
		return "An internal logic error in SQLite";
	  case SQLITE_PERM:
		return "Access permission denied";
	  case SQLITE_ABORT:
		return "Callback routine requested an abort";
	  case SQLITE_BUSY:
		return "The database file is locked";
	  case SQLITE_LOCKED:
		return "A table in the database is locked";
	  case SQLITE_NOMEM:
		return "A malloc() failed";
	  case SQLITE_READONLY:
		return "Attempt to write a readonly database";
	  case SQLITE_INTERRUPT:
		return "Operation terminated by sqlite_interrupt()";
	  case SQLITE_IOERR:
		return "Some kind of disk I/O error occurred";
	  case SQLITE_CORRUPT:
		return "The database disk image is malformed";
	  case SQLITE_NOTFOUND:
		return "(Internal Only) Table or record not found";
	  case SQLITE_FULL:
		return "Insertion failed because database is full";
	  case SQLITE_CANTOPEN:
		return "Unable to open the database file";
	  case SQLITE_PROTOCOL:
		return "Database lock protocol error";
	  case SQLITE_EMPTY:
		return "(Internal Only) Database table is empty";
	  case SQLITE_SCHEMA:
		return "The database schema changed";
	  case SQLITE_TOOBIG:
		return "Too much data for one row of a table";
	  case SQLITE_CONSTRAINT:
		return "Abort due to constraint violation";
	  case SQLITE_MISMATCH:
		return "Data type mismatch";
	  case SQLITE_MISUSE:
		return "Library used incorrectly";
	  case SQLITE_NOLFS:
		return "Uses OS features not supported on host";
	  case SQLITE_AUTH:
		return "Authorization denied";
	  case SQLITE_ROW:
		return "sqlite_step() has another row ready";
	  case SQLITE_DONE:
		return "sqlite_step() has finished executing";
	  default:
		return "Unknown error";
    }
}
