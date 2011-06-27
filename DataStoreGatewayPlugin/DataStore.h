#ifndef DATA_STORE_H
#define DATA_STORE_H

#include "GatewayConnector.h"

class sqlite3;
class sqlite3_stmt;

namespace Json
{
  class Value;
}

class DataStoreReceiver : public ammo::gateway::DataPushReceiverListener,
					                public ammo::gateway::GatewayConnectorDelegate,
                          public ammo::gateway::PullRequestReceiverListener

{
public:
  DataStoreReceiver (void);
  ~DataStoreReceiver (void);
	
  // GatewayConnectorDelegate methods
  virtual void onConnect (ammo::gateway::GatewayConnector *sender);
  virtual void onDisconnect (ammo::gateway::GatewayConnector *sender);
  
  // DataPushReceiverListener methods
  virtual void onPushDataReceived (ammo::gateway::GatewayConnector *sender,
							                     ammo::gateway::PushData &pushData);
	
  // PullRequestReceiverListener methods
  virtual void onPullRequestReceived (ammo::gateway::GatewayConnector *sender,
                                      ammo::gateway::PullRequest &pullReq);
                               
  void db_filepath (const std::string &path);
  bool init (void);

private:
  struct PrivateContact 
  {
    std::string first_name;
    std::string middle_initial;
    std::string last_name;
    std::string rank;
    std::string call_sign;
    std::string branch;
    std::string unit;
    std::string email;
    std::string phone;
    std::vector<unsigned char> photo;
    std::vector<unsigned char> insignia;
  };

  bool pushContactData (const ammo::gateway::PushData &pushData);
  bool pushGeneralData (const ammo::gateway::PushData &pushData);

  bool parseJson (const std::string &input, Json::Value &root);
  void contactFromJson (const Json::Value &jsonRoot, PrivateContact &info);
  
  bool createTable (const char *tbl_string, const char *msg);
  
  bool bind_text (int column,
                  const std::string &text,
                  const char *push_type,
                  const char *column_name);
                  
  bool bind_int (int column,
                 int val,
                 const char *push_type,
                 const char *column_name);
                 
  bool bind_blob (int column,
                  const void *val,
                  int size,
                  const char *push_type,
                  const char *column_name);
                  
private:
  // Pointer to open database.
  sqlite3 *db_;
  
  // Temporary holder for statements being constructed.
  sqlite3_stmt *stmt_;
	
  // Set by the config manager.
  std::string db_filepath_;
};

#endif        //  #ifndef DATA_STORE_H

