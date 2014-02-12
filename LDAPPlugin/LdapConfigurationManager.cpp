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
