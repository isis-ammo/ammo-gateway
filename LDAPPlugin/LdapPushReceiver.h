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
