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

#ifndef CONNECTION_MANAGER_H
#define CONNECTION_MANAGER_H

#include <set>
#include <ace/Event_Handler.h>

class AndroidEventHandler;

class ConnectionManager : public ACE_Event_Handler {
public:
  static const int TIMEOUT_TIME_SECONDS = 60;
  static ConnectionManager *getInstance();

  ConnectionManager();

  void registerConnection(AndroidEventHandler *handler);
  void unregisterConnection(AndroidEventHandler *handler);
  
  void checkTimeouts();
  
  //Event handler methods (called by the reactor when our timer expires)
  int handle_timeout(const ACE_Time_Value &currentTime, const void *act = 0);

private:
  static ConnectionManager *sharedInstance;
  
  typedef std::set<AndroidEventHandler *> EventHandlerSet;
  EventHandlerSet eventHandlers;
};

#endif //CONNECTION_MANAGER_H
