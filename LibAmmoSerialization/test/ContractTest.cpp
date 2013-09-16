#include "ContractTest.h"

#include <Contract.h>

using namespace ammo::gateway;

CPPUNIT_TEST_SUITE_REGISTRATION(ContractTest);

ContractTest::ContractTest() {

}

ContractTest::~ContractTest() {

}

void ContractTest::setUp() {

}

void ContractTest::tearDown() {

}

void ContractTest::testEmptyContract() {
}

void ContractTest::testNameEmptyUnformatted() {
  Name n("");

  CPPUNIT_ASSERT_EQUAL(std::string(""), n.getName());
}

void ContractTest::testNameEmptyCamel() {
  Name n("");

  CPPUNIT_ASSERT_EQUAL(std::string(""), n.toCamelCase());
}

void ContractTest::testNameEmptySnake() {
  Name n("");

  CPPUNIT_ASSERT_EQUAL(std::string(""), n.toSnakeCase());
}

void ContractTest::testNameEmptyCobra() {
  Name n("");

  CPPUNIT_ASSERT_EQUAL(std::string(""), n.toCobraCase());
}

void ContractTest::testNameOneWordUnformatted() {
  Name n("test");

  CPPUNIT_ASSERT_EQUAL(std::string("test"), n.getName());
}

void ContractTest::testNameOneWordCamel() {
  Name n("test");

  CPPUNIT_ASSERT_EQUAL(std::string("test"), n.toCamelCase());
}

void ContractTest::testNameOneWordSnake() {
  Name n("test");

  CPPUNIT_ASSERT_EQUAL(std::string("test"), n.toSnakeCase());
}

void ContractTest::testNameOneWordCobra() {
  Name n("test");

  CPPUNIT_ASSERT_EQUAL(std::string("Test"), n.toCobraCase());
}

void ContractTest::testNameMultiWordUnformatted() {
  Name n("test multiple words");

  CPPUNIT_ASSERT_EQUAL(std::string("test multiple words"), n.getName());
}

void ContractTest::testNameMultiWordCamel() {
  Name n("test multiple words");

  CPPUNIT_ASSERT_EQUAL(std::string("testMultipleWords"), n.toCamelCase());
}

void ContractTest::testNameMultiWordSnake() {
  Name n("test multiple words");

  CPPUNIT_ASSERT_EQUAL(std::string("test_multiple_words"), n.toSnakeCase());
}

void ContractTest::testNameMultiWordCobra() {
  Name n("test multiple words");

  CPPUNIT_ASSERT_EQUAL(std::string("TestMultipleWords"), n.toCobraCase());
}

void testFieldRefNoConvert() {
  const std::string testXml = "<field ref=\"sender\" />";

  std::tr1::shared_ptr<pugi::xml_node> fieldRefNode = parseXml(testXml);

  FieldRef fieldRef(fieldRefNode.get());

  CPPUNIT_ASSERT_EQUAL(std::string("sender"), fieldRef.getRefName().getName());
  CPPUNIT_ASSERT_EQUAL(false, fieldRef.isConvertToTypeSet());
}

void testFieldRefConvert() {
  const std::string testXml = "<field ref=\"sender\" type=\"INTEGER\"/>";

  std::tr1::shared_ptr<pugi::xml_node> fieldRefNode = parseXml(testXml);

  FieldRef fieldRef(fieldRefNode.get());

  CPPUNIT_ASSERT_EQUAL(std::string("sender"), fieldRef.getRefName().getName());
  CPPUNIT_ASSERT_EQUAL(true, fieldRef.isConvertToTypeSet());
  CPPUNIT_ASSERT_EQUAL(FIELD_TYPE_INTEGER, fieldRef.getConvertToType());
}

void testMessageNoFieldRefs() {
  const std::string testXml = "<message encoding=\"terse\"></message>";

  std::tr1::shared_ptr<pugi::xml_node> messageNode = parseXml(testXml);

  Message message(messageNode.get());

  CPPUNIT_ASSERT_EQUAL(std::string("terse"), message.getEncoding());
  CPPUNIT_ASSERT_EQUAL(true, message.getFieldRefs().empty());
}

void testMessageWithFieldRefs() {
  const std::string testXml = "<message encoding=\"json\"><field ref=\"field a\" /><field ref=\"field b\" /></message>";

  std::tr1::shared_ptr<pugi::xml_node> messageNode = parseXml(testXml);

  Message message(messageNode.get());

  CPPUNIT_ASSERT_EQUAL(std::string("terse"), message.getEncoding());
  CPPUNIT_ASSERT_EQUAL(2UL, message.getFieldRefs().size());
  CPPUNIT_ASSERT_EQUAL(std::string("field_a"), message.getFieldRefs()[0].getRefName().toSnakeCase());
  CPPUNIT_ASSERT_EQUAL(std::string("field_b"), message.getFieldRefs()[1].getRefName().toSnakeCase());
}

void testField() {
  const std::string testXml = "<field type=\"TEXT\" name=\"test field\" default=\"default value\" null=\"no\" />";

  std::tr1::shared_ptr<pugi::xml_node> fieldNode = parseXml(testXml);

  Field field(fieldNode.get());

  CPPUNIT_ASSERT_EQUAL(FIELD_TYPE_TEXT, field.getType());
  CPPUNIT_ASSERT_EQUAL(std::string("test_field"), field.getName().toSnakeCase());
  CPPUNIT_ASSERT_EQUAL(std::string("default value"), field.getDefaultValue());
  CPPUNIT_ASSERT_EQUAL(false, field.getAllowNull());
}

void testFieldImplicitAllowNull() {
  const std::string testXml = "<field type=\"TEXT\" name=\"test field\" default=\"default value\" />";

  std::tr1::shared_ptr<pugi::xml_node> fieldNode = parseXml(testXml);

  Field field(fieldNode.get());

  CPPUNIT_ASSERT_EQUAL(FIELD_TYPE_TEXT, field.getType());
  CPPUNIT_ASSERT_EQUAL(std::string("test_field"), field.getName().toSnakeCase());
  CPPUNIT_ASSERT_EQUAL(std::string("default value"), field.getDefaultValue());
  CPPUNIT_ASSERT_EQUAL(true, field.getAllowNull());
}

void testFieldExplicitAllowNull() {
  const std::string testXml = "<field type=\"TEXT\" name=\"test field\" default=\"default value\" null=\"yes\" />";

  std::tr1::shared_ptr<pugi::xml_node> fieldNode = parseXml(testXml);

  Field field(fieldNode.get());

  CPPUNIT_ASSERT_EQUAL(FIELD_TYPE_TEXT, field.getType());
  CPPUNIT_ASSERT_EQUAL(std::string("test_field"), field.getName().toSnakeCase());
  CPPUNIT_ASSERT_EQUAL(std::string("default value"), field.getDefaultValue());
  CPPUNIT_ASSERT_EQUAL(true, field.getAllowNull());
}

void testRelation() {
  const std::string testXml = "<relation name=\"test relation\">"
                                "<field type=\"INTEGER\" name=\"test field 1\" />"
                                "<field type=\"INTEGER\" name=\"test field 2\" />"
                                "<message encoding=\"terse\"></message>"
                                "<message encoding=\"json\"></message>"
                              "</relation>";

  std::tr1::shared_ptr<pugi::xml_node> relationNode = parseXml(testXml);

  Relation relation(relationNode.get());

  CPPUNIT_ASSERT_EQUAL(std::string("testRelation"), relation.getName().toCamelCase());

  //test iteration across fields
  Relation::FieldVector::const_iterator fieldIt = relation.getFields().begin();
  CPPUNIT_ASSERT(fieldIt != relation.getFields().end());
  CPPUNIT_ASSERT_EQUAL(std::string("test_field_1"), fieldIt->getName().toSnakeCase());
  fieldIt++;
  CPPUNIT_ASSERT(fieldIt != relation.getFields().end());
  CPPUNIT_ASSERT_EQUAL(std::string("test_field_2"), fieldIt->getName().toSnakeCase());
  fieldIt++;
  CPPUNIT_ASSERT(fieldIt == relation.getFields().end());

  //test iteration across messages
  Relation::MessageMap::const_iterator messageIt = relation.getMessages().begin();
  CPPUNIT_ASSERT(messageIt != relation.getMessages().end());
  CPPUNIT_ASSERT_EQUAL(std::string("terse"), messageIt->first);
  CPPUNIT_ASSERT_EQUAL(std::string("terse"), messageIt->second.getEncoding());
  messageIt++;
  CPPUNIT_ASSERT(messageIt != relation.getMessages().end());
  CPPUNIT_ASSERT_EQUAL(std::string("json"), messageIt->first);
  CPPUNIT_ASSERT_EQUAL(std::string("json"), messageIt->second.getEncoding());
  messageIt++;
  CPPUNIT_ASSERT(messageIt == relation.getMessages().end());

  //test direct access to messages
  CPPUNIT_ASSERT_EQUAL(std::string("terse"), relation.getMessages().at("terse").getEncoding());
  CPPUNIT_ASSERT_EQUAL(std::string("json"), relation.getMessages().at("json").getEncoding());
}

void testContract() {
  const std::string testXml = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                              "<content-provider name=\"test\">"
                                "<sponsor name=\"edu.vu.isis.ammo.gateway.test\" />"
                                "<relation name=\"test relation 1\"></relation>"
                                "<relation name=\"test relation 2\"></relation>"
                              "</content-provider>";

  std::tr1::shared_ptr<pugi::xml_node> contractNode = parseXml(testXml);

  Contract contract(contractNode.get());

  CPPUNIT_ASSERT_EQUAL(std::string("edu.vu.isis.ammo.gateway.test"), contract.getSponsor());

  //test direct access to relations
  CPPUNIT_ASSERT_EQUAL(std::string("testRelation1"), contract.getRelations().at("test relation 1").getName().toCamelCase());
  CPPUNIT_ASSERT_EQUAL(std::string("testRelation2"), contract.getRelations().at("test relation 2").getName().toCamelCase());
}
