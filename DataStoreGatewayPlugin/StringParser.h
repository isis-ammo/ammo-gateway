#ifndef STRING_PARSER_H
#define STRING_PARSER_H

#include <string>

class StringParser
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
