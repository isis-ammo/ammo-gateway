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

#include "LdapConfigurationManager.h"
#include "json/value.h"
#include "log.h"

static const char *LDAP_CONFIG_FILE = "LdapPluginConfig.json";

using namespace std;

LdapConfigurationManager *LdapConfigurationManager::sharedInstance = NULL;

//============================================================
//
// Default constructor
//
// Use the default config file if none specified
//============================================================
LdapConfigurationManager::LdapConfigurationManager() : ConfigurationManager(LDAP_CONFIG_FILE)
{
  init();
}

void LdapConfigurationManager::init()
{
  ldapServerAddress = "localhost";
  ldapServerPort = 389;
  ldapUsername = "cn=Manager,dc=ammo,dc=tdm";
  ldapPassword = "ammmo";
}

void LdapConfigurationManager::decode(const Json::Value& root)
{
  CM_DecodeString ( root, "LdapServerAddress", ldapServerAddress );
  CM_DecodeInt    ( root, "LdapServerPort",    ldapServerPort    );
  CM_DecodeString ( root, "LdapUsername",      ldapUsername      );
  CM_DecodeString ( root, "LdapPassword",      ldapPassword      );

  LOG_INFO("LDAP Plugin Configuration: ");
  LOG_INFO("  Ldap Server Address: " << ldapServerAddress);
  LOG_INFO("  Ldap Server Port: " << ldapServerPort);
  LOG_INFO("  Ldap Username: " << ldapUsername);
  LOG_INFO("  Ldap Password: " << ldapPassword);
}

//============================================================
//
// getLdapServerAddress()
//
//============================================================
std::string LdapConfigurationManager::getLdapServerAddress()
{
  return ldapServerAddress;
}

//============================================================
//
// getLdapServerPort()
//
//============================================================
int LdapConfigurationManager::getLdapServerPort()
{
  return ldapServerPort;
}

//============================================================
//
// getLdapUsername()
//
//============================================================
std::string LdapConfigurationManager::getLdapUsername()
{
  return ldapUsername;
}

//============================================================
//
// getLdapPassword()
//
//============================================================
std::string LdapConfigurationManager::getLdapPassword()
{
  return ldapPassword;
}

//============================================================
//
// getInstance()
//
//============================================================
LdapConfigurationManager* LdapConfigurationManager::getInstance()
{
  if(sharedInstance == NULL)
    {
      sharedInstance = new LdapConfigurationManager();
	  sharedInstance->populate();
    }
  return sharedInstance;
}
