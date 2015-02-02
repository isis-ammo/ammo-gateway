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

#include "LdapPushReceiver.h"
#include "LdapConfigurationManager.h"
#include "GatewayLdapConstants.h"

#include "json/json.h"

#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>

#include "decode.h"
#include <fstream>


using namespace std;
using namespace ammo::gateway;

/*
 * LDAPPlugin
 * Dataflow Scenarios:
 * 1. User on the device edits his own contact information --> push to LDAP Server (ldapadd/modify ...)
 * 2. User on the device queries for a specific contact --> pull from LDAP server (ldapsearch)
 */


//============================================================
// Push Reciever Constructor
//  a) Read Configuration
//  b) Connect to the LDAP Server
//  TBD: where / when do we authenticate with LDAP Server with admin user credentials
//============================================================
LdapPushReceiver::LdapPushReceiver() : ldap(0)
{
  LdapConfigurationManager *config = LdapConfigurationManager::getInstance();

  string ldapAddress = config->getLdapServerAddress();
  int ldapPort = config->getLdapServerPort();
  LOG_INFO("Attempting Connection to LDAP Server @:" << ldapAddress << ':' << ldapPort);

  // Create LDAP instance
  this->ldap = LdapClient::createInstance();
  if (!this->ldap) {
      LOG_ERROR("Error Creating LDAP Client");
      return;
  }
  LOG_TRACE("Succeeded Creating LDAP Client");

  // Initialize LDAP instance with connection data.
  if (!this->ldap->init(ldapAddress, ldapPort)) {
      LOG_ERROR("Error Initializing LDAP Library: " << this->ldap->getLastErrorMsg());
      return;
  }
  LOG_TRACE("Succeeded Initializing LDAP Library");

  // Set LDAP to version 3.
  int version = LDAP_VERSION3;
  this->ldap->setOption(LDAP_OPT_PROTOCOL_VERSION, &version);

  string basedn = config->getLdapUsername();
  string passwd = config->getLdapPassword();

  // Bind LDAP to a user.
  LOG_INFO("Attempting bind operation w/user : " << basedn << " ... ");
  if (!this->ldap->bind(basedn, passwd)) {
      LOG_ERROR("Error Binding to LDAP Server: " << this->ldap->getLastErrorMsg());
  }
  LOG_INFO("Connected to LDAP Server @");
}

//============================================================
//
// onConnect()
//
//============================================================
void LdapPushReceiver::onConnect(GatewayConnector *sender)
{
}

//============================================================
//
// onDisconnect()
//
//============================================================
void LdapPushReceiver::onDisconnect(GatewayConnector *sender)
{
}



//============================================================
//
// onDataReceived()
//
// Data Push from the Device
// User has edited his own contact information - do an ldapadd/modify on the LDAPServer
//============================================================
void LdapPushReceiver::onPushDataReceived(GatewayConnector *sender, ammo::gateway::PushData &pushData)
{
  LOG_DEBUG("Got data.");
  LOG_TRACE("  " << pushData);

  if(pushData.mimeType == CONTACT_MIME_TYPE)
    {
      // Extract JSON string from message payload
      std::string json = payloadToJson(pushData.data);

      // Populate contact object from data in JSON
      LdapContact* contact = NULL;
      contact = objectFromJson(json);
      if (contact)
        {
          // Edit the LDAP entry for this contact
          if (!editContact(*contact))
            {
              LOG_ERROR("ERROR while updated LDAP for: " << contact->name);
            }
        }
    }
  else
    {
      LOG_ERROR("ERROR!  Invalid Mime Type.");
    }

}


//============================================================
//
// onDataReceived()
//
// Pull Request
// Query containing LDAP search parameters
//============================================================
void LdapPushReceiver::onPullRequestReceived(GatewayConnector *sender, ammo::gateway::PullRequest &pullReq)
{
  string response = "asdf";
  vector<string> jsonResults;
  get(pullReq.query, jsonResults);
  for(vector<string>::iterator it = jsonResults.begin(); it != jsonResults.end(); it++)
    {
      string data = *it;
      PullResponse resp = PullResponse::createFromPullRequest(pullReq);

      // TODO: the response URI should be some unique identifier for this record
      resp.uri = "ammo-demo:test-object";

      resp.data = data;
      sender->pullResponse(resp);
    }
}

//============================================================
//
// get()
//
//============================================================
bool LdapPushReceiver::get(std::string query, std::vector<std::string> &jsonResults)
{

  //LdapConfigurationManager *config = LdapConfigurationManager::getInstance();

  LDAPMessage *results;
  //std::string filter = "(& (objectClass=x-MilitaryPerson) (objectClass=inetOrgPerson)";
  std::string filter = "(& (objectClass=MilitaryPerson) ";


  // build the filter based on query expression
  // query = comma-separated field-name / value pairs

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

  filter += " )";

  //changed the timeout to 5 sec from 1 ... since jpeg files are taking long
  LdapClient::TimeVal timeout = { 5, 0 };

  LDAPControl *serverctrls = NULL, *clientctrls = NULL;
  char *attrs[] = { const_cast<char *>("*"), NULL };
  const std::string basedn = "dc=ammo,dc=tdm";

  LOG_DEBUG("LDAP Starting Search for: " << filter);
  if (!this->ldap->search(basedn, /* LDAP search base dn (distinguished name) */
                          LDAP_SCOPE_SUBTREE, /* scope - root and all descendants */
                          filter, /* filter - query expression */
                          attrs, /* requested attributes (white-space seperated list, * = ALL) */
                          0, /* attrsonly - if we only want to get attribut types */
                          &serverctrls,
                          &clientctrls,
                          timeout,
                          0, // number of results : 0 = unlimited
                          &results)) {
      LOG_ERROR("LDAP search failed for " << filter << ": "
           << hex << this->ldap->getLastError() << " - " << this->ldap->getLastErrorMsg());
      return false;
  }
  LOG_DEBUG("LDAP Search Returned " << ldap->countEntries(results) << " results");

  // Pack the search results into JSON objects and store in a vector
  LDAPMessage *entry = ldap->firstEntry(results);
  while(entry)
    {
      jsonResults.push_back( jsonForObject(entry) );
      entry = ldap->nextEntry(entry);
    }

  // Free the LDAP message object
  if (results)
    {
      ldap->msgFree(entry);
    }

  return true;
}

//============================================================
//
// payloadToJson()
//
//============================================================
std::string LdapPushReceiver::payloadToJson(std::string &data)
{
  //cout << "Extracting JSON metadata..." << endl << flush;
  unsigned int jsonEnd = 0;
  for(string::iterator it = data.begin(); it != data.end(); it++)
    {
      jsonEnd++;
      if((*it) == 0) break;
    }

  std::string jsonOut(&data[0], jsonEnd);
  LOG_TRACE("JSON string: " << jsonOut);

  return jsonOut;
}


//============================================================
//
// objectFromJson()
//
//============================================================
bool LdapPushReceiver::parseJson(std::string input, Json::Value& jsonRoot)
{
  Json::Reader jsonReader;
  bool parseSuccess = jsonReader.parse(input, jsonRoot);

  if(!parseSuccess)
    {
      LOG_ERROR("JSON parsing error:");
      LOG_ERROR(jsonReader.getFormatedErrorMessages());
      return parseSuccess;
    }

  LOG_TRACE("Parsed JSON: " << jsonRoot.toStyledString());
  return parseSuccess;
}

//============================================================
//
// objectFromJson()
//
//============================================================
LdapContact* LdapPushReceiver::objectFromJson(std::string input)
{
  Json::Value jsonRoot;
  if (!parseJson(input, jsonRoot)) return NULL;
  
  /*
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
  */

  LdapContact* contact = new LdapContact;
  contact->name = jsonRoot["name"].asString();
  contact->middle_initial = jsonRoot["middle_initial"].asString();
  contact->lastname = jsonRoot["lastname"].asString();
  contact->rank = jsonRoot["rank"].asString();
  contact->callsign = jsonRoot["callsign"].asString();
  contact->branch = jsonRoot["branch"].asString();
  contact->unit = jsonRoot["unit"].asString();
  contact->email = jsonRoot["email"].asString();
  contact->phone = jsonRoot["phone"].asString();

  return contact;
}

//============================================================
//
// jsonForObject()
//
//============================================================
string LdapPushReceiver::jsonForObject(LDAPMessage *entry) {
  Json::Value root;
  struct berval **vals;

  // name
  vals = ldap->getValuesLen(entry, "givenName");
  if (vals)
    {
      root["name"] = vals[0]->bv_val; // there must be only one name
      ldap->valueFreeLen(vals);
    }

  vals = ldap->getValuesLen(entry, "sn");
  if (vals)
    {
      root["lastname"] = vals[0]->bv_val; // there must be only one name
      ldap->valueFreeLen(vals);
    }

  vals = ldap->getValuesLen(entry, "initials");
  if (vals) {
    root["middle_initial"] = vals[0]->bv_val; // there must be only one name
    ldap->valueFreeLen(vals);
  }

  // rank
  vals = ldap->getValuesLen(entry, "Rank");
  if (vals)
    {
      root["rank"] = vals[0]->bv_val;
      ldap->valueFreeLen(vals);
    }

  // callsign
  vals = ldap->getValuesLen(entry, "Callsign");
  if (vals)
    {
      root["callsign"] = vals[0]->bv_val;
      ldap->valueFreeLen(vals);
    }

  // branch
  vals = ldap->getValuesLen(entry, "Branch");
  if (vals)
    {
      root["branch"] = vals[0]->bv_val;
      ldap->valueFreeLen(vals);
    }

  // unit (generic)
  vals = ldap->getValuesLen(entry, "Unit");
  if (vals)
    {
      root["unit"] = vals[0]->bv_val;
      ldap->valueFreeLen(vals);
    }

  // unit (specific, separated)
  vals = ldap->getValuesLen(entry, "unitDivision");
  if (vals)
    {
      root["unitDivision"] = vals[0]->bv_val;
      ldap->valueFreeLen(vals);
    }
  vals = ldap->getValuesLen(entry, "unitBrigade");
  if (vals)
    {
      root["unitBrigade"] = vals[0]->bv_val;
      ldap->valueFreeLen(vals);
    }
  vals = ldap->getValuesLen(entry, "unitBattalion");
  if (vals)
    {
      root["unitBattalion"] = vals[0]->bv_val;
      ldap->valueFreeLen(vals);
    }
  vals = ldap->getValuesLen(entry, "unitCompany");
  if (vals)
    {
      root["unitCompany"] = vals[0]->bv_val;
      ldap->valueFreeLen(vals);
    }
  vals = ldap->getValuesLen(entry, "unitPlatoon");
  if (vals)
    {
      root["unitPlatoon"] = vals[0]->bv_val;
      ldap->valueFreeLen(vals);
    }
  vals = ldap->getValuesLen(entry, "unitSquad");
  if (vals)
    {
      root["unitSquad"] = vals[0]->bv_val;
      ldap->valueFreeLen(vals);
    }


  // Tigr user ID
  vals = ldap->getValuesLen(entry, "tigrUid");
  if (vals) {
    root["tigruid"] = vals[0]->bv_val;
    ldap->valueFreeLen(vals);
  }
  
  // Numerical user ID
  vals = ldap->getValuesLen(entry, "userIdNumber");
  if (vals) {
    root["userIdNum"] = vals[0]->bv_val;
    ldap->valueFreeLen(vals);
  }

  // email
  vals = ldap->getValuesLen(entry, "mail");
  if (vals)
    {
      root["email"] = vals[0]->bv_val;    // use only the first mail
      ldap->valueFreeLen(vals);
    }

  // phone
  vals = ldap->getValuesLen(entry, "mobile");
  if (vals)
    {
      root["phone"] = vals[0]->bv_val;    // use only the first phone
      ldap->valueFreeLen(vals);
    }


  // This block is outdated and overwriting the unit info -- comment out
  /*
  char *dn = ldap_get_dn(ldapServer, entry);
  char **edn = ldap_explode_dn(dn, 0);
  string unit;
  for(int i=0; edn && edn[i]; i++)
    {
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
  */


#ifdef TEST_PHOTO
  std::string ret = root.toStyledString();

  // photo
  vals = ldap->getValuesLen(entry, "jpegPhoto");
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
    LOG_TRACE("Ret string" << ret.data ());

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
    LOG_TRACE("Before Allocation ");

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

    ldap->valueFreeLen(vals);


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
  LOG_TRACE("JSON: " << root.toStyledString());
  return ret;
#else
  return root.toStyledString();
#endif

}

//============================================================
//
// editContact()
//
//============================================================
bool LdapPushReceiver::editContact(const LdapContact& contact)
{
  LOG_DEBUG("LdapPushReceiver::editContact()");
  LOG_TRACE("  " << contact.name);
  return true;
}

//============================================================
//
// write_callback()
//
//============================================================
/*static int write_callback(char *data, size_t size, size_t nmemb, std::string *writerData)
{
  if(writerData == NULL)
    {
      return 0;
    }
  writerData->append(data, size*nmemb);
  return size * nmemb;
}*/
