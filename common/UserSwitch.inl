#include <ace/OS_NS_pwd.h>
#include <ace/OS_NS_unistd.h>

const char *AMMO_USER = "ammo-gateway";

void dropPrivileges() {
#if !defined(ACE_LACKS_PWD_FUNCTIONS) || !defined(ACE_LACKS_GETUID) || !defined(ACE_LACKS_SETUID)
  if(ACE_OS::getuid() == 0) {
    struct passwd *user = ACE_OS::getpwnam(AMMO_USER);
    if(user != NULL) {
      int result = ACE_OS::setgid(user->pw_gid); //have to switch gids first; a non-root user can't switch its own gid, apparently
      if(result == 0) {
        result = ACE_OS::setuid(user->pw_uid);
        if(result == 0) {
          LOG_TRACE("Successfully switched to ammo-gateway user.");
        } else {
          LOG_WARN("setuid failed when switching to daemon user " << AMMO_USER << " " << ACE_OS::last_error());
        }
      } else {
        LOG_WARN("setgid failed when switching to daemon user " << AMMO_USER << ACE_OS::last_error());
      }
    } else {
      LOG_DEBUG("Daemon user " << AMMO_USER << " could not be found.");
    }
  } else {
    LOG_DEBUG("Already running as an unprivileged user; not attempting to change to ammo-gateway");
  }
#endif
}

//:mode=c++: (jEdit modeline for syntax highlighting)
