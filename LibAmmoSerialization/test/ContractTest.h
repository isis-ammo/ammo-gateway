#ifndef CONTRACT_TEST_H
#define CONTRACT_TEST_H

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Portability.h>

class ContractTest : public CPPUNIT_NS::TestFixture {
  CPPUNIT_TEST_SUITE(ContractTest);
  CPPUNIT_TEST(testEmptyContract);
  CPPUNIT_TEST(testNameEmptyUnformatted);
  CPPUNIT_TEST(testNameEmptyCamel);
  CPPUNIT_TEST(testNameEmptySnake);
  CPPUNIT_TEST(testNameEmptyCobra);

  CPPUNIT_TEST(testNameOneWordUnformatted);
  CPPUNIT_TEST(testNameOneWordCamel);
  CPPUNIT_TEST(testNameOneWordSnake);
  CPPUNIT_TEST(testNameOneWordCobra);

  CPPUNIT_TEST(testNameMultiWordUnformatted);
  CPPUNIT_TEST(testNameMultiWordCamel);
  CPPUNIT_TEST(testNameMultiWordSnake);
  CPPUNIT_TEST(testNameMultiWordCobra);
  CPPUNIT_TEST_SUITE_END();

public:
  ContractTest();
  virtual ~ContractTest();

  virtual void setUp();
  virtual void tearDown();

  void testNameEmptyUnformatted();
  void testNameEmptyCamel();
  void testNameEmptySnake();
  void testNameEmptyCobra();

  void testNameOneWordUnformatted();
  void testNameOneWordCamel();
  void testNameOneWordSnake();
  void testNameOneWordCobra();

  void testNameMultiWordUnformatted();
  void testNameMultiWordCamel();
  void testNameMultiWordSnake();
  void testNameMultiWordCobra();

  void testEmptyContract();

private:
  ContractTest( const ContractTest &copy);
  void operator=(const ContractTest &copy);
};

#endif //CONTRACT_TEST_H
