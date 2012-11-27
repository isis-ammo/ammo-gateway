#ifndef SERVICEDESC_H
#define SERVICEDESC_H

/** Service Description */
struct ServiceDesc_t
{
    std::string humanName;   /// name as read by a human
    std::string svcName;     /// name as given to Windows Service Manager
    std::string configName;  /// configuration file name
    std::string logName;     /// name as given to logging mechanism
};

#endif // SERVICEDESC_H
