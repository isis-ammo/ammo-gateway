#include "Contract.h"

#include <sstream>
#include <algorithm>
#include <cctype>
#include <iostream>

#include <log.h>

using namespace ammo::gateway;

FieldType ammo::gateway::fieldTypeFromString(const std::string &typeString) throw(InvalidContractException) {
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
    throw InvalidContractException("Invalid contract: Invalid field type; assuming NULL");
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

FieldRef::FieldRef(tinyxml2::XMLElement *fieldRefNode) throw(InvalidContractException) : refName(""), convertToType(FIELD_TYPE_NULL), convertToTypeSet(false) {
  const char *newRefName = fieldRefNode->Attribute("ref");
  if(newRefName != NULL) {
    refName = Name(std::string(newRefName));
    if(refName.getName() == "") {
      throw InvalidContractException("Invalid contract: Field reference name cannot be empty");
    }
  } else {
    throw InvalidContractException("Invalid contract: FieldRef found with no ref name");
  }

  const char *newType = fieldRefNode->Attribute("type");
  if(newType != NULL) {
    convertToTypeSet = true;
    convertToType = fieldTypeFromString(newType);
  } else {
    convertToTypeSet = false;
  }
}

Message::Message(tinyxml2::XMLElement *messageNode) throw(InvalidContractException) : encoding(""), fieldRefs() {
  const char *newEncoding = messageNode->Attribute("encoding");
  if(newEncoding != NULL) {
    encoding = newEncoding;
    if(encoding == "") {
      throw InvalidContractException("Invalid contract: Message encoding cannot be empty");
    }
  } else {
    throw InvalidContractException("Invalid contract: Encoding found with no encoding type");
  }

  for(tinyxml2::XMLElement *fieldRefChild = messageNode->FirstChildElement("field"); fieldRefChild != NULL; fieldRefChild = fieldRefChild->NextSiblingElement("field")) {
    FieldRef newFieldRef(fieldRefChild);
    fieldRefs.push_back(newFieldRef);
  }
}

Field::Field(tinyxml2::XMLElement *fieldNode) throw(InvalidContractException) : type(FIELD_TYPE_NULL), name(""), defaultValue(""), allowNull(true) {
  const char *newType = fieldNode->Attribute("type");
  if(newType != NULL) {
    type = fieldTypeFromString(std::string(newType));
  } else {
    throw InvalidContractException("Invalid contract: Field found with no type");
  }

  const char *newName = fieldNode->Attribute("name");
  if(newName != NULL) {
    name = Name(std::string(newName));

    if(name.getName() == "") {
      throw InvalidContractException("Invalid contract: Field name cannot be empty");
    }
  } else {
    throw InvalidContractException("Invalid contract: Field found with no name");
  }

  const char *newDefault = fieldNode->Attribute("default");
  if(newDefault != NULL) {
    defaultValue = newDefault;
  } else {
    defaultValue = "";
  }

  const char *newAllowNull = fieldNode->Attribute("null");
  if(newAllowNull != NULL) {
    if(std::string(newAllowNull) == "yes") {
      allowNull = true;
    } else if(std::string(newAllowNull) == "no") {
      allowNull = false;
    } else {
      throw InvalidContractException("Invalid contract: Invalid value for 'null' attribute (can be 'yes' or 'no'");
    }
  } else {
    allowNull = true;
  }
}

Relation::Relation(tinyxml2::XMLElement *relationNode) throw(InvalidContractException) : name(""), fields(), messages() {
  const char *newName = relationNode->Attribute("name");
  if(newName != NULL) {
    name = Name(std::string(newName));

    if(name.getName() == "") {
      throw InvalidContractException("Invalid contract: Field name cannot be empty");
    }
  } else {
    throw InvalidContractException("Invalid contract: Relation found with no name");
  }

  for(tinyxml2::XMLElement *fieldChild = relationNode->FirstChildElement("field"); fieldChild != NULL; fieldChild = fieldChild->NextSiblingElement("field")) {
    Field newField(fieldChild);
    fields.push_back(newField);
  }

  for(tinyxml2::XMLElement *messageChild = relationNode->FirstChildElement("message"); messageChild != NULL; messageChild = messageChild->NextSiblingElement("message")) {
    Message newMessage(messageChild);
    messages.insert(MessageMap::value_type(newMessage.getEncoding(), newMessage));
  }
}

Contract::Contract(tinyxml2::XMLElement *contractRoot) throw(InvalidContractException) : sponsor(""), relations() {
  tinyxml2::XMLElement *sponsorChild = contractRoot->FirstChildElement("sponsor");
  if(sponsorChild != NULL) {
    const char *sponsorName = sponsorChild->Attribute("name");
    if(sponsorName != NULL) {
      sponsor = sponsorName;

      if(sponsor == "") {
        throw InvalidContractException("Invalid Contract: Sponsor name cannot be empty");
      }
    } else {
      throw InvalidContractException("Invalid contract: Sponsor element contained no name");
    }
  } else {
    throw InvalidContractException("Invalid contract: Contract was missing sponsor");
  }

  for(tinyxml2::XMLElement *relationChild = contractRoot->FirstChildElement("relation"); relationChild != NULL; relationChild = relationChild->NextSiblingElement("relation")) {
    Relation newRelation(relationChild);
    relations.insert(RelationMap::value_type(newRelation.getName().getName(), newRelation));
  }
}

std::tr1::shared_ptr<tinyxml2::XMLDocument> ammo::gateway::parseXml(const std::string &xmlString) {
  std::tr1::shared_ptr<tinyxml2::XMLDocument> document(new tinyxml2::XMLDocument);

  document->Parse(xmlString.c_str());

  return document;
}
