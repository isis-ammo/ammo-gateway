class GatewayConnectorDelegate;
class DataPushReceiverListener;

class GatewayConnector {
public:
  GatewayConnector(GatewayConnectorDelegate *delegate);
  
  //General connection negotiation and bookkeeping
  bool associateDevice(string device, string user, string key);
  
  //--Data-Push support methods--
  //Sender-side
  bool pushData(string uri, string mimeType, vector<char> &data);
  //Receiver-side
  bool registerDataInterest(string uri, DataPushReceiverListener *listener);
  bool unregisterDataInterest(string uri);
pri
};

class GatewayConnectorDelegate {
  virtual void onConnect(GatewayConnector &sender) = 0;
  virtual void onDisconnect(GatewayConnector &sender) = 0;
};

class DataPushReceiverListener {
public:
  virtual void onDataReceived(GatewayConnector &sender, string uri, vector<char> &data) = 0;
};
