#include <ace/OS_NS_sys_time.h>

#include "log.h"

#include "StringParser.h"

StringParser::StringParser (void)
  : start_index_ (0),
    end_index_ (0),
    TOKEN_SEPARATOR_ (',')
{
}

void
StringParser::tokenize (std::string &token,
                        const std::string &token_string)
{
  if (end_index_ == std::string::npos)
    {
      LOG_ERROR ("projection string \""
                 << token_string.c_str ()
                 << "\" has too few fields");
      return;
    }
    
  end_index_ = token_string.find (TOKEN_SEPARATOR_, start_index_);
  token = token_string.substr (start_index_,
                               end_index_ - start_index_);
  start_index_ = end_index_ + 1;
}

void
StringParser::fini_check (const std::string &token_string) const
{
  if (end_index_ != std::string::npos)
    {
      LOG_ERROR ("projection string \""
                 << token_string.c_str ()
                 << "\" has too many fields");
    }
}

