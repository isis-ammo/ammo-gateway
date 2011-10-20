#ifndef LDAP_PUSH_RECEIVER_H
#define LDAP_PUSH_RECEIVER_H

#include "GatewayConnector.h"
#include <ctime>
#include <vector>

#include <ldap.h>

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
  LDAP *ldapServer;
};

//static int write_callback(char *data, size_t size, size_t nmemb, std::string *writerData);

#endif  // LDAP_PUSH_RECEIVER_H
