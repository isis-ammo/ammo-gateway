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

#include "SubscriptionDataStore.h"

using namespace std;

SubscriptionDataStore::SubscriptionDataStore() : subscriptions() {
  
}

SubscriptionDataStore::SubscriptionMap SubscriptionDataStore::getSubscriptionMap() {
  return subscriptions;
}

SubscriptionDataStore::SubscriptionHandlerSet SubscriptionDataStore::getHandlersForType(const std::string &typeName) {
  SubscriptionHandlerSet matchingHandlers;

  for(SubscriptionMap::iterator it = subscriptions.begin(); it!= subscriptions.end(); it++) {
    if(typeName.find(it->first) == 0) { //looking for subscribers which are a prefix of mimeType
      matchingHandlers.insert(it->second);
    }
  }
  
  return matchingHandlers;
}

void SubscriptionDataStore::subscribe(const std::string &typeName, const std::string &handlerName) {
  subscriptions.insert(SubscriptionMap::value_type(typeName, handlerName));
}

void SubscriptionDataStore::unsubscribe(const std::string &typeName, const std::string &handlerName) {
  SubscriptionMap::iterator it;
  pair<SubscriptionMap::iterator,SubscriptionMap::iterator> handlerIterators;
  
  handlerIterators = subscriptions.equal_range(typeName);
  
  bool foundSubscription = false;
  
  for(it = handlerIterators.first; it != handlerIterators.second;) {
    //need to increment the iterator *before* we erase it, because erasing it
    //invalidates the iterator (it doesn't invalidate other iterators in the list,
    //though)
    SubscriptionMap::iterator eraseIter = it++;
    
    if(handlerName == (*eraseIter).second) {
      //LOG_TRACE("Removing an element");
      subscriptions.erase(eraseIter);
      foundSubscription = true;
      break;
    }
  }
}
