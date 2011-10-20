#
# make install target for Makefile - append me to Makefile
#

###############################################################################
# Configuration
###############################################################################

# Installation directory for Gateway
INSTALL_DIR_BASE=/usr
VERSION=`cat .build-version | sed 's/release-//' | cut -d- -f 1,2`

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
	mkdir -p $(INSTALL_DIR)/include/ammo-gateway
	mkdir -p $(DESTDIR)/etc/ammo-gateway
	echo "Installing binaries ..."
	install -m 755 build/bin/AndroidGatewayPlugin $(INSTALL_DIR)/bin
	install -m 755 build/bin/AtsGatewayPlugin $(INSTALL_DIR)/bin
	install -m 755 build/bin/GatewayCore $(INSTALL_DIR)/bin
	install -m 755 build/bin/GatewayUsbTransfer $(INSTALL_DIR)/bin
	install -m 755 build/bin/LdapGatewayPlugin $(INSTALL_DIR)/bin
	install -m 755 build/bin/DataStoreGatewayPlugin $(INSTALL_DIR)/bin
	install -m 755 build/bin/PassGatewayPlugin $(INSTALL_DIR)/bin
	install -m 755 build/bin/SamplePushReceiverGatewayPlugin $(INSTALL_DIR)/bin
	install -m 755 build/bin/SamplePushTestDriverPlugin $(INSTALL_DIR)/bin
	install -m 755 build/bin/SpotPushReceiverGatewayPlugin $(INSTALL_DIR)/bin
	echo "Installing headers ..."
	install -m 755 LibGatewayConnector/GatewayConnector.h $(INSTALL_DIR)/include/ammo-gateway
	install -m 755 LibGatewayConnector/Enumerations.h $(INSTALL_DIR)/include/ammo-gateway	
	install -m 755 LibGatewayConnector/LibGatewayConnector_Export.h $(INSTALL_DIR)/include/ammo-gateway	
	install -m 755 common/log.h $(INSTALL_DIR)/include/ammo-gateway	
	install -m 755 common/version.h $(INSTALL_DIR)/include/ammo-gateway	
	echo "Installing JSON headers..."
	install -m 755 LibJSON/autolink.h $(INSTALL_DIR)/include/ammo-json
	install -m 755 LibJSON/config.h $(INSTALL_DIR)/include/ammo-json
	install -m 755 LibJSON/features.h $(INSTALL_DIR)/include/ammo-json
	install -m 755 LibJSON/json.h $(INSTALL_DIR)/include/ammo-json
	install -m 755 LibJSON/reader.h $(INSTALL_DIR)/include/ammo-json
	install -m 755 LibJSON/value.h $(INSTALL_DIR)/include/ammo-json
	install -m 755 LibJSON/writer.h $(INSTALL_DIR)/include/ammo-json
	echo "Installing libs ..."
	install -m 644 build/lib/libgatewayconnector.so.$(VERSION) $(INSTALL_DIR)/lib
	ln -s $(INSTALL_DIR_BASE)/lib/libgatewayconnector.so.$(VERSION) $(INSTALL_DIR)/lib/libgatewayconnector.so
	install -m 644 build/lib/libgeotrans-mgrs.so.$(VERSION) $(INSTALL_DIR)/lib
	ln -s $(INSTALL_DIR_BASE)/lib/libgeotrans-mgrs.so.$(VERSION) $(INSTALL_DIR)/lib/libgeotrans-mgrs.so
	install -m 644 build/lib/libjson.so.$(VERSION) $(INSTALL_DIR)/lib
	ln -s $(INSTALL_DIR_BASE)/lib/libjson.so.$(VERSION) $(INSTALL_DIR)/lib/libjson.so
	echo "Installing scripts ..."
	install -m 755 dist/template/bin/launch_ammo_gateway_headless.sh $(INSTALL_DIR)/bin
	install -m 755 dist/template/bin/launch_ammo_gateway.sh $(INSTALL_DIR)/bin
	install -m 755 dist/template/bin/kill_all_gateway.sh $(INSTALL_DIR)/bin
	echo "Installing config files ..."
	install -m 644 build/etc/AtsPluginConfig.json $(DESTDIR)/etc/ammo-gateway
	install -m 644 build/etc/GatewayConfig.json $(DESTDIR)/etc/ammo-gateway
	install -m 644 build/etc/LdapPluginConfig.json $(DESTDIR)/etc/ammo-gateway
	install -m 644 build/etc/DataStorePluginConfig.json $(DESTDIR)/etc/ammo-gateway
	install -m 644 build/etc/PassPluginConfig.json $(DESTDIR)/etc/ammo-gateway

