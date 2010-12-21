#include "LdapPushReceiver.h"

#include "LdapConfigurationManager.h"

#include "json/json.h"

#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>
#include <boost/tokenizer.hpp>
#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>

#include <ldap.h>

using namespace std;

/*
 * LDAPPlugin
 * Dataflow Scenarios:
 * 1. User on the device edits his own contact information --> push to LDAP Server (ldapadd/modify ...)
 * 2. User on the device queries for a specific contact --> pull from LDAP server (ldapsearch)
 */

/*
 * Push Reciever Consstructor
 * a) Read Configuration
 * b) Connect to the LDAP Server
 * TBD: where / when do we authenticate with LDAP Server with admin user credentials
 */


LdapPushReceiver::LdapPushReceiver() : ldapServer(0) {


 LdapConfigurationManager *config = LdapConfigurationManager::getInstance();

 string ldapAddress = config->getLdapBaseAddress();
 cout << "Attempting Connection to LDAP Server @:" << ldapAddress << endl;

 int ret = ldap_initialize( &ldapServer, ldapAddress.c_str() );


 if (ret != LDAP_SUCCESS) {
   cout << "Error Initializing LDAP Library: " << ldap_err2string(ret) << endl;
   return;
 }

 int version = 3;
 ldap_set_option(ldapServer, LDAP_OPT_PROTOCOL_VERSION, &version);

 string basedn = config->getLdapUsername();
 string passwd = config->getLdapPassword();

 cout << "Attempting bind operation w/user : " << basedn << " ... " << endl;
 LDAPControl *serverctrls=NULL, *clientctrls=NULL;
 struct berval *servercredp=NULL;
 struct berval creds;
 creds.bv_val = strdup( passwd.c_str() );
 creds.bv_len = passwd.length();

 ret = ldap_sasl_bind_s( ldapServer,
			 basedn.c_str(),
			 LDAP_SASL_SIMPLE, // simple authentication
			 &creds,
			 &serverctrls,
			 &clientctrls,
			 &servercredp);

 if (ret != LDAP_SUCCESS) {
   cout << "Error Binding to LDAP Server: " << ldap_err2string(ret) << endl;
 }

 cout << "Connected to LDAP Server @" << ldapAddress << endl;

}

void LdapPushReceiver::onConnect(GatewayConnector *sender) {
  
}

void LdapPushReceiver::onDisconnect(GatewayConnector *sender) {
  
}

/*
 * Data Push from the Device
 * User has edited his own contact information - do an ldapadd/modify on the LDAPServer
 */

void LdapPushReceiver::onDataReceived(GatewayConnector *sender, std::string uri, std::string mimeType, std::vector<char> &data) {
  cout << "Got data." << endl;
  cout << "  URI: " << uri << endl;
  cout << "  Mime type: " << mimeType << endl;

  if(mimeType == "application/vnd.edu.vu.isis.ammmo.launcher.contact_edit") {
    cout << "Extracting JSON metadata..." << endl << flush;
    
    unsigned int jsonEnd = 0;
    for(vector<char>::iterator it = data.begin(); it != data.end(); it++) {
      jsonEnd++;
      if((*it) == 0) {
        break;
      }
    }
    
    string json(&data[0], jsonEnd);
    
    cout << "JSON string: " << json << endl;
    
    Json::Value jsonRoot;
    Json::Reader jsonReader;
    
    bool parseSuccess = jsonReader.parse(json, jsonRoot);
    
    if(!parseSuccess) {
      cout << "JSON parsing error:" << endl;
      cout << jsonReader.getFormatedErrorMessages() << endl;
      return;
    }
    
    cout << "Parsed JSON: " << jsonRoot.toStyledString() << endl;

    LdapContact contact;
    contact.name = jsonRoot["name"].asString();
    contact.middle_initial = jsonRoot["middle_initial"].asString();
    contact.lastname = jsonRoot["lastname"].asString();
    contact.rank = jsonRoot["rank"].asString();
    contact.callsign = jsonRoot["callsign"].asString();
    contact.branch = jsonRoot["branch"].asString();
    contact.unit = jsonRoot["unit"].asString();
    contact.email = jsonRoot["email"].asString();
    contact.phone = jsonRoot["phone"].asString();

    editContact(contact);
  } else {
    cout << "ERROR!  Invalid Mime Type." << endl << flush;
  }
  
}

/*
 * Pull Request
 * Query containing LDAP search parameters
 */

void LdapPushReceiver::onDataReceived(GatewayConnector *sender, 
			      std::string requestUid, std::string pluginId,
			      std::string mimeType, std::string query,
			      std::string projection, unsigned int maxResults,
			      unsigned int startFromCount, bool liveQuery) {

  string response = "asdf";
  
  vector<string> jsonResults;
  
  get(query, jsonResults);
  
  for(vector<string>::iterator it = jsonResults.begin(); it != jsonResults.end(); it++) {
    
    vector<char> data(it->begin(), it->end());
    
    sender->pullResponse(requestUid, pluginId, mimeType, "ammmo-demo:test-object", data);
  }
}

bool LdapPushReceiver::get(std::string query, std::vector<std::string> &jsonResults) {
  LdapConfigurationManager *config = LdapConfigurationManager::getInstance();

  LDAPMessage *results;
  std::string filter = "&(objectClass=x-Military)";
  
  // build the filter based on query expression
  // query = comma-separated field-name / value pairs
  boost::char_separator<char> sep(",");
  boost::tokenizer< boost::char_separator<char> > tokens(query, sep);

  BOOST_FOREACH(string t, tokens) {
    string::size_type epos=t.find('=');
    string attr,val;
    if (epos != string::npos) {
      attr = t.substr(0,epos);
      val = t.substr(epos+1);
    }
    boost::trim(attr);
    boost::trim(val);

    if (attr != "" && val != "")
      filter += ( string("(") +  attr + string("=") + val + string(")") );
  }

  struct timeval timeout = { 1, 0 }; // 1 sec timeout
  LDAPControl *serverctrls, *clientctrls;
  char *attrs[] = { "*" };

  int ret = ldap_search_ext_s(ldapServer,
			      "dc=transapp,dc=darpa,dc=mil", /* LDAP search base dn (distinguished name) */
			      LDAP_SCOPE_SUBTREE, /* scope - root and all descendants */
			      filter.c_str(), /* filter - query expression */
			      attrs, /* requested attributes (white-space seperated list, * = ALL) */
			      0, /* attrsonly - if we only want to get attribut types */
			      &serverctrls,
			      &clientctrls,
			      &timeout,
			      10,
			      &results);

  if (ret != LDAP_SUCCESS)   {
    cout << "LDAP Search failed: " << ldap_err2string(ret) << endl;
    return false;
  }


  /* process the results */
  LDAPMessage *entry = ldap_first_entry(ldapServer, results);
  while(entry)  {
    jsonResults.push_back( jsonForObject(entry) );
    entry = ldap_next_entry(ldapServer, entry);
  }

  return true;
}

string LdapPushReceiver::jsonForObject(LDAPMessage *entry) {
    Json::Value root;
    struct berval **vals;
    
    // name
    vals = ldap_get_values_len(ldapServer, entry, "cn");
    string cn = vals[0]->bv_val; // there must be only one name
    ldap_value_free_len(vals);

    vector<string> parts;
    boost::split(parts, cn, boost::is_any_of(" "));

    root["name"] = parts[0];
    if (parts.size() >= 2)  {
      root["middle_initial"] = parts[1];
      root["lastname"] = parts[2];
    }

    // rank
    vals = ldap_get_values_len(ldapServer, entry, "x-Rank");
    root["rank"] = vals[0]->bv_val;
    ldap_value_free_len(vals);

    // callsign
    vals = ldap_get_values_len(ldapServer, entry, "x-Callsign");
    root["callsign"] = vals[0]->bv_val;
    ldap_value_free_len(vals);
    
    // branch
    vals = ldap_get_values_len(ldapServer, entry, "x-Branch");
    root["branch"] = vals[0]->bv_val;
    ldap_value_free_len(vals);
    
    // unit
    vals = ldap_get_values_len(ldapServer, entry, "x-Unit");
    root["unit"] = vals[0]->bv_val;
    ldap_value_free_len(vals);
    
    // email
    vals = ldap_get_values_len(ldapServer, entry, "mail");
    root["email"] = vals[0]->bv_val;	// use only the first mail
    ldap_value_free_len(vals);
    
    // phone
    vals = ldap_get_values_len(ldapServer, entry, "phone");
    root["phone"] = vals[0]->bv_val;	// use only the first phone
    ldap_value_free_len(vals);
    
    // insignia - photo not to serialize with JSON
    
    cout << "JSON: " << root.toStyledString() << endl;
    return root.toStyledString();

}


bool LdapPushReceiver::editContact(const LdapContact&) {
}



static int write_callback(char *data, size_t size, size_t nmemb, std::string *writerData) {
  if(writerData == NULL) {
    return 0;
  }
  
  writerData->append(data, size*nmemb);
  
  return size * nmemb;
}
