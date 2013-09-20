#ifndef CONTRACT_TEST_H
#define CONTRACT_TEST_H

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Portability.h>

class ContractTest : public CPPUNIT_NS::TestFixture {
  CPPUNIT_TEST_SUITE(ContractTest);
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

  CPPUNIT_TEST(testEmptyContract);

  CPPUNIT_TEST(testFieldRefNoConvert);
  CPPUNIT_TEST(testFieldRefConvert);

  CPPUNIT_TEST(testMessageNoFieldRefs);
  CPPUNIT_TEST(testMessageWithFieldRefs);

  CPPUNIT_TEST(testField);
  CPPUNIT_TEST(testFieldImplicitAllowNull);
  CPPUNIT_TEST(testFieldExplicitAllowNull);

  CPPUNIT_TEST(testRelation);

  CPPUNIT_TEST(testContract);

  CPPUNIT_TEST(testFieldRefNoName);
  CPPUNIT_TEST(testFieldRefEmptyName);
  CPPUNIT_TEST(testMessageNoEncoding);
  CPPUNIT_TEST(testMessageEmptyEncoding);
  CPPUNIT_TEST(testFieldNoType);
  CPPUNIT_TEST(testFieldEmptyType);
  CPPUNIT_TEST(testFieldNoName);
  CPPUNIT_TEST(testFieldEmptyName);
  CPPUNIT_TEST(testFieldInvalidNull);
  CPPUNIT_TEST(testRelationNoName);
  CPPUNIT_TEST(testRelationEmptyName);
  CPPUNIT_TEST(testContractMissingSponsor);
  CPPUNIT_TEST(testContractNoSponsorName);
  CPPUNIT_TEST(testContractEmptySponsorName);

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

  void testFieldRefNoConvert();
  void testFieldRefConvert();

  void testMessageNoFieldRefs();
  void testMessageWithFieldRefs();

  void testField();
  void testFieldImplicitAllowNull();
  void testFieldExplicitAllowNull();

  void testRelation();

  void testContract();

  //tests for correct exception handling
  void testFieldRefNoName();
  void testFieldRefEmptyName();
  void testMessageNoEncoding();
  void testMessageEmptyEncoding();
  void testFieldNoType();
  void testFieldEmptyType();
  void testFieldNoName();
  void testFieldEmptyName();
  void testFieldInvalidNull();
  void testRelationNoName();
  void testRelationEmptyName();
  void testContractMissingSponsor();
  void testContractNoSponsorName();
  void testContractEmptySponsorName();

private:
  ContractTest( const ContractTest &copy);
  void operator=(const ContractTest &copy);
};

#endif //CONTRACT_TEST_H
