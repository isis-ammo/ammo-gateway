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
