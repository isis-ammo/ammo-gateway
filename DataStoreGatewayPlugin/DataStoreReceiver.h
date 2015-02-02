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

#ifndef DATA_STORE_RECEIVER_H
#define DATA_STORE_RECEIVER_H

#include "GatewayConnector.h"

#include "DataStoreDispatcher.h"

struct sqlite3;

class DataStoreReceiver : public ammo::gateway::DataPushReceiverListener,
					                public ammo::gateway::GatewayConnectorDelegate,
                          public ammo::gateway::PullRequestReceiverListener

{
public:
  DataStoreReceiver (void);
  ~DataStoreReceiver (void);
	
  // GatewayConnectorDelegate methods
  virtual void onConnect (ammo::gateway::GatewayConnector *sender);
  virtual void onDisconnect (ammo::gateway::GatewayConnector *sender);
  
  // DataPushReceiverListener methods
  virtual void onPushDataReceived (ammo::gateway::GatewayConnector *sender,
							                     ammo::gateway::PushData &pushData);
	
  // PullRequestReceiverListener methods
  virtual void onPullRequestReceived (ammo::gateway::GatewayConnector *sender,
                                      ammo::gateway::PullRequest &pullReq);
                               
  void db_filepath (const std::string &path);
  bool init (void);

private:
  // Create all directories that don't exist, expanding
  // environment variables if necessary.
  bool check_path (void);

private:
  // Dispatcher for pushes and pulls.
  DataStoreDispatcher dispatcher_;
  
  // Pointer to open database.
  sqlite3 *db_;
  
  // Set by the config manager.
  std::string db_filepath_;
};

#endif        //  #ifndef DATA_STORE_RECEIVER_H

