#include "GatewayConnector.h"
#include "GatewayConfigurationManager.h"
#include "ace/Connector.h"
#include "protocol/GatewayPrivateMessages.pb.h"
#include <string>
#include <iostream>

using namespace std;

GatewayConnector::GatewayConnector(GatewayConnectorDelegate *delegate) : connected(false), handler(NULL), delegate(delegate) {
  GatewayConfigurationManager *config = GatewayConfigurationManager::getInstance();
  
  ACE_INET_Addr serverAddress(config->getGatewayPort(), config->getGatewayAddress().c_str());
  connector = new ACE_Connector<GatewayServiceHandler, ACE_SOCK_Connector>();
  int status = connector->connect(handler, serverAddress);
  if(status == -1) {
    std::cout << "connection failed" << std::endl << std::flush;
    std::cout << "errno: " << errno << std::endl << std::flush;
    std::cout << "error: " << strerror(errno) << std::endl << std::flush;
    connected = false;
  } else {
    connected = true;
    handler->setParentConnector(this);
  }
  if(handler == NULL) {
    std::cout << "Handler not created by ACE_Connector" << std::endl << std::flush;
  } else {
    std::cout << "Gateway service handler created by ACE_Connector" << std::endl << std::flush;
  }
}

GatewayConnector::~GatewayConnector() {
  std::cout << "Deleting GatewayConnector()" << std::endl << std::flush;
  if(connected) {
    handler->close();
    connector->close();
  }
  delete connector;
}
  
bool GatewayConnector::associateDevice(string device, string user, string key) {
  ammo::gateway::protocol::GatewayWrapper msg;
  ammo::gateway::protocol::AssociateDevice *associateMsg = msg.mutable_associate_device();
  associateMsg->set_device(device);
  associateMsg->set_user(user);
  associateMsg->set_key(key);
  
  msg.set_type(ammo::gateway::protocol::GatewayWrapper_MessageType_ASSOCIATE_DEVICE);
  
  std::cout << "Sending Associate Device message to gateway core" << std::endl << std::flush;
  if(connected) {
    handler->sendData(msg);
    return true;
  } else {
    std::cout << "Not connected to gateway; can't send data" << std::endl << std::flush;
    return false;
  }
}

bool GatewayConnector::pushData(string uri, string mimeType, const string &data) {
  ammo::gateway::protocol::GatewayWrapper msg;
  ammo::gateway::protocol::PushData *pushMsg = msg.mutable_push_data();
  pushMsg->set_uri(uri);
  pushMsg->set_mime_type(mimeType);
  pushMsg->set_data(data);
  
  msg.set_type(ammo::gateway::protocol::GatewayWrapper_MessageType_PUSH_DATA);
  
  std::cout << "Sending Data Push message to gateway core" << std::endl << std::flush;
  if(connected) {
    handler->sendData(msg);
    return true;
  } else {
    std::cout << "Not connected to gateway; can't send data" << std::endl << std::flush;
    return false;
  }
}

bool GatewayConnector::pullRequest(std::string requestUid, std::string pluginId,
				   std::string mimeType, std::string query,
				   std::string projection, unsigned int maxResults,
				   unsigned int startFromCount, bool liveQuery) {

  ammo::gateway::protocol::GatewayWrapper msg;
  ammo::gateway::protocol::PullRequest *pullMsg = msg.mutable_pull_request();
  pullMsg->set_request_uid(requestUid);
  pullMsg->set_plugin_id(pluginId);
  pullMsg->set_mime_type(mimeType);
  pullMsg->set_query(query);
  pullMsg->set_projection(projection);
  pullMsg->set_max_results(maxResults);
  pullMsg->set_start_from_count(startFromCount);
  pullMsg->set_live_query(liveQuery);
  
  msg.set_type(ammo::gateway::protocol::GatewayWrapper_MessageType_PULL_REQUEST);
  
  std::cout << "Sending Pull Request message to gateway core" << std::endl << std::flush;
  if(connected) {
    handler->sendData(msg);
    return true;
  } else {
    std::cout << "Not connected to gateway; can't send data" << std::endl << std::flush;
    return false;
  }
}

bool GatewayConnector::pullResponse(std::string requestUid, std::string pluginId,
				   std::string mimeType, std::string uri,
				    std::vector<char>& data) {

  ammo::gateway::protocol::GatewayWrapper msg;
  ammo::gateway::protocol::PullResponse *pullMsg = msg.mutable_pull_response();
  pullMsg->set_request_uid(requestUid);
  pullMsg->set_plugin_id(pluginId);
  pullMsg->set_mime_type(mimeType);
  pullMsg->set_uri(uri);
  pullMsg->set_data( std::string(data.begin(), data.end()) );
  
  msg.set_type(ammo::gateway::protocol::GatewayWrapper_MessageType_PULL_RESPONSE);
  
  std::cout << "Sending Pull Response message to gateway core" << std::endl << std::flush;
  if(connected) {
    handler->sendData(msg);
    return true;
  } else {
    std::cout << "Not connected to gateway; can't send data" << std::endl << std::flush;
    return false;
  }
}




bool GatewayConnector::registerDataInterest(string mime_type, DataPushReceiverListener *listener) {
  ammo::gateway::protocol::GatewayWrapper msg;
  ammo::gateway::protocol::RegisterDataInterest *di = msg.mutable_register_data_interest();
  di->set_mime_type(mime_type);
  
  msg.set_type(ammo::gateway::protocol::GatewayWrapper_MessageType_REGISTER_DATA_INTEREST);
  
  std::cout << "Sending RegisterDataInterest message to gateway core" << std::endl << std::flush;
  if(connected) {
    handler->sendData(msg);
    receiverListeners[mime_type] = listener;
    return true;
  } else {
    std::cout << "Not connected to gateway; can't send data" << std::endl << std::flush;
    std::cout << "Receiver listener was not registered; it won't receive any data." << std::endl << std::flush;
    return false;
  }
}

bool GatewayConnector::unregisterDataInterest(string mime_type) {
  ammo::gateway::protocol::GatewayWrapper msg;
  ammo::gateway::protocol::UnregisterDataInterest *di = msg.mutable_unregister_data_interest();
  di->set_mime_type(mime_type);
  
  msg.set_type(ammo::gateway::protocol::GatewayWrapper_MessageType_UNREGISTER_DATA_INTEREST);
  
  std::cout << "Sending UnregisterDataInterest message to gateway core" << std::endl << std::flush;
  if(connected) {
    handler->sendData(msg);
    receiverListeners.erase(mime_type);
    return true;
  } else {
    std::cout << "Not connected to gateway; can't send data" << std::endl << std::flush;
    return false;
  }
}


bool GatewayConnector::registerPullInterest(string mime_type, PullRequestReceiverListener *listener) {
  ammo::gateway::protocol::GatewayWrapper msg;
  ammo::gateway::protocol::RegisterPullInterest *di = msg.mutable_register_pull_interest();
  di->set_mime_type(mime_type);
  
  msg.set_type(ammo::gateway::protocol::GatewayWrapper_MessageType_REGISTER_PULL_INTEREST);
  
  std::cout << "Sending RegisterPullInterest message to gateway core" << std::endl << std::flush;
  if(connected) {
    handler->sendData(msg);
    pullRequestListeners[mime_type] = listener;
    return true;
  } else {
    std::cout << "Not connected to gateway; can't send data" << std::endl << std::flush;
    std::cout << "Pull Request listener was not registered; it won't receive any data." << std::endl << std::flush;
    return false;
  }
}

bool GatewayConnector::unregisterPullInterest(string mime_type) {
  ammo::gateway::protocol::GatewayWrapper msg;
  ammo::gateway::protocol::UnregisterPullInterest *di = msg.mutable_unregister_pull_interest();
  di->set_mime_type(mime_type);
  
  msg.set_type(ammo::gateway::protocol::GatewayWrapper_MessageType_UNREGISTER_PULL_INTEREST);
  
  std::cout << "Sending UnregisterPullInterest message to gateway core" << std::endl << std::flush;
  if(connected) {
    handler->sendData(msg);
    pullRequestListeners.erase(mime_type);
    return true;
  } else {
    std::cout << "Not connected to gateway; can't send pull" << std::endl << std::flush;
    return false;
  }
}

bool GatewayConnector::registerPullResponseInterest(string mime_type, PullResponseReceiverListener *listener) {
  pullResponseListeners[mime_type] = listener;
  return true;
}

bool GatewayConnector::unregisterPullResponseInterest(string mime_type) {
  pullResponseListeners.erase(mime_type);
  return true;
}


void GatewayConnector::onAssociateResultReceived(const ammo::gateway::protocol::AssociateResult &msg) {
  std::cout << "Got associate result of " << msg.result() << std::endl << std::flush;
  if(delegate != NULL) {
    delegate->onAuthenticationResponse(this, msg.result());
  }
}

void GatewayConnector::onPushDataReceived(const ammo::gateway::protocol::PushData &msg) {
  string uri = msg.uri();
  string mimeType = msg.mime_type();
  vector<char> data(msg.data().begin(), msg.data().end());
  
  receiverListeners[mimeType]->onDataReceived(this, uri, mimeType, data);
}

void GatewayConnector::onPullRequestReceived(const ammo::gateway::protocol::PullRequest &msg) {
  string mimeType = msg.mime_type();
  map<std::string, PullRequestReceiverListener *>::iterator it = pullRequestListeners.find(mimeType);
  if ( it != pullRequestListeners.end() ) {
    (*it).second->onDataReceived(this, msg.request_uid(), msg.plugin_id(), msg.mime_type(), msg.query(),
				 msg.projection(), msg.max_results(), msg.start_from_count(), msg.live_query() );
  } 
}


void GatewayConnector::onPullResponseReceived(const ammo::gateway::protocol::PullResponse &msg) {
  string mimeType = msg.mime_type();
  map<std::string, PullResponseReceiverListener *>::iterator it = pullResponseListeners.find(mimeType);
  if ( it != pullResponseListeners.end() ) {
    vector<char> data(msg.data().begin(), msg.data().end());
    (*it).second->onDataReceived(this, msg.request_uid(), msg.plugin_id(), msg.mime_type(), msg.uri(), data );
  }
}




//--GatewayConnectorDelegate default implementations (for optional delegate methods)

void GatewayConnectorDelegate::onAuthenticationResponse(GatewayConnector *sender, bool result) {
  std::cout << "GatewayConnectorDelegate::onAuthenticationResponse : result = " << result << std::endl << std::flush;
}


