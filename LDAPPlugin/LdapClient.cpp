#include "LdapClient.h"

//------------------------------------------------------------------------------

#ifdef OPENLDAP

#include <cstdlib>     // free()
#include <sys/time.h>  // struct timeval
#include <sstream>     // std::stringstream

/** OpenLDAP client implementation */
class OpenLdapClient : public LdapClient
{
public:
    OpenLdapClient() : LdapClient(), ldap(NULL), retval(LDAP_SUCCESS) { }
    ~OpenLdapClient();

    bool bind(const std::string& dn,
              const std::string& pass);

    int getLastError()            { return this->retval; }
    std::string getLastErrorMsg() { return std::string(ldap_err2string(this->retval)); }

    struct berval** getValuesLen(LDAPMessage* entry,
                                 const char* attr);

    int countEntries(LDAPMessage* result);

    LDAPMessage* firstEntry(LDAPMessage* result);

    bool init(const std::string& host,
              int port);

    bool msgFree(LDAPMessage* msg);

    LDAPMessage* nextEntry(LDAPMessage* entry);

    bool search(const std::string& basedn,
                int scope,
                const std::string& filter,
                char* attrs[],
                int attrsonly,
                LDAPControl** serverctrls,
                LDAPControl** clientctrls,
                const LdapClient::TimeVal& timeout,
                int sizelimit,
                LDAPMessage** results);

    bool setOption(int option,
                   void *invalue);

    bool valueFreeLen(struct berval** val);

private:
    void setTimeval(const LdapClient::TimeVal& src, struct timeval& dest);

private:
    LDAP* ldap;
    int retval;
};

OpenLdapClient::~OpenLdapClient()
{
    ldap_unbind_ext(this->ldap, NULL, NULL);
    free(this->ldap);
    this->ldap = NULL;
}

bool OpenLdapClient::bind(const std::string& dn,
                          const std::string& passwd)
{
    if (!this->ldap) {
        this->retval = LDAP_CONNECT_ERROR;
        return false;
    }

    LDAPControl *serverctrls=NULL, *clientctrls=NULL;
    struct berval *servercredp=NULL;
    struct berval creds;
    creds.bv_val = strdup( passwd.c_str() );
    creds.bv_len = passwd.length();

    this->retval = ldap_sasl_bind_s(ldap,
                                    dn.c_str(),
                                    LDAP_SASL_SIMPLE,
                                    &creds,
                                    &serverctrls,
                                    &clientctrls,
                                    &servercredp);

    return this->retval == LDAP_SUCCESS;
}

int OpenLdapClient::countEntries(LDAPMessage* result)
{
    if (!this->ldap) {
        this->retval = LDAP_CONNECT_ERROR;
        return false;
    }

    int num_entries = ldap_count_entries(ldap, result);

    this->retval = LDAP_OTHER;

    return num_entries;
}

LDAPMessage* OpenLdapClient::firstEntry(LDAPMessage* result)
{
    if (!this->ldap) {
        this->retval = LDAP_CONNECT_ERROR;
        return false;
    }

    LDAPMessage* msg = ldap_first_message(ldap, result);

    this->retval = LDAP_OTHER;

    return msg;
}

struct berval** OpenLdapClient::getValuesLen(LDAPMessage* entry,
                                             const char* attr)
{
    if (!this->ldap) {
        this->retval = LDAP_CONNECT_ERROR;
        return false;
    }

    return ldap_get_values_len(this->ldap, entry, attr);
}

bool OpenLdapClient::init(const std::string& host, int port)
{
    if (this->ldap) {
        this->retval = LDAP_CONNECT_ERROR;
        return false;
    }

    std::stringstream uri_buf;
    uri_buf << "ldap://" << host << ':' << port;

    this->retval = ldap_initialize(&this->ldap, uri_buf.str().c_str());

    return this->retval == LDAP_SUCCESS;
}

bool OpenLdapClient::msgFree(LDAPMessage* msg)
{
    if (!this->ldap) {
        this->retval = LDAP_CONNECT_ERROR;
        return false;
    }

    this->retval = ldap_msgfree(msg);

    return this->retval == LDAP_SUCCESS;
}

LDAPMessage* OpenLdapClient::nextEntry(LDAPMessage* entry)
{
    if (!this->ldap) {
        this->retval = LDAP_CONNECT_ERROR;
        return false;
    }

    return ldap_next_entry(this->ldap, entry);
}

bool OpenLdapClient::search(const std::string& basedn,
                            int scope,
                            const std::string& filter,
                            char* attrs[],
                            int attrsonly,
                            LDAPControl** serverctrls,
                            LDAPControl** clientctrls,
                            const LdapClient::TimeVal& timeout,
                            int sizelimit,
                            LDAPMessage** results)
{
    if (!this->ldap) {
        this->retval = LDAP_CONNECT_ERROR;
        return false;
    }

    struct timeval openldap_timeout;

    this->setTimeval(timeout, openldap_timeout);

    ldap_search_ext_s(this->ldap,
                      basedn.c_str(),
                      scope,
                      filter.c_str(),
                      attrs,
                      attrsonly,
                      serverctrls,
                      clientctrls,
                      &openldap_timeout,
                      sizelimit,
                      results);

    return this->retval == LDAP_SUCCESS;
}

bool OpenLdapClient::setOption(int option, void* invalue)
{
    if (!this->ldap) {
        this->retval = LDAP_CONNECT_ERROR;
        return false;
    }

    this->retval = ldap_set_option(this->ldap,
                                   option,
                                   invalue);

    return this->retval == LDAP_SUCCESS;
}

void OpenLdapClient::setTimeval(const LdapClient::TimeVal& src, struct timeval& dest)
{
    dest.tv_sec  = src.tv_sec;
    dest.tv_usec = src.tv_usec;
}

bool OpenLdapClient::valueFreeLen(struct berval** val)
{
    if (!this->ldap) {
        this->retval = LDAP_CONNECT_ERROR;
        return false;
    }

    ldap_value_free_len(val);
    this->retval = LDAP_SUCCESS;

    return this->retval == LDAP_SUCCESS;
}

#endif  // OPENLDAP

//------------------------------------------------------------------------------

#ifdef WINLDAP

/** Microsoft Windows LDAP client implementation */
class WinLdapClient : public LdapClient
{
public:
    WinLdapClient() : ldap(NULL), retval(LDAP_SUCCESS) { }
    ~WinLdapClient();

    bool bind(const std::string& dn,
              const std::string* passwd);

    int countEntries(LDAPMessage* result);

    LDAPMessage* firstEntry(LDAPMessage* result);

    int getLastError()            { return this->retval; }
    std::string getLastErrorMsg() { return std::string(ldap_err2string(this->retval)); }

    struct berval** getValuesLen(LDAPMessage* entry,
                                 const char* attr);

    bool init(const std::string& host,
              int port);

    bool msgFree(LDAPMessage* msg);

    LDAPMessage* nextEntry(LDAPMessage* entry);

    bool search(const std::string& basedn,
                int scope,
                const std::string& filter,
                char* attrs[],
                int attrsonly,
                LDAPControl** serverctrls,
                LDAPControl** clientctrls,
                const LdapClient::TimeVal& timeout,
                int sizelimit,
                LDAPMessage** results);

    bool setOption(int option,
                   void *invalue);

    bool valueFreeLen(struct berval** val);

private:
    void setTimeval(const LdapClient::TimeVal& src, struct l_timeval& dest);

private:
    LDAP* ldap;
    int retval;
};

WinLdapClient::~WinLdapClient()
{
    // TODO
}

bool WinLdapClient::bind(const std::string& dn,
                         const std::string& passwd)
{
    if (!this->ldap) {
        this->retval = LDAP_CONNECT_ERROR;
        return false;
    }

    this->retval = ldap_simple_bind_s(this->ldap,
                                      dn.c_str(),
                                      passwd->c_str());

    return this->retval == LDAP_SUCCESS;
}

int WinLdapClient::countEntries(LDAPMessage* result)
{
    if (!this->ldap) {
        this->retval = LDAP_CONNECT_ERROR;
        return false;
    }

    int num_entries = ldap_count_entries(ldap, result);

    this->retval = ld_errno;

    return num_entries;
}

LDAPMessage* WinLdapClient::firstEntry(LDAPMessage* result)
{
    if (!this->ldap) {
        this->retval = LDAP_CONNECT_ERROR;
        return false;
    }

    LDAPMessage* msg = ldap_first_message(ldap, result);

    this->retval = ld_errno;

    return msg;
}

struct berval** WinLdapClient::getValuesLen(LDAPMessage* entry,
                                            const char* attr)
{
    if (!this->ldap) {
        this->retval = LDAP_CONNECT_ERROR;
        return false;
    }

    return ldap_get_values_len(this->ldap, entry, attr);
}

bool WinLdapClient::init(const std::string &host, int port)
{
    this->ldap = ldap_init(host.c_str(), port);
    this->retval = LdapGetLastError();
    return this->retval == LDAP_SUCCESS;
}

bool WinLdapClient::msgFree(LDAPMessage* msg)
{
    if (!this->ldap) {
        this->retval = LDAP_CONNECT_ERROR;
        return false;
    }

    this->retval = ldap_msgfree(msg);

    return this->retval == LDAP_SUCCESS;
}

LDAPMessage* WinnLdapClient::nextEntry(LDAPMessage* entry)
{
    if (!this->ldap) {
        this->retval = LDAP_CONNECT_ERROR;
        return false;
    }

    return ldap_next_entry(this->ldap, entry);
}

bool WinLdapClient::setOption(int option, void* invalue)
{
    if (!this->ldap) {
        this->retval = LDAP_CONNECT_ERROR;
        return false;
    }

    this->retval = ldap_set_option(this->ldap,
                                   option,
                                   invalue);

    return this->retval == LDAP_SUCCESS;
}

void WinLdapClient::setTimeval(const LdapClient::TimeVal& src, struct l_timeval& dest)
{
    dest.tv_sec  = src.tv_sec;
    dest.tv_usec = src.tv_usec;
}

bool WinLdapClient::valueFreeLen(struct berval** val)
{
    if (!this->ldap) {
        this->retval = LDAP_CONNECT_ERROR;
        return false;
    }

    this->retval = ldap_value_free_len(val);

    return this->retval == LDAP_SUCCESS;
}

#endif // WINLDAP

//------------------------------------------------------------------------------

LdapClient* LdapClient::createInstance()
{
    LdapClient* ldap = NULL;

#ifdef OPENLDAP
    ldap = new OpenLdapClient();
#elif  WINLDAP
    ldap = new WinLdapClient();
#else
    #error "You must specify an LDAP define for LdapClient."
#endif

    return ldap;
}

//------------------------------------------------------------------------------
