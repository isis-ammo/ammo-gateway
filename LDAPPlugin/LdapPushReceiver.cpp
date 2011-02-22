#include "LdapPushReceiver.h"

#include "LdapConfigurationManager.h"

#include "json/json.h"

#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>

#include <ldap.h>
#include "decode.h"
#include <fstream>

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

 int version = LDAP_VERSION3;
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

void LdapPushReceiver::onDataReceived(GatewayConnector *sender, std::string uri, std::string mimeType, std::vector<char> &data, std::string originUser) {
  cout << "Got data." << endl;
  cout << "  URI: " << uri << endl;
  cout << "  Mime type: " << mimeType << endl;

  if(mimeType == "application/vnd.edu.vu.isis.ammo.launcher.contact_edit") {
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
    
    sender->pullResponse(requestUid, pluginId, mimeType, "ammo-demo:test-object", data);
  }
}

bool LdapPushReceiver::get(std::string query, std::vector<std::string> &jsonResults) {
  LdapConfigurationManager *config = LdapConfigurationManager::getInstance();

  LDAPMessage *results;
  //std::string filter = "(& (objectClass=x-MilitaryPerson) (objectClass=inetOrgPerson)";
  std::string filter = "(& (objectClass=MilitaryPerson) ";

  
  // build the filter based on query expression
  // query = comma-separated field-name / value pairs

  // <NEW_NON_BOOST>
  {
    // Divide the query string into tokens (separated by '|')
    char separator = '|';
    std::vector<std::string> results;

    std::string::size_type pos1 = 0;
    std::string::size_type pos2 = 0;
    while (pos2 != std::string::npos)
      {
	pos2 = query.find_first_of(separator, pos1);
	std::string t;
	if (pos2 != std::string::npos)
	  {
	    t = query.substr(pos1, pos2-pos1);
	    pos1 = (pos2 + 1);
	  }
	else
	  {
	    t = query.substr(pos1);
	  }
	if (t.size() != 0) results.push_back(t);
      }
    
    // Now divide tokens into key-value pairs (separated by '=')
    std::vector<std::string>::iterator p;
    for (p=results.begin(); p < results.end(); p++)
      {
	std::string t = (*p);
	std::string::size_type epos = t.find('=');
	std::string attr,val;
	if (epos != std::string::npos)
	  {
	    attr = t.substr(0,epos);
	    val = t.substr(epos+1);
	  }
	if (attr != "" && val != "") 
	  {
	    filter += ( std::string("(") +  attr + std::string("=") + val + std::string(") ") );
	  }
      }
  } 
  // </NEW_NON_BOOST>

  filter += " )";

  //changed the timeout to 5 sec from 1 ... since jpeg files are taking long
  struct timeval timeout = { 5, 0 }; 

  LDAPControl *serverctrls = NULL, *clientctrls = NULL;
  char *attrs[] = { "*", NULL };
  
  cout << "LDAP Starting Search for: " << filter << endl;

  int ret = ldap_search_ext_s(ldapServer,
			      "dc=transapps,dc=darpa,dc=mil", /* LDAP search base dn (distinguished name) */
			      LDAP_SCOPE_SUBTREE, /* scope - root and all descendants */
			      filter.c_str(), /* filter - query expression */
			      attrs, /* requested attributes (white-space seperated list, * = ALL) */
			      0, /* attrsonly - if we only want to get attribut types */
			      &serverctrls,
			      &clientctrls,
			      &timeout,
			      -1, // number of results : -1 = unlimited
			      &results);

  cout << "LDAP Return From Search for: " << filter << endl;

  if (ret != LDAP_SUCCESS)   {
    cout << "LDAP Search failed for " << filter << ": " << hex << ret << " - " << ldap_err2string(ret) << endl;
    return false;
  } else {
    cout << "LDAP Search Returned " << ldap_count_entries(ldapServer, results) << " results" << endl;
 }


  /* process the results */
  LDAPMessage *entry = ldap_first_entry(ldapServer, results);
  while(entry)  {
    jsonResults.push_back( jsonForObject(entry) );
    entry = ldap_next_entry(ldapServer, entry);
  }

  if (results) {
    // free results                                                                                                                                
    ldap_msgfree(results);
  }


  return true;
}

string LdapPushReceiver::jsonForObject(LDAPMessage *entry) {
    Json::Value root;
    struct berval **vals;
  

    // name
    vals = ldap_get_values_len(ldapServer, entry, "givenName");
    if (vals) {
      root["name"] = vals[0]->bv_val; // there must be only one name
      ldap_value_free_len(vals);
    }
    
    vals = ldap_get_values_len(ldapServer, entry, "sn");
    if (vals) {
      root["lastname"] = vals[0]->bv_val; // there must be only one name
      ldap_value_free_len(vals);
    }

    vals = ldap_get_values_len(ldapServer, entry, "initials");
    if (vals) {
      root["middle_initial"] = vals[0]->bv_val; // there must be only one name
      ldap_value_free_len(vals);
    }

    // rank
    vals = ldap_get_values_len(ldapServer, entry, "Rank");
    if (vals) {
      root["rank"] = vals[0]->bv_val;
      ldap_value_free_len(vals);
    }

    // callsign
    vals = ldap_get_values_len(ldapServer, entry, "Callsign");
    if (vals) {
      root["callsign"] = vals[0]->bv_val;
      ldap_value_free_len(vals);
    }
    
    // branch
    vals = ldap_get_values_len(ldapServer, entry, "Branch");
    if (vals) {
      root["branch"] = vals[0]->bv_val;
      ldap_value_free_len(vals);
    }
    
    // unit (generic)
    vals = ldap_get_values_len(ldapServer, entry, "Unit");
    if (vals) {
      root["unit"] = vals[0]->bv_val;
      ldap_value_free_len(vals);
    }
    
    // unit (specific, separated)
    vals = ldap_get_values_len(ldapServer, entry, "unitDivision");
    if (vals) {
      root["unitDivision"] = vals[0]->bv_val;
      ldap_value_free_len(vals);
    }
    vals = ldap_get_values_len(ldapServer, entry, "unitBrigade");
    if (vals) {
      root["unitBrigade"] = vals[0]->bv_val;
      ldap_value_free_len(vals);
    }
    vals = ldap_get_values_len(ldapServer, entry, "unitBattalion");
    if (vals) {
      root["unitBattalion"] = vals[0]->bv_val;
      ldap_value_free_len(vals);
    }
    vals = ldap_get_values_len(ldapServer, entry, "unitCompany");
    if (vals) {
      root["unitCompany"] = vals[0]->bv_val;
      ldap_value_free_len(vals);
    }
    vals = ldap_get_values_len(ldapServer, entry, "unitPlatoon");
    if (vals) {
      root["unitPlatoon"] = vals[0]->bv_val;
      ldap_value_free_len(vals);
    }
    vals = ldap_get_values_len(ldapServer, entry, "unitSquad");
    if (vals) {
      root["unitSquad"] = vals[0]->bv_val;
      ldap_value_free_len(vals);
    }


    // Tigr user ID
    vals = ldap_get_values_len(ldapServer, entry, "tigrUid");
    if (vals) {
      root["tigruid"] = vals[0]->bv_val;
      ldap_value_free_len(vals);
    }
    
    // email
    vals = ldap_get_values_len(ldapServer, entry, "mail");
    if (vals) {
      root["email"] = vals[0]->bv_val;	// use only the first mail
      ldap_value_free_len(vals);
    }
    
	
    
    // phone
    vals = ldap_get_values_len(ldapServer, entry, "mobile");
    if (vals) {
      root["phone"] = vals[0]->bv_val;	// use only the first phone
      ldap_value_free_len(vals);
    }


    char *dn = ldap_get_dn(ldapServer, entry);
    char **edn = ldap_explode_dn(dn, 0);
    string unit;
    for(int i=0; edn && edn[i]; i++) {
      int ret = strncmp( edn[i], "ou", 2 );
      if (ret != 0)
	continue;
      char *oval = strchr( edn[i], '=' );
      if (oval)
         oval++;

      if (unit == "")
         unit = string(oval);
      else
         unit = string(oval) + string("/") + unit;
    }
    root["unit"] = unit;
    

 
#ifdef TEST_PHOTO
    std::string ret = root.toStyledString();

    // photo
    vals = ldap_get_values_len(ldapServer, entry, "jpegPhoto");
    if (vals) {

      unsigned long len = vals[0]->bv_len; // get the length of the data  

      //Decode image from base64 using the libb64 lib ....
      base64::decoder d;
      unsigned char* image = new unsigned char [len];//allocate image buffer ...

      // decode the base64 data into plain characters ...

      //decode is now commented since b
      //d.decode (vals[0]->bv_val,vals[0]->bv_len, image);ase64 data cannot 
      // be inserted into LDAP ..
      //using the memcpy for now ... later will add the decode function
      memcpy (image, vals[0]->bv_val, len);
      cout << "Ret string" << ret.data () << endl;

      int buffer_len = 1/*first null terminator*/ 
	             + 6/*The string "photo" and a null terminator*/
	             + len/*the actual data*/ 
                     + 2*4/*the length added two times*/;

      unsigned long nlen = htonl (len); // convert to maintain endianness

      // form the string that stores the photo 
      unsigned char* photo_buf = new unsigned char [buffer_len];

      

      photo_buf[0] = '\0'; // start the photo buffer with a null
      int index = 1;
      memcpy (photo_buf+index, "photo", 5);
      index += 5;
      photo_buf[6] = '\0';
     // memcpy (photo_buf + index, "\0", 1);
      index += 1;
      memcpy (photo_buf + index, (unsigned char*)(&nlen), 4);
      index += 4;
      memcpy (photo_buf + index, image, len);
      index += len;
      memcpy (photo_buf + index, (unsigned char*)(&nlen), 4);
      index += 4;
      //cout << "After photo buf setting " << endl;


      //get the length of the root_str, needed for memcpy in the end
      int root_len = ret.length ();
      cout << "Before Allocation " << endl;

      char* root_str = new char[root_len + index];
      memset (root_str, 0 , root_len + index);

      //copy the string underlying root
      memcpy(root_str, ret.data (), root_len);
    


      // concat the photo data
      // use the index as the length of the 
      //buffer since it tells the number of characters 
      //in the photo_buf
      memcpy (root_str + root_len, photo_buf, index);
      //cout << "After final root_string " << endl;

      ldap_value_free_len(vals); 


      // added this file to check the binary nature of the 
      // root string sent .. will delete it later 
      ofstream out ("root", ifstream::binary);

      out.write (root_str, root_len + index);

      out.close ();
      delete [] image; 
      delete [] photo_buf;
      //need also to delete the root_str .. 
      ret = root_str;
    }
    cout << "JSON: " << root.toStyledString() << endl;
    return ret; 
#else
    return root.toStyledString();
#endif 

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
