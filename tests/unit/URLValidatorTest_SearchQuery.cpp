/**
 * @file URLValidatorTest_SearchQuery.cpp
 * @brief URLValidator 검색어 처리 테스트 (F-09 추가)
 */

#include <gtest/gtest.h>
#include "../../src/utils/URLValidator.h"

using namespace webosbrowser;

class URLValidatorSearchQueryTest : public ::testing::Test {
protected:
    // 테스트 전후 설정 없음
};

// isSearchQuery() 테스트
TEST_F(URLValidatorSearchQueryTest, IsSearchQuery_SingleWord) {
    EXPECT_TRUE(URLValidator::isSearchQuery("youtube"));
    EXPECT_TRUE(URLValidator::isSearchQuery("apple"));
}

TEST_F(URLValidatorSearchQueryTest, IsSearchQuery_MultipleWords) {
    EXPECT_TRUE(URLValidator::isSearchQuery("고양이 동영상"));
    EXPECT_TRUE(URLValidator::isSearchQuery("best restaurants"));
}

TEST_F(URLValidatorSearchQueryTest, IsSearchQuery_Korean) {
    EXPECT_TRUE(URLValidator::isSearchQuery("날씨 서울"));
    EXPECT_TRUE(URLValidator::isSearchQuery("네이버"));
}

TEST_F(URLValidatorSearchQueryTest, IsSearchQuery_SpecialCharacters) {
    EXPECT_TRUE(URLValidator::isSearchQuery("a+b=c"));
    EXPECT_TRUE(URLValidator::isSearchQuery("C++ tutorial"));
}

// URL 우선 처리 테스트
TEST_F(URLValidatorSearchQueryTest, IsSearchQuery_UrlWithProtocol_ReturnsFalse) {
    EXPECT_FALSE(URLValidator::isSearchQuery("https://google.com"));
    EXPECT_FALSE(URLValidator::isSearchQuery("http://naver.com"));
    EXPECT_FALSE(URLValidator::isSearchQuery("ftp://files.com"));
}

TEST_F(URLValidatorSearchQueryTest, IsSearchQuery_DomainFormat_ReturnsFalse) {
    EXPECT_FALSE(URLValidator::isSearchQuery("google.com"));
    EXPECT_FALSE(URLValidator::isSearchQuery("www.example.com"));
    EXPECT_FALSE(URLValidator::isSearchQuery("sub.domain.co.kr"));
}

TEST_F(URLValidatorSearchQueryTest, IsSearchQuery_Localhost_ReturnsFalse) {
    EXPECT_FALSE(URLValidator::isSearchQuery("localhost"));
    EXPECT_FALSE(URLValidator::isSearchQuery("localhost:8080"));
}

TEST_F(URLValidatorSearchQueryTest, IsSearchQuery_IPAddress_ReturnsFalse) {
    EXPECT_FALSE(URLValidator::isSearchQuery("192.168.1.1"));
    EXPECT_FALSE(URLValidator::isSearchQuery("10.0.0.1"));
}

TEST_F(URLValidatorSearchQueryTest, IsSearchQuery_Empty_ReturnsFalse) {
    EXPECT_FALSE(URLValidator::isSearchQuery(""));
    EXPECT_FALSE(URLValidator::isSearchQuery("   "));
}

// 회귀 테스트: 기존 F-03 기능 정상 동작 확인
TEST_F(URLValidatorSearchQueryTest, ExistingUrlValidation_StillWorksCorrectly) {
    // 도메인 형식 인식
    EXPECT_TRUE(URLValidator::isUrl("google.com"));
    EXPECT_TRUE(URLValidator::isUrl("www.naver.com"));

    // 프로토콜 포함 URL 인식
    EXPECT_TRUE(URLValidator::isUrl("https://example.com"));
    EXPECT_TRUE(URLValidator::isUrl("http://localhost"));

    // 검색어는 URL로 인식하지 않음
    EXPECT_FALSE(URLValidator::isUrl("youtube"));
    EXPECT_FALSE(URLValidator::isUrl("고양이 동영상"));
}
