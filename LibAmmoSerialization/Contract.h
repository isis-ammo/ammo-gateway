#ifndef CONTRACT_H
#define CONTRACT_H

#include <string>
#include <vector>
#include <set>
#include <map>

#include <tr1/memory>

#include "tinyxml2.h"

namespace ammo {
  namespace gateway {
    enum FieldType {
      FIELD_TYPE_NULL = 0,
      FIELD_TYPE_BOOL = 1,
      FIELD_TYPE_BLOB = 2,
      FIELD_TYPE_FLOAT = 3,
      FIELD_TYPE_INTEGER = 4,
      FIELD_TYPE_LONG = 5,
      FIELD_TYPE_TEXT = 6,
      FIELD_TYPE_REAL = 7,
      FIELD_TYPE_FK = 8,
      FIELD_TYPE_GUID = 9,
      FIELD_TYPE_EXCLUSIVE = 10,
      FIELD_TYPE_INCLUSIVE = 11,
      FIELD_TYPE_TIMESTAMP = 12,
      FIELD_TYPE_SHORT = 13,
      FIELD_TYPE_FILE = 14
    };

    FieldType fieldTypeFromString(const std::string &typeString);

    class Name {
    public:
      Name();
      Name(std::string unformattedName);

      std::string getName() const { return unformattedName; };
      std::string toCamelCase() const { return camelCaseName; };
      std::string toSnakeCase() const { return snakeCaseName; };
      std::string toCobraCase() const { return cobraCaseName; };

    private:
      std::string unformattedName;
      std::string camelCaseName;
      std::string snakeCaseName;
      std::string cobraCaseName;

      static std::string makeCamelCase(std::string name);
      static std::string makeSnakeCase(std::string name);
      static std::string makeCobraCase(std::string name);
    };

    class FieldRef {
    public:
      FieldRef(tinyxml2::XMLElement *fieldRefNode);

      Name getRefName() const { return refName; }
      FieldType getConvertToType() const { return convertToType; }
      bool isConvertToTypeSet() const { return convertToTypeSet; }

    private:
      Name refName;
      FieldType convertToType;
      bool convertToTypeSet;
    };

    class Message {
    public:
      typedef std::vector<FieldRef> FieldRefVector;

      Message(tinyxml2::XMLElement *messageNode);

      const std::string &getEncoding() const { return encoding; }
      const FieldRefVector &getFieldRefs() const { return fieldRefs; }

    private:
      std::string encoding;
      FieldRefVector fieldRefs;
    };

    class Field {
    public:
      Field(tinyxml2::XMLElement *fieldNode);

      FieldType getType() const { return type; }
      Name getName() const { return name; }
      const std::string &getDefaultValue() const { return defaultValue; }
      const bool getAllowNull() const { return allowNull; }
    private:
      FieldType type;
      Name name;
      std::string defaultValue;
      bool allowNull;
    };

    class Relation {
    public:
      typedef std::vector<Field> FieldVector;
      typedef std::map<std::string, Message> MessageMap;

      Relation(tinyxml2::XMLElement *relationNode);

      Name getName() const { return name; }

      const FieldVector &getFields() const { return fields; }
      const MessageMap &getMessages() const { return messages; }

    private:
      Name name;
      FieldVector fields;
      MessageMap messages;
    };

    class Contract {
    public:
      typedef std::map<std::string, Relation> RelationMap;

      Contract(tinyxml2::XMLElement *contractRoot);

      /*
      * Gets the sponsor name for this contract.
      */
      std::string getSponsor() const { return sponsor; }

      /*
      * These functions get the iterators required to iterate across
      * all relations in the contract.
      */
      const RelationMap &getRelations() const { return relations; }
    private:
      std::string sponsor;
      RelationMap relations;
    };

    /*
    * Parses an XML document (contained in a string) and returns the root.
    * This is intended as a helper for unit testing.
    */
    std::tr1::shared_ptr<tinyxml2::XMLDocument> parseXml(const std::string &xmlString);
  };
};

#endif
