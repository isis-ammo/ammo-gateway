#ifndef TERSE_ENCODER_H
#define TERSE_ENCODER_H

#include <string>

#include "Typedefs.h"

class TerseEncoder {
public:
  TerseEncoder();
  ~TerseEncoder();

  bool encodeTerseFromJson(const std::string &mimeType, const std::string &jsonData, std::string &terseOutput);
private:
  bool encodePli(const std::string &jsonData, std::string &terseOutput);
  bool encodeChat(const std::string &jsonData, std::string &terseOutput);

};

#endif //TERSE_ENCODER_H
