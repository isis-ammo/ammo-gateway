#ifndef DATA_STORE_DISPATCHER_H
#define DATA_STORE_DISPATCHER_H

#include "GatewaySyncSerialization.h"

struct sqlite3;
struct sqlite3_stmt;
class QueryHandler;
class DataStoreConfigManager;

namespace ammo
{
  namespace gateway
  {
    class PullRequest;
    class GatewayConnector;
    class PointToPointMessage;
  }
}

class DataStoreDispatcher
{
public:
  DataStoreDispatcher (void);
  
  void
  dispatchPushData (
    sqlite3 *db,
    ammo::gateway::PushData &pd);
                       
  void
  dispatchPullRequest (
    sqlite3 *db,
    ammo::gateway::GatewayConnector *sender,
    ammo::gateway::PullRequest &pr);
                            
  void
  dispatchPointToPointMessage (
    sqlite3 *db,
    ammo::gateway::GatewayConnector *sender,
    const ammo::gateway::PointToPointMessage &msg);
                            
  void set_cfg_mgr (DataStoreConfigManager *cfg_mgr);
  
private:
  // Match entries stored later than tv's value, and store
  // their checksums in the class member.
  bool
  collect_recent_checksums (sqlite3 *db,
                            const ACE_Time_Value &tv);
                            
  // Match entries whose checksums are equal to any in the list arg.
  bool
  match_requested_checksums (sqlite3 *db,
                             sendObjectsMessageData &holder,
                             const std::vector<std::string> &checksums);
                             
  bool
  prepare_match_statement (sqlite3 *db,
                           sqlite3_stmt *&stmt,
                           const char *tbl_name,
                           const std::vector<std::string> &checksums);
                           
  void
  extract_original_row (sqlite3_stmt *stmt,
                        sendObjectsMessageData &holder);
  
  void
  extract_contacts_row (sqlite3_stmt *stmt,
                        sendObjectsMessageData &holder);
  
  // Identify which checksums from a list are not in the local db.
  bool
  collect_missing_checksums (sqlite3 *db,
                             const std::vector<std::string> &checksums);
                             
  // Generate a uid for point-to-point messages.
  std::string gen_uuid (void);
                     
private:
  // Store an instance of the config manager for convenience.
  DataStoreConfigManager *cfg_mgr_;

  // Persistent container for collected checksums.
  std::vector<std::string> checksums_;
  
  // Holder for generated UUIDs.
  const ACE_CString *new_uuid_;
};

#endif /* DATA_STORE_DISPATCHER_H */
