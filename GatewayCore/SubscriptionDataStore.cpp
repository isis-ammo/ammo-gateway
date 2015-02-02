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
