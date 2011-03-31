#
# make install target for Makefile - append me to Makefile
#

###############################################################################
# Configuration
###############################################################################

# Installation directory for Gateway
INSTALL_DIR_BASE=/usr

###############################################################################
# Installation rules
###############################################################################

INSTALL_DIR=$(DESTDIR)/$(INSTALL_DIR_BASE)

.PHONY: install
install: all
	echo "Creating directories ..."
	mkdir -p $(INSTALL_DIR)
	mkdir -p $(INSTALL_DIR)/bin
	mkdir -p $(INSTALL_DIR)/lib
	mkdir -p $(DESTDIR)/etc/ammo-gateway
	echo "Installing binaries ..."
	install -m 755 build/bin/AndroidGatewayPlugin $(INSTALL_DIR)/bin
	install -m 755 build/bin/AtsGatewayPlugin $(INSTALL_DIR)/bin
	install -m 755 build/bin/GatewayCore $(INSTALL_DIR)/bin
	install -m 755 build/bin/GatewayUsbTransfer $(INSTALL_DIR)/bin
	install -m 755 build/bin/LdapGatewayPlugin $(INSTALL_DIR)/bin
	install -m 755 build/bin/LocationStoreGatewayPlugin $(INSTALL_DIR)/bin
	install -m 755 build/bin/PassGatewayPlugin $(INSTALL_DIR)/bin
	install -m 755 build/bin/SamplePushReceiverGatewayPlugin $(INSTALL_DIR)/bin
	install -m 755 build/bin/SamplePushTestDriverPlugin $(INSTALL_DIR)/bin
	install -m 755 build/bin/SpotPushReceiverGatewayPlugin $(INSTALL_DIR)/bin
	install -m 755 build/bin/TigrGatewayPlugin $(INSTALL_DIR)/bin
	echo "Installing libs ..."
	install -m 644 build/lib/libgatewayconnector.so $(INSTALL_DIR)/lib
	install -m 644 build/lib/libgeotrans-mgrs.so $(INSTALL_DIR)/lib
	install -m 644 build/lib/libjson.so $(INSTALL_DIR)/lib
	echo "Installing scripts ..."
	install -m 755 dist/template/bin/launch_ammo_gateway_headless.sh $(INSTALL_DIR)/bin
	install -m 755 dist/template/bin/launch_ammo_gateway.sh $(INSTALL_DIR)/bin
	install -m 755 dist/template/bin/kill_all_gateway.sh $(INSTALL_DIR)/bin
	echo "Installing config files ..."
	install -m 644 build/etc/AtsPluginConfig.json $(DESTDIR)/etc/ammo-gateway
	install -m 644 build/etc/GatewayConfig.json $(DESTDIR)/etc/ammo-gateway
	install -m 644 build/etc/LdapPluginConfig.json $(DESTDIR)/etc/ammo-gateway
	install -m 644 build/etc/LocationStorePluginConfig.json $(DESTDIR)/etc/ammo-gateway
	install -m 644 build/etc/PassPluginConfig.json $(DESTDIR)/etc/ammo-gateway
	install -m 644 build/etc/TigrPluginConfig.json $(DESTDIR)/etc/ammo-gateway

