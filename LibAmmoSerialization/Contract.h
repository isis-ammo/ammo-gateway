#ifndef CONTRACT_H
#define CONTRACT_H

#include <string>
#include <list>
#include <set>
#include <map>

#include "Typedefs.h"

namespace ammo {
  namespace gateway {
    class Contract {
      public:
        typedef std::map<Relation> RelationMap;

        /*
        * Gets the sponsor name for this contract.
        */
        const std::string getSponsor() const { return sponsor; }

        /*
        * These functions get the iterators required to iterate across
        * all relations in the contract.
        */
        RelationSet::const_iterator getRelationSetIteratorBegin() const;
        RelationSet::const_iterator getRelationSetIteratorEnd() const;
      private:
        std::string sponsor;
        RelationMap relations;
    };

    class Relation {
      public:
        typedef std::list<Field> FieldList;
        typedef std::set<Message> MessageSet;

        const FieldList::const_iterator getFieldListIteratorBegin() const;
        const FieldList::const_iterator getFieldListIteratorEnd() const;

      private:
        Name name;
        FieldList fields;
        MessageList messages;
    };
  };
};

#endif
