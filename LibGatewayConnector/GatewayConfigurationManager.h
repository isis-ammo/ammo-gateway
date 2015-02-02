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
