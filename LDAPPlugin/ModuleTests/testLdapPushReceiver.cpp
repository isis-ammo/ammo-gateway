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

#include <iostream>
#include <string>

#include <ctime>

//#include "ace/Reactor.h"

//#include "LdapPushReceiver.h"
//#include "GatewayConnector.h"

#include "LdapConfigurationManager.h"


using namespace std;

const string CONTACT_MIME_TYPE = "ammo/edu.vu.isis.ammo.launcher.contact";
const string CONTACT_PULL_MIME_TYPE = "ammo/edu.vu.isis.ammo.launcher.contact_pull";


int main(int argc, char **argv) {  
  LdapConfigurationManager::getInstance();
  
  cout << "Creating gateway connector..." << endl << flush;
  
  //  LdapPushReceiver *pushReceiver = new LdapPushReceiver();
  
  //  GatewayConnector *gatewayConnector = new GatewayConnector(pushReceiver);
  
  //  cout << "Registering interest in " << CONTACT_MIME_TYPE << endl << flush;
  //  gatewayConnector->registerDataInterest(CONTACT_MIME_TYPE, pushReceiver);
  
  //  cout << "Registering interest in " << CONTACT_PULL_MIME_TYPE << endl << flush;
  //  gatewayConnector->registerPullInterest(CONTACT_PULL_MIME_TYPE, pushReceiver);
  
  //  //Get the process-wide ACE_Reactor (the one the acceptor should have registered with)
  //  ACE_Reactor *reactor = ACE_Reactor::instance();
  //  cout << "Starting event loop..." << endl << flush;
  //  reactor->run_reactor_event_loop();
  
  return 0;
}
