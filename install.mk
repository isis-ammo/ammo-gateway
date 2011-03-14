#
# make install target for Makefile - append me to Makefile
#

###############################################################################
# Configuration
###############################################################################

# Installation directory for Gateway
INSTALL_DIR_BASE=/opt/ammo-gateway

###############################################################################
# Installation rules
###############################################################################

INSTALL_DIR=$(DESTDIR)/$(INSTALL_DIR_BASE)

.PHONY: install
install: all
	mkdir -p $(INSTALL_DIR)
	mkdir -p $(INSTALL_DIR)/bin
	mkdir -p $(INSTALL_DIR)/lib
	install -o root -m 755 build/bin/AndroidGatewayPlugin $(INSTALL_DIR)/bin
	install -o root -m 755 build/bin/AtsGatewayPlugin $(INSTALL_DIR)/bin
	install -o root -m 755 build/bin/GatewayCore $(INSTALL_DIR)/bin
	install -o root -m 755 build/bin/GatewayUsbTransfer $(INSTALL_DIR)/bin
	install -o root -m 755 build/bin/LdapGatewayPlugin $(INSTALL_DIR)/bin
	install -o root -m 755 build/bin/LocationStoreGatewayPlugin $(INSTALL_DIR)/bin
	install -o root -m 755 build/bin/PassGatewayPlugin $(INSTALL_DIR)/bin
	install -o root -m 755 build/bin/SamplePushReceiverGatewayPlugin $(INSTALL_DIR)/bin
	install -o root -m 755 build/bin/SamplePushTestDriverPlugin $(INSTALL_DIR)/bin
	install -o root -m 755 build/bin/SpotPushReceiverGatewayPlugin $(INSTALL_DIR)/bin
	install -o root -m 755 build/bin/TigrGatewayPlugin $(INSTALL_DIR)/bin
	install -o root -m 644 build/bin/LdapPluginConfig.json $(INSTALL_DIR)/bin
	install -o root -m 644 build/bin/AtsPluginConfig.json $(INSTALL_DIR)/bin
	install -o root -m 644 build/bin/LocationStorePluginConfig.json $(INSTALL_DIR)/bin
	install -o root -m 644 build/bin/PassPluginConfig.json $(INSTALL_DIR)/bin
	install -o root -m 644 build/bin/TigrPluginConfig.json $(INSTALL_DIR)/bin
	install -o root -m 644 build/bin/GatewayConfig.json $(INSTALL_DIR)/bin
	install -o root -m 644 build/lib/libgatewayconnector.so $(INSTALL_DIR)/lib
	install -o root -m 644 build/lib/libgeotrans-mgrs.so $(INSTALL_DIR)/lib
	install -o root -m 644 build/lib/libjson.so $(INSTALL_DIR)/lib
	install -o root -m 755 dist/template/launch_gateway_headless.sh $(INSTALL_DIR)
	install -o root -m 755 dist/template/launch.sh $(INSTALL_DIR)
	install -o root -m 755 dist/template/kill_all_gateway.sh $(INSTALL_DIR)

