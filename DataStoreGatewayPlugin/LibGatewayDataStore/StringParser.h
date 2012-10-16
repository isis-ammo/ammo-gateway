#ifndef STRING_PARSER_H
#define STRING_PARSER_H

#include <string>

#include "LibGatewayDataStore_Export.h"

class LibGatewayDataStore_Export StringParser
{
public:
  StringParser (void);
  
protected:
  void tokenize (std::string &token,
                 const std::string &token_string);
                 
  void fini_check (const std::string &token_string) const;

private:
  size_t start_index_;
  size_t end_index_;
  char TOKEN_SEPARATOR_;
};

#endif // STRING_PARSER_H
