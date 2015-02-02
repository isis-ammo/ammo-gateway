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
