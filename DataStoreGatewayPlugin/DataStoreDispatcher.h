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

#ifndef DATA_STORE_DISPATCHER_H
#define DATA_STORE_DISPATCHER_H

class QueryHandler;
struct sqlite3;
class DataStoreConfigManager;

namespace ammo
{
  namespace gateway
  {
    class PullRequest;
    class GatewayConnector;
  }
}

class DataStoreDispatcher
{
public:
  DataStoreDispatcher (void);
  
  void dispatchPushData (sqlite3 *db,
                         ammo::gateway::PushData &pd);
                       
  void dispatchPullRequest (sqlite3 *db,
                            ammo::gateway::GatewayConnector *sender,
                            ammo::gateway::PullRequest &pr);
                            
  void set_cfg_mgr (DataStoreConfigManager *cfg_mgr);
                            
private:
  DataStoreConfigManager *cfg_mgr_;
};

#endif /* DATA_STORE_DISPATCHER_H */
