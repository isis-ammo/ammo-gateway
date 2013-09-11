#include "Contract.h"

#include <sstream>
#include <algorithm>
#include <cctype>

using namespace ammo::gateway;

Name::Name(std::string unformattedName) :
unformattedName(unformattedName),
camelCaseName(Name::makeCamelCase(unformattedName)),
snakeCaseName(Name::makeSnakeCase(unformattedName)),
cobraCaseName(Name::makeCobraCase(unformattedName))
{

}

std::string Name::makeCamelCase(std::string name) {
  std::string::size_type position = 0;

  std::ostringstream camelCaseStream;

  while(position != std::string::npos) {
    std::string::size_type nextSpace = name.find(" ", position);

    //if not found, we take everything to the end of the string
    std::string::size_type substringLength = std::string::npos;
    if(nextSpace != std::string::npos) {
      substringLength = nextSpace - position;
    }

    std::string token = name.substr(position, substringLength);

    //make lowercase
    std::transform(token.begin(), token.end(), token.begin(), ::tolower);

    //make first character uppercase, unless this is the first token (position = 0)
    if(position != 0 && token.length() >= 1) {
      token[0] = toupper(token[0]);
    }

    camelCaseStream << token;

    if(nextSpace != std::string::npos) {
      position = nextSpace + 1;
    } else {
      position = std::string::npos;
    }
  }

  return camelCaseStream.str();
}

std::string Name::makeSnakeCase(std::string name) {
  std::string::size_type position = 0;

  std::ostringstream snakeCaseStream;

  while(position != std::string::npos) {
    std::string::size_type nextSpace = name.find(" ", position);

    //if not found, we take everything to the end of the string
    std::string::size_type substringLength = std::string::npos;
    if(nextSpace != std::string::npos) {
      substringLength = nextSpace - position;
    }

    std::string token = name.substr(position, substringLength);

    //make lowercase
    std::transform(token.begin(), token.end(), token.begin(), ::tolower);

    //join with underscores (easiest way to do this is to prefix all but first
    //token with an underscore)
    if(position != 0) {
      snakeCaseStream << "_";
    }

    snakeCaseStream << token;

    if(nextSpace != std::string::npos) {
      position = nextSpace + 1;
    } else {
      position = std::string::npos;
    }
  }

  return snakeCaseStream.str();
}

std::string Name::makeCobraCase(std::string name) {
  std::string::size_type position = 0;

  std::ostringstream camelCaseStream;

  while(position != std::string::npos) {
    std::string::size_type nextSpace = name.find(" ", position);

    //if not found, we take everything to the end of the string
    std::string::size_type substringLength = std::string::npos;
    if(nextSpace != std::string::npos) {
      substringLength = nextSpace - position;
    }

    std::string token = name.substr(position, substringLength);

    //make lowercase
    std::transform(token.begin(), token.end(), token.begin(), ::tolower);

    //make first character uppercase, always (this is the difference between cobra and camel case)
    if(token.length() >= 1) {
      token[0] = toupper(token[0]);
    }

    camelCaseStream << token;

    if(nextSpace != std::string::npos) {
      position = nextSpace + 1;
    } else {
      position = std::string::npos;
    }
  }

  return camelCaseStream.str();
}
