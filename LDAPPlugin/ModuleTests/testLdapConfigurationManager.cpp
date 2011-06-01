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
