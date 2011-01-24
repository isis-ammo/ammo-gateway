#!/bin/bash

function start_gateway() {
gnome-terminal --title="Gateway" \
  --window-with-profile="Gateway" \
  --working-directory="${RELEASE_DIR}" \
  --command="${RELEASE_DIR}/GatewayCore"  
}

function start_ldap_plugin() {
gnome-terminal --title="LDAP Plugin" \
  --window-with-profile="LDAP Plugin" \
  --working-directory="${RELEASE_DIR}" \
  --command="${RELEASE_DIR}/LdapGatewayPlugin"  
}

function start_ldap() {
sudo gnome-terminal --title="LDAP" \
  --window-with-profile="LDAP" \
  --working-directory="${RELEASE_DIR}" \
  --command="/opt/slapy/libexec/slapd -d 10 -f /opt/slapy/etc/openldap/slapd.conf"
}

function start_android_plugin() {
gnome-terminal --title="Android Plugin" \
  --window-with-profile="Android Plugin" \
  --working-directory="${RELEASE_DIR}" \
  --command="${RELEASE_DIR}/AndroidGatewayPlugin --listenPort 33289 
# --loggerConfig ${RELEASE_DIR}/Android.log4cxx"  
}

function start_tigr_plugin() {
gnome-terminal --title="TIGR Plugin" \
  --window-with-profile="TIGR Plugin" \
  --working-directory="${RELEASE_DIR}" \
  --command="${RELEASE_DIR}/TigrGatewayPlugin"  
}

pushd ./build/bin
export RELEASE_DIR=$(dirs +0)

select action in 'EXIT' 'start' 'stop'; do
  case ${action} in 
    'EXIT') break;;
    'start')  
      select subaction in 'EXIT' 'All' 'LDAP' 'Gateway' \
           'LDAP Plugin' 'Android Plugin' 'Tigr Plugin' \
           'PASS Plugin'
      do
        case ${subaction} in
        'EXIT') break;;
        'All') start_gateway
               start_android_plugin
               start_ldap_plugin
               start_pass_plugin
               start_tigr_plugin
        ;;
        'LDAP') start_ldap ;;
        'Gateway') start_gateway ;;
        'LDAP Plugin') start_ldap_plugin ;;
        'Android Plugin') start_android_plugin ;;
        'Tigr Plugin') start_tigr_plugin ;;
        'PASS Plugin') start_pass_plugin ;;
        esac
      done;;
    'stop')  
      select subaction in 'EXIT' 'All' 'LDAP' 'Gateway' \
           'LDAP Plugin' 'Android Plugin' 'Tigr Plugin' \
           'PASS Plugin'
      do
        case ${subaction} in
        'EXIT') break;;
        esac
      done;;
  esac
done
popd
