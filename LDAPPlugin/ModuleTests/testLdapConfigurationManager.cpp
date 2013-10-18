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

#include "LdapConfigurationManager.h"

using namespace std;

int main(int argc, char **argv) {  
  cout << "Test of LdapConfigurationManager ..." << endl << flush;

  // Test: have default values been set correctly?
  {
    cout << "Test #1:  ..." ;

    LdapConfigurationManager* t = LdapConfigurationManager::getInstance();

    if ((t->getLdapBaseAddress() == "localhost") && 
	(t->getLdapUsername() == "cn=Manager,dc=ammo,dc=tdm") && 
	(t->getLdapPassword() == "ammmo"))
      {
	cout << "*** PASSED ***";
      }
    else
      {
	cout << "*** FAILED ***";
      }
    cout << endl << flush;
  }

  // Test: set values from file
  {
    cout << "Test #2:  ..." ;
    LdapConfigurationManager* t = new LdapConfigurationManager("data/testLdapConfigurationManager_Test2.json");
    
    if ((t->getLdapBaseAddress() == "ldap://localhost") && 
	(t->getLdapUsername() == "cn=librarian,dc=books,dc=info") && 
	(t->getLdapPassword() == "AmmoRfun"))
      {
	cout << "*** PASSED ***";
      }
    else
      {
	cout << "*** FAILED ***";
      }
    cout << endl << flush;

    delete t;
  }
  
  return 0;
}
