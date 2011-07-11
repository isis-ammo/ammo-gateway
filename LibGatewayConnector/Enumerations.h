#ifndef ENUMERATIONS_H
#define ENUMERATIONS_H

namespace ammo {
  namespace gateway {
    enum MessageScope {
      SCOPE_GLOBAL = 0,
      SCOPE_LOCAL = 1
    };
    
    enum AuthenticationMessageType {
      CLIENT_NONCE,
      SERVER_NONCE,
      CLIENT_KEYXCHANGE,
      CLIENT_PHNAUTH,
      CLIENT_FINISH,
      SERVER_FINISH,
      STATUS
    };
  }
}

#endif //ENUMERATIONS_H
