Summary:        AMMO Gateway
Name:           ammo-gateway
Version:        AMMO_VERSION
Release:        AMMO_RELEASE%{dist}
License:        AMMO
Group:          Applications/Internet
Source:         %{name}-%{version}.tar.gz
URL:            http://ammo.isis.vanderbilt.edu
BuildRoot:      %(mktemp -ud %{_tmppath}/%{name}-%{version}-%{release}-XXXXXX)
Requires:       ace = 6.0.2, protobuf, openldap-servers
Requires(pre):  glibc-common, shadow-utils
BuildRequires:  gcc-c++, ace-devel = 6.0.2, protobuf-compiler, protobuf-devel

%define USERNAME  ammo-gateway
%define GROUPNAME ammo-gateway
%define HOMEDIR   /var/run/ammo-gateway

%description
Android Middleware Server


%prep
%setup -q

%build
mwc.pl --type make Gateway.mwc
cat install.mk >> GNUmakefile
make PROTOBUF_ROOT=/usr GATEWAY_ROOT=`pwd`

%install
make DESTDIR=%{buildroot} PROTOBUF_ROOT=/usr GATEWAY_ROOT=`pwd` install

%pre
getent group %{GROUPNAME} >/dev/null || groupadd -r %{GROUPNAME}
getent passwd %{USERNAME} >/dev/null || \
    useradd -r -g %{GROUPNAME} -d %{HOMEDIR} -s /sbin/nologin \
    -c "AMMO Gateway User" %{USERNAME}
exit 0

%post -p /sbin/ldconfig
%postun -p /sbin/ldconfig

%clean
rm -rf %{buildroot}

%files
%defattr(-, root, root, -)
/etc/init.d/ammo-gateway
/etc/ammo-gateway/AtsPluginConfig.json
/etc/ammo-gateway/GatewayConfig.json
/etc/ammo-gateway/LdapPluginConfig.json
/etc/ammo-gateway/DataStorePluginConfig.json
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
/usr/lib/libgatewayconnector.so
/usr/lib/libgatewayconnector.so.AMMO_VERSION_STRING
/usr/lib/libgeotrans-mgrs.so
/usr/lib/libgeotrans-mgrs.so.AMMO_VERSION_STRING
/usr/lib/libjson.so
/usr/lib/libjson.so.AMMO_VERSION_STRING

%changelog
* BUILD_DATE John Williams <johnwilliams@isis.vanderbilt.edu> - AMMO_VERSION
- Autobuild Release for AMMO project

