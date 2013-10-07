Summary:        AMMO Gateway
Name:           ammo-gateway
Version:        AMMO_VERSION
Release:        AMMO_RELEASE%{dist}
License:        AMMO
Group:          Applications/Internet
Source:         %{name}-%{version}.tar.gz
URL:            http://ammo.isis.vanderbilt.edu
BuildRoot:      %(mktemp -ud %{_tmppath}/%{name}-%{version}-%{release}-XXXXXX)
Requires:       ace = 6.0.2, gsoap, protobuf
Requires(pre):  glibc-common, shadow-utils
Requires(post): chkconfig
Requires(preun): chkconfig
Requires(preun): initscripts
BuildRequires:  gcc-c++, ace-devel = 6.0.2, gsoap, gsoap-devel, protobuf-compiler, protobuf-devel

%define USERNAME  ammo-gateway
%define GROUPNAME ammo-gateway
%define HOMEDIR   /var/run/ammo-gateway

%description
Android Middleware Server


%prep
%setup -q

%build
ant configure
mwc.pl --type make Gateway.mwc
cat install.mk >> GNUmakefile
make PROTOBUF_ROOT=/usr GATEWAY_ROOT=`pwd`
ant build

%install
make DESTDIR=%{buildroot} PROTOBUF_ROOT=/usr GATEWAY_ROOT=`pwd` install

%pre
getent group %{GROUPNAME} >/dev/null || groupadd -r %{GROUPNAME}
getent passwd %{USERNAME} >/dev/null || \
    useradd -r -g %{GROUPNAME} -d %{HOMEDIR} -s /sbin/nologin \
    -c "AMMO Gateway User" %{USERNAME}
if [ $1 -eq 2 ] ; then
    /sbin/service ammo-gateway stop >/dev/null 2>&1
fi
exit 0

%post
/sbin/ldconfig
chown ammo-gateway:ammo-gateway /var/log/ammo-gateway /var/run/ammo-gateway /var/db/ammo-gateway /etc/ammo-gateway/keys
chmod 700 /etc/ammo-gateway/keys
if [ $1 -eq 1 ] ; then
    /sbin/chkconfig --add ammo-gateway
fi
/sbin/service ammo-gateway start >/dev/null 2>&1

%preun
if [ $1 -eq 0 ] ; then
    /sbin/service ammo-gateway stop >/dev/null 2>&1
    /sbin/chkconfig --del ammo-gateway
fi


%postun -p /sbin/ldconfig

%clean
rm -rf %{buildroot}

%files
%defattr(-, root, root, -)
/etc/init.d/ammo-gateway
/etc/ammo-gateway/AtsPluginConfig.json
/etc/ammo-gateway/GatewayConfig.json
/etc/ammo-gateway/LdapPluginConfig.json
/etc/ammo-gateway/LoggingConfig.json
/etc/ammo-gateway/DataStorePluginConfig.json
/etc/ammo-gateway/SerialPluginConfig.json
/etc/ammo-gateway/keys
/etc/ammo-gateway/jgroups/udp.xml
/etc/ammo-gateway/jgroups/udpMedia.xml
/usr/bin/AndroidGatewayPlugin
/usr/bin/AtsGatewayPlugin
/usr/bin/GatewayCore
/usr/bin/GatewayUsbTransfer
/usr/bin/LdapGatewayPlugin
/usr/bin/DataStoreGatewayPlugin
/usr/bin/SamplePushReceiverGatewayPlugin
/usr/bin/SamplePushTestDriverPlugin
/usr/bin/SpotPushReceiverGatewayPlugin
/usr/bin/kill_all_gateway.sh
/usr/bin/launch_ammo_gateway.sh
/usr/bin/launch_ammo_gateway_headless.sh
/usr/bin/mcastplugin.sh
/usr/bin/rmcastplugin.sh
/usr/include/ammo-gateway/GatewayConnector.h
/usr/include/ammo-gateway/Enumerations.h
/usr/include/ammo-gateway/LibGatewayConnector_Export.h
/usr/include/ammo-gateway/log.h
/usr/include/ammo-gateway/version.h
/usr/include/ammo-json/autolink.h
/usr/include/ammo-json/config.h
/usr/include/ammo-json/features.h
/usr/include/ammo-json/forwards.h
/usr/include/ammo-json/json.h
/usr/include/ammo-json/reader.h
/usr/include/ammo-json/value.h
/usr/include/ammo-json/writer.h
/usr/lib/libgatewayconnector.so
/usr/lib/libgatewayconnector.so.AMMO_VERSION_STRING
/usr/lib/libgeotrans-mgrs.so
/usr/lib/libgeotrans-mgrs.so.AMMO_VERSION_STRING
/usr/lib/libjson.so
/usr/lib/libjson.so.AMMO_VERSION_STRING
/usr/share/java/gatewaypluginapi.jar
/usr/share/java/mcastplugin.jar
/usr/share/java/rmcastplugin.jar
/var/log/ammo-gateway
/var/db/ammo-gateway
/var/run/ammo-gateway

%changelog
* BUILD_DATE John Williams <johnwilliams@isis.vanderbilt.edu> - AMMO_VERSION
- Autobuild Release for AMMO project

