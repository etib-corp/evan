/*
 Copyright (c) 2025 ETIB Corporation

 Permission is hereby granted, free of charge, to any person obtaining a copy of
 this software and associated documentation files (the "Software"), to deal in
 the Software without restriction, including without limitation the rights to
 use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 of the Software, and to permit persons to whom the Software is furnished to do
 so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
 */

#include "test_Version.hpp"

/**
 * @brief Test suite for the Version class
 */
class VersionTest : public ::testing::Test {
protected:
  void SetUp() override {
    // Setup code if needed
  }

  void TearDown() override {
    // Cleanup code if needed
  }
};

/**
 * @brief Test constructor with valid parameters
 */
TEST_F(VersionTest, ConstructorSetsCorrectValues) {
  evan::Version version(1, 2, 3);
  
  EXPECT_EQ(version._major, 1);
  EXPECT_EQ(version._minor, 2);
  EXPECT_EQ(version._patch, 3);
}

/**
 * @brief Test constructor with zero values
 */
TEST_F(VersionTest, ConstructorWithZeroValues) {
  evan::Version version(0, 0, 0);
  
  EXPECT_EQ(version._major, 0);
  EXPECT_EQ(version._minor, 0);
  EXPECT_EQ(version._patch, 0);
}

/**
 * @brief Test constructor with maximum values
 */
TEST_F(VersionTest, ConstructorWithMaxValues) {
  unsigned int max_val = 999;
  evan::Version version(max_val, max_val, max_val);
  
  EXPECT_EQ(version._major, max_val);
  EXPECT_EQ(version._minor, max_val);
  EXPECT_EQ(version._patch, max_val);
}

/**
 * @brief Test to_uint32_t conversion with standard version numbers
 */
TEST_F(VersionTest, ToUint32tStandardConversion) {
  evan::Version version(1, 0, 0);
  uint32_t result = version.to_uint32_t();
  
  // VK_MAKE_VERSION(1, 0, 0) should produce a specific bit pattern
  uint32_t expected = VK_MAKE_VERSION(1, 0, 0);
  EXPECT_EQ(result, expected);
}

/**
 * @brief Test to_uint32_t conversion with version 1.2.3
 */
TEST_F(VersionTest, ToUint32tVersion123) {
  evan::Version version(1, 2, 3);
  uint32_t result = version.to_uint32_t();
  
  uint32_t expected = VK_MAKE_VERSION(1, 2, 3);
  EXPECT_EQ(result, expected);
}

/**
 * @brief Test to_uint32_t conversion with zero version
 */
TEST_F(VersionTest, ToUint32tZeroVersion) {
  evan::Version version(0, 0, 0);
  uint32_t result = version.to_uint32_t();
  
  uint32_t expected = VK_MAKE_VERSION(0, 0, 0);
  EXPECT_EQ(result, expected);
  EXPECT_EQ(result, 0);
}

/**
 * @brief Test to_uint32_t with various version combinations
 */
TEST_F(VersionTest, ToUint32tVariousVersions) {
  // Test version 2.1.5
  {
    evan::Version version(2, 1, 5);
    uint32_t expected = VK_MAKE_VERSION(2, 1, 5);
    EXPECT_EQ(version.to_uint32_t(), expected);
  }
  
  // Test version 3.4.7
  {
    evan::Version version(3, 4, 7);
    uint32_t expected = VK_MAKE_VERSION(3, 4, 7);
    EXPECT_EQ(version.to_uint32_t(), expected);
  }
}

/**
 * @brief Test that different versions produce different uint32_t values
 */
TEST_F(VersionTest, DifferentVersionsProduceDifferentValues) {
  evan::Version version1(1, 0, 0);
  evan::Version version2(2, 0, 0);
  evan::Version version3(1, 1, 0);
  evan::Version version4(1, 0, 1);
  
  uint32_t val1 = version1.to_uint32_t();
  uint32_t val2 = version2.to_uint32_t();
  uint32_t val3 = version3.to_uint32_t();
  uint32_t val4 = version4.to_uint32_t();
  
  EXPECT_NE(val1, val2);
  EXPECT_NE(val1, val3);
  EXPECT_NE(val1, val4);
  EXPECT_NE(val2, val3);
  EXPECT_NE(val2, val4);
  EXPECT_NE(val3, val4);
}

/**
 * @brief Test multiple calls to to_uint32_t return consistent results
 */
TEST_F(VersionTest, ToUint32tConsistentResults) {
  evan::Version version(5, 6, 7);
  
  uint32_t result1 = version.to_uint32_t();
  uint32_t result2 = version.to_uint32_t();
  uint32_t result3 = version.to_uint32_t();
  
  EXPECT_EQ(result1, result2);
  EXPECT_EQ(result2, result3);
}

/**
 * @brief Test that version fields can be modified after construction
 */
TEST_F(VersionTest, VersionFieldsCanBeModified) {
  evan::Version version(1, 2, 3);
  
  version._major = 4;
  version._minor = 5;
  version._patch = 6;
  
  EXPECT_EQ(version._major, 4);
  EXPECT_EQ(version._minor, 5);
  EXPECT_EQ(version._patch, 6);
  
  uint32_t expected = VK_MAKE_VERSION(4, 5, 6);
  EXPECT_EQ(version.to_uint32_t(), expected);
}