#include "Contract.h"

#include <sstream>
#include <algorithm>
#include <cctype>
#include <iostream>

using namespace ammo::gateway;

FieldType fieldTypeFromString(const std::string &typeString) {
  if(typeString == "NULL") {
    return FIELD_TYPE_NULL;
  } else if(typeString == "BOOL") {
    return FIELD_TYPE_BOOL;
  } else if(typeString == "BLOB") {
    return FIELD_TYPE_BLOB;
  } else if(typeString == "FLOAT") {
    return FIELD_TYPE_FLOAT;
  } else if(typeString == "INTEGER") {
    return FIELD_TYPE_INTEGER;
  } else if(typeString == "LONG") {
    return FIELD_TYPE_LONG;
  } else if(typeString == "TEXT") {
    return FIELD_TYPE_TEXT;
  } else if(typeString == "REAL") {
    return FIELD_TYPE_REAL;
  } else if(typeString == "FK") {
    return FIELD_TYPE_FK;
  } else if(typeString == "GUID") {
    return FIELD_TYPE_GUID;
  } else if(typeString == "EXCLUSIVE") {
    return FIELD_TYPE_EXCLUSIVE;
  } else if(typeString == "INCLUSIVE") {
    return FIELD_TYPE_INCLUSIVE;
  } else if(typeString == "TIMESTAMP") {
    return FIELD_TYPE_TIMESTAMP;
  } else if(typeString == "SHORT") {
    return FIELD_TYPE_SHORT;
  } else if(typeString == "FILE") {
    return FIELD_TYPE_FILE;
  } else {
    return FIELD_TYPE_NULL;
  }
}

Name::Name() :
unformattedName(""),
camelCaseName(""),
snakeCaseName(""),
cobraCaseName("")
{

}

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

FieldRef::FieldRef(tinyxml2::XMLElement *fieldRefNode) {

}

Message::Message(tinyxml2::XMLElement *messageNode) {

}

Field::Field(tinyxml2::XMLElement *fieldNode) {

}

Relation::Relation(tinyxml2::XMLElement *relationNode) {

}

Contract::Contract(tinyxml2::XMLElement *contractRoot) {

}

std::tr1::shared_ptr<tinyxml2::XMLDocument> ammo::gateway::parseXml(const std::string &xmlString) {
  std::tr1::shared_ptr<tinyxml2::XMLDocument> document(new tinyxml2::XMLDocument);

  document->Parse(xmlString.c_str());

  std::cout << document->FirstChildElement()->Name() << std::endl;

  return document;
}
