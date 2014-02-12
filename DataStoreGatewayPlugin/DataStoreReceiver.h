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

