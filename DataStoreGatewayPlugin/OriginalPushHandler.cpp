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

#include <sqlite3.h>

#include "ace/OS_NS_sys_time.h"

#include "GatewayConnector.h"
#include "log.h"

#include "OriginalPushHandler.h"
#include "DataStoreUtils.h"

OriginalPushHandler::OriginalPushHandler (sqlite3 *db,
                                          const ammo::gateway::PushData &pd)
  : PushHandler (db, pd)
{
}

bool
OriginalPushHandler::handlePush (void)
{
  ACE_Time_Value tv (ACE_OS::gettimeofday ());
	
  int status =
	  sqlite3_prepare (db_,
			               "insert into data_table values (?,?,?,?,?,?)",
			               -1,
			               &stmt_,
			               0);
	
  if (status != SQLITE_OK)
    {
      LOG_ERROR ("Data push - "
                 << "prep of sqlite statement failed: "
		             << sqlite3_errmsg (db_));
		
      return false;
    }
    
  unsigned int index = 1;
  
  bool good_binds =
    DataStoreUtils::bind_text (db_, stmt_, index, pd_.uri, true)
    && DataStoreUtils::bind_text (db_, stmt_, index, pd_.mimeType, true)
    && DataStoreUtils::bind_text (db_, stmt_, index, pd_.originUsername, true)
    && DataStoreUtils::bind_int (db_, stmt_, index, tv.sec ())
    && DataStoreUtils::bind_int (db_, stmt_, index, tv.usec ())
    && DataStoreUtils::bind_blob (db_, stmt_, index, pd_.data.data (), pd_.data.length (), true);

  if (good_binds)
    {	
      status = sqlite3_step (stmt_);
	
      if (status != SQLITE_DONE)
        {
          LOG_ERROR ("Data push - "
                     << "insert operation failed: "
                     << sqlite3_errmsg (db_));
		
          return false;
        }
    }

  return good_binds;
}
