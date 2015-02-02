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
