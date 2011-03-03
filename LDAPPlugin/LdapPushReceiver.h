#ifndef LDAP_PUSH_RECEIVER_H
#define LDAP_PUSH_RECEIVER_H

#include "GatewayConnector.h"
#include <ctime>
#include <vector>

#include <ldap.h>

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


class LdapPushReceiver : public DataPushReceiverListener, 
			 public GatewayConnectorDelegate, 
			 public PullRequestReceiverListener
{
public:
  LdapPushReceiver();
  //GatewayConnectorDelegate methods
  virtual void onConnect(GatewayConnector *sender);
  virtual void onDisconnect(GatewayConnector *sender);

  //DataPushReceiverListener methods
  virtual void onDataReceived(GatewayConnector *sender, std::string uri, 
			      std::string mimeType, std::vector<char> &data, 
			      std::string originUser);

  bool get(std::string query, std::vector<std::string> &jsonResults);

  std::string jsonForObject(LDAPMessage *entry);

  bool editContact(const LdapContact& );

  //PullRequestReceiverListener methods
  virtual void onDataReceived(GatewayConnector *sender,
                              std::string requestUid, std::string pluginId,
                              std::string mimeType, std::string query,
                              std::string projection, unsigned int maxResults,
                              unsigned int startFromCount, bool liveQuery);

private:
  std::map<int, LdapContact> unsentContacts;
  LDAP *ldapServer;
};

static int write_callback(char *data, size_t size, size_t nmemb, std::string *writerData);

#endif        //  #ifndef LDAP_PUSH_RECEIVER_H

