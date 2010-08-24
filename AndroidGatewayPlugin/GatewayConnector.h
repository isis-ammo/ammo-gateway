class GatewayConnectorDelegate;
class DataPushReceiverDelegate;

class GatewayConnector {
public:
  GatewayConnector(GatewayConnectorDelegate *delegate);
  
  //General connection negotiation and bookkeeping
  bool associateDevice(string device, string user, string key);
  
  //--Data-Push support methods--
  //Sender-side
  bool pushData(string uri, string mimeType, vector<char> &data);
  //Receiver-side
  bool registerDataInterest(string uri, DataPushReceiverDelegate *delegate);
  bool unregisterDataInterest(string uri);
};

class GatewayConnectorDelegate {
  virtual void onConnect(GatewayConnector &sender) = 0;
  virtual void onDisconnect(GatewayConnector &sender) = 0;
};

class DataPushReceiverDelegate {
public:
  virtual void onDataReceived(GatewayConnector &sender, string uri, vector<char> &data) = 0;
};
