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

#ifndef LDAP_PUSH_RECEIVER_H
#define LDAP_PUSH_RECEIVER_H

#include "GatewayConnector.h"
#include <ctime>
#include <vector>

#include "LdapClient.h"
#include "json/json.h"

class LdapContact 
{
public:
  std::string name;
  std::string middle_initial;
  std::string lastname;
  std::string rank;
  std::string callsign;
  std::string branch;
  std::string unit;
  std::string email;
  std::string phone;
  std::vector<unsigned char> photo;
  std::vector<unsigned char> insignia;
};


class LdapPushReceiver : public ammo::gateway::DataPushReceiverListener, 
			 public ammo::gateway::GatewayConnectorDelegate, 
			 public ammo::gateway::PullRequestReceiverListener
{
public:
  LdapPushReceiver();
  
  // virtual method from GatewayConnectorDelegate
  virtual void onConnect(ammo::gateway::GatewayConnector *sender);
  virtual void onDisconnect(ammo::gateway::GatewayConnector *sender);

  // virtual method from DataPushReceiverListener 
  virtual void onPushDataReceived(ammo::gateway::GatewayConnector *sender, ammo::gateway::PushData &pushData);

  // virtual method from PullRequestReceiverListener
  virtual void onPullRequestReceived(ammo::gateway::GatewayConnector *sender, ammo::gateway::PullRequest &pullReq);


private:
  bool get(std::string query, std::vector<std::string> &jsonResults);
  bool editContact(const LdapContact& );
  std::string jsonForObject(LDAPMessage *entry);
  LdapContact* objectFromJson(std::string input);
  std::string payloadToJson(std::string &data);
  bool parseJson(std::string input, Json::Value& jsonRoot);

private:
  std::map<int, LdapContact> unsentContacts;
  LdapClient* ldap;
};

//static int write_callback(char *data, size_t size, size_t nmemb, std::string *writerData);

#endif  // LDAP_PUSH_RECEIVER_H
