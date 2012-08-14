#ifndef GATEWAY_CONFIGURATION_MANAGER_H
#define GATEWAY_CONFIGURATION_MANAGER_H

#include "ConfigurationManager.h"

namespace ammo {
  namespace gateway {
    namespace internal {
      class GatewayConfigurationManager : public ConfigurationManager {
      public:
        static GatewayConfigurationManager* getInstance();
        static GatewayConfigurationManager* getInstance(std::string configfile);
        
        std::string getGatewayAddress();
        std::string getGatewayInterface();
        int getGatewayPort();

      protected:
        void init();
		void decode(const Json::Value& root);

      private:
        GatewayConfigurationManager(const char *configfile);
        
        static GatewayConfigurationManager *sharedInstance;
        
        std::string gatewayAddress;
        std::string gatewayInterface;
        int gatewayPort;
      };
    }
  }
}

#endif //GATEWAY_CONFIGURATION_MANAGER_H
