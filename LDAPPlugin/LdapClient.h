#ifndef LDAPCLIENT_H
#define LDAPCLIENT_H

// Sanity check for LDAP library selection #define
#ifdef OPENLDAP
#elif  WINLDAP
#else
  #error "You must specify an LDAP define for LdapClient."
#endif

// Include the library specific LDAP header
#ifdef OPENLDAP
  #include <ldap.h>
#elif WINLDAP
  #include <Windows.h>
  #include <Winldap.h>
#endif


#include <string>

/**
 * LdapClient provides a wrapper about LDAP calls which vary slightly between implementations.
 * Supported LDAP implementations and required defines are called out below:
 *
 * OPENLDAP - OpenLDAP
 * WINLDAP  - Microsoft's LDAP for Windows
 */
class LdapClient
{
public:
    struct TimeVal
    {
        long long tv_sec;
        long long tv_usec;
    };

public:
    /** Create an instance of an LdapClient. */
    static LdapClient* createInstance();

public:
    virtual ~LdapClient() { }

    /** Bind to a host using the credentials provided. */
    virtual bool bind(const std::string& dn,
                      const std::string& passwd) = 0;

    /** Get the number of entries in a search result. */
    virtual int countEntries(LDAPMessage* result) = 0;

    /** Get the first entry of a search result. */
    virtual LDAPMessage* firstEntry(LDAPMessage* result) = 0;

    /** Get the LDAP error code of the last operation. */
    virtual int getLastError() = 0;

    /** Get the human readable string version for the LDAP error code of the last operation. */
    virtual std::string getLastErrorMsg() = 0;

    /** Return berval structures with values of the attribute specified or NULL if not found. */
    virtual struct berval** getValuesLen(LDAPMessage* entry,
                                         const char* attr) = 0;

    /** Initialize to the host specified. */
    virtual bool init(const std::string& host,
                      int port) = 0;

    /** Free memory used by LDAPMessages when done with them. */
    virtual bool msgFree(LDAPMessage* msg) = 0;

    /** Get the next entry from a search result. */
    virtual LDAPMessage* nextEntry(LDAPMessage* entry) = 0;

    /** Search the LDAP directory. */
    virtual bool search(const std::string& basedn,
                        int scope,
                        const std::string& filter,
                        char* attrs[],
                        int attrsonly,
                        LDAPControl** serverctrls,
                        LDAPControl** clientctrls,
                        const LdapClient::TimeVal& timeout,
                        int sizelimit,
                        LDAPMessage** results) = 0;

    /** Set LDAP options. */
    virtual bool setOption(int option,
                           void* invalue) = 0;

    /** Free berval structures. Use with getValuesLen(). */
    virtual bool valueFreeLen(struct berval** val) = 0;

protected:
    LdapClient() { }
};

#endif // LDAPCLIENT_H
