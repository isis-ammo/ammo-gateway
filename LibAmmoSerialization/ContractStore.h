#ifndef CONTRACT_STORE_H
#define CONTRACT_STORE_H

#include <map>
#include "Contract.h"

namespace ammo {
  namespace gateway {
    class ContractStore {
    public:
      ContractStore();

      ammo::gateway::Relation &getRelationForType(std::string mimeType);

    private:
      std::map<std::string, ammo::gateway::Relation> relations;

      void loadContracts();
    };
  }
}

#endif //CONTRACT_STORE_H
