
all: ModuleTests

clean depend generated realclean check-syntax $(CUSTOM_TARGETS):
	@$(MAKE) -f Makefile.testLdapConfigurationManager $(@)
	@$(MAKE) -f Makefile.testLdapPushReceiver $(@)

ModuleTests:
	@$(MAKE) -f Makefile.testLdapConfigurationManager generated all
	@$(MAKE) -f Makefile.testLdapPushReceiver generated all

.PHONY: ModuleTests

#LdapGatewayPluginTest:
#	@echo "foo"

