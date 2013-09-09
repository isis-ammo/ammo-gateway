#ifndef CONTRACT_H
#define CONTRACT_H

#include <string>
#include <list>
#include <set>
#include <map>

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

    FieldType fieldTypeFromString(std::string typeString);

    class Name {
    public:
      Name(std::string unformattedName);

      std::string getName() const;
      std::string toCamelCase() const;
      std::string toSnakeCase() const;
      std::string toCobraCase() const;

    private:
      std::string unformattedName;
      std::string camelCaseName;
      std::string snakeCaseName;
      std::string cobraCaseName;
    };

    class FieldRef {
    public:
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
      typedef std::list<FieldRef> FieldRefList;

      const std::string &getEncoding() const { return encoding; }
      const FieldRefList &getFieldRefs() const { return fieldRefs; }

    private:
      std::string encoding;
      FieldRefList fieldRefs;
    };

    class Field {
    public:
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
      typedef std::list<Field> FieldList;
      typedef std::set<Message> MessageSet;

      Name getName() const { return name; }

      const FieldList &getFields() const { return fields; }
      const MessageSet &getMessages() const { return messages; }

    private:
      Name name;
      FieldList fields;
      MessageSet messages;
    };

    class Contract {
    public:
      typedef std::map<std::string, Relation> RelationMap;

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
  };
};

#endif
