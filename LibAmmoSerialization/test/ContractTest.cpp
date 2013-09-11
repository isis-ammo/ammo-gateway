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
  ammo::gateway::Contract c;

  CPPUNIT_ASSERT_EQUAL(true, c.getRelations().empty());
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
