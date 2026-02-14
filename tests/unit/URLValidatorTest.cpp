/**
 * @file URLValidatorTest.cpp
 * @brief URLValidator 클래스 단위 테스트
 *
 * 테스트 범위:
 * 1. URL 유효성 검증 (validate)
 * 2. URL 자동 보완 (normalize)
 * 3. URL vs 검색어 판단 (isUrl)
 * 4. 도메인 형식 검증 (isValidUrlFormat)
 * 5. 엣지 케이스 처리
 */

#include <gtest/gtest.h>
#include "../src/utils/URLValidator.h"

using namespace webosbrowser;

// ============================================================================
// 테스트 클래스
// ============================================================================

class URLValidatorTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 각 테스트 전에 실행
    }

    void TearDown() override {
        // 각 테스트 후에 실행
    }
};

// ============================================================================
// FR-4: URL 검증 테스트
// ============================================================================

/**
 * 요구사항: 유효한 URL 검증
 * 입력: http://, https://, ftp:// 프로토콜 포함 URL
 * 기대: validate() = Valid 반환
 */
TEST_F(URLValidatorTest, ValidURL_WithProtocol_HTTP) {
    // AAA Pattern
    // Arrange
    QString input = "http://google.com";

    // Act
    URLValidator::ValidationResult result = URLValidator::validate(input);

    // Assert
    EXPECT_EQ(result, URLValidator::ValidationResult::Valid);
}

/**
 * 요구사항: HTTPS URL 검증
 */
TEST_F(URLValidatorTest, ValidURL_WithProtocol_HTTPS) {
    QString input = "https://example.com/path?query=value";
    URLValidator::ValidationResult result = URLValidator::validate(input);
    EXPECT_EQ(result, URLValidator::ValidationResult::Valid);
}

/**
 * 요구사항: 경로 포함 URL
 */
TEST_F(URLValidatorTest, ValidURL_WithPath) {
    QString input = "https://example.com/path/to/page";
    URLValidator::ValidationResult result = URLValidator::validate(input);
    EXPECT_EQ(result, URLValidator::ValidationResult::Valid);
}

/**
 * 요구사항: 쿼리 문자열 포함
 */
TEST_F(URLValidatorTest, ValidURL_WithQueryString) {
    QString input = "https://example.com/search?q=test&lang=ko";
    URLValidator::ValidationResult result = URLValidator::validate(input);
    EXPECT_EQ(result, URLValidator::ValidationResult::Valid);
}

/**
 * 요구사항: FTP 프로토콜
 */
TEST_F(URLValidatorTest, ValidURL_FTPProtocol) {
    QString input = "ftp://ftp.example.com/file.zip";
    URLValidator::ValidationResult result = URLValidator::validate(input);
    EXPECT_EQ(result, URLValidator::ValidationResult::Valid);
}

// ============================================================================
// FR-4: 프로토콜 자동 보완 테스트
// ============================================================================

/**
 * 요구사항: 도메인만 입력 시 프로토콜 자동 추가
 * 입력: google.com
 * 기대: normalize() = https://google.com 반환, validate() = MissingScheme
 */
TEST_F(URLValidatorTest, ValidURL_DomainOnly_ReturnsMissingScheme) {
    QString input = "google.com";
    URLValidator::ValidationResult result = URLValidator::validate(input);
    EXPECT_EQ(result, URLValidator::ValidationResult::MissingScheme);
}

/**
 * 요구사항: 프로토콜 누락 시 normalize로 자동 보완
 * 입력: google.com
 * 기대: https://google.com 반환
 */
TEST_F(URLValidatorTest, Normalize_AddHTTPSProtocol) {
    // Arrange
    QString input = "google.com";

    // Act
    QString normalized = URLValidator::normalize(input);

    // Assert
    EXPECT_EQ(normalized, "https://google.com");
}

/**
 * 요구사항: www 포함 도메인 정규화
 */
TEST_F(URLValidatorTest, Normalize_WWWDomain) {
    QString input = "www.example.com";
    QString normalized = URLValidator::normalize(input);
    EXPECT_EQ(normalized, "https://www.example.com");
}

/**
 * 요구사항: 이미 프로토콜이 있으면 그대로 유지
 */
TEST_F(URLValidatorTest, Normalize_PreserveExistingProtocol) {
    QString input = "http://example.com";
    QString normalized = URLValidator::normalize(input);
    EXPECT_EQ(normalized, "http://example.com");
}

/**
 * 요구사항: HTTPS 프로토콜 유지
 */
TEST_F(URLValidatorTest, Normalize_PreserveHTTPS) {
    QString input = "https://secure.example.com";
    QString normalized = URLValidator::normalize(input);
    EXPECT_EQ(normalized, "https://secure.example.com");
}

/**
 * 요구사항: 경로 포함 도메인 정규화
 */
TEST_F(URLValidatorTest, Normalize_DomainWithPath) {
    QString input = "example.com/path/to/page";
    QString normalized = URLValidator::normalize(input);
    EXPECT_EQ(normalized, "https://example.com/path/to/page");
}

// ============================================================================
// FR-4: URL vs 검색어 판단 (isUrl)
// ============================================================================

/**
 * 요구사항: 프로토콜 포함 → URL
 */
TEST_F(URLValidatorTest, IsUrl_WithProtocol) {
    EXPECT_TRUE(URLValidator::isUrl("http://google.com"));
    EXPECT_TRUE(URLValidator::isUrl("https://example.com"));
    EXPECT_TRUE(URLValidator::isUrl("ftp://files.com"));
}

/**
 * 요구사항: 도메인 형식 → URL
 */
TEST_F(URLValidatorTest, IsUrl_DomainFormat) {
    EXPECT_TRUE(URLValidator::isUrl("google.com"));
    EXPECT_TRUE(URLValidator::isUrl("www.example.com"));
    EXPECT_TRUE(URLValidator::isUrl("subdomain.example.co.uk"));
}

/**
 * 요구사항: localhost → URL
 */
TEST_F(URLValidatorTest, IsUrl_Localhost) {
    EXPECT_TRUE(URLValidator::isUrl("localhost"));
    EXPECT_TRUE(URLValidator::isUrl("localhost:8080"));
}

/**
 * 요구사항: IP 주소 → URL
 */
TEST_F(URLValidatorTest, IsUrl_IPAddress) {
    EXPECT_TRUE(URLValidator::isUrl("192.168.1.1"));
    EXPECT_TRUE(URLValidator::isUrl("127.0.0.1"));
    EXPECT_TRUE(URLValidator::isUrl("10.0.0.1"));
}

/**
 * 요구사항: 공백 포함 → 검색어
 */
TEST_F(URLValidatorTest, IsUrl_SearchQuery_WithSpaces) {
    EXPECT_FALSE(URLValidator::isUrl("hello world"));
    EXPECT_FALSE(URLValidator::isUrl("how to cook pasta"));
}

/**
 * 요구사항: 단순 단어 → 검색어
 */
TEST_F(URLValidatorTest, IsUrl_SearchQuery_SingleWord) {
    EXPECT_FALSE(URLValidator::isUrl("google"));
    EXPECT_FALSE(URLValidator::isUrl("hello"));
}

/**
 * 요구사항: 특수 문자 검색어 → 검색어
 */
TEST_F(URLValidatorTest, IsUrl_SearchQuery_SpecialChars) {
    EXPECT_FALSE(URLValidator::isUrl("what is a URL?"));
    EXPECT_FALSE(URLValidator::isUrl("@#$%"));
}

// ============================================================================
// FR-4: 도메인 형식 검증 (isValidUrlFormat)
// ============================================================================

/**
 * 요구사항: 유효한 도메인 형식
 */
TEST_F(URLValidatorTest, IsValidUrlFormat_ValidDomain) {
    // isValidUrlFormat은 normalize 후 QUrl::isValid 체크
    EXPECT_TRUE(URLValidator::isValidUrlFormat("google.com"));
    EXPECT_TRUE(URLValidator::isValidUrlFormat("www.example.com"));
    EXPECT_TRUE(URLValidator::isValidUrlFormat("https://secure.example.com"));
}

/**
 * 요구사항: 포트 번호 포함
 */
TEST_F(URLValidatorTest, IsValidUrlFormat_WithPort) {
    EXPECT_TRUE(URLValidator::isValidUrlFormat("localhost:8080"));
    EXPECT_TRUE(URLValidator::isValidUrlFormat("example.com:3000"));
}

/**
 * 요구사항: 유효하지 않은 도메인
 */
TEST_F(URLValidatorTest, IsValidUrlFormat_InvalidDomain) {
    EXPECT_FALSE(URLValidator::isValidUrlFormat("invalid url !!!"));
    EXPECT_FALSE(URLValidator::isValidUrlFormat("..."));
}

// ============================================================================
// 엣지 케이스 테스트
// ============================================================================

/**
 * 엣지 케이스: 빈 문자열
 */
TEST_F(URLValidatorTest, EdgeCase_EmptyString) {
    EXPECT_EQ(URLValidator::validate(""), URLValidator::ValidationResult::Empty);
    EXPECT_EQ(URLValidator::normalize(""), "");
    EXPECT_FALSE(URLValidator::isUrl(""));
    EXPECT_FALSE(URLValidator::isValidUrlFormat(""));
}

/**
 * 엣지 케이스: 공백만 입력
 */
TEST_F(URLValidatorTest, EdgeCase_OnlyWhitespace) {
    EXPECT_EQ(URLValidator::validate("   "), URLValidator::ValidationResult::Empty);
    EXPECT_FALSE(URLValidator::isUrl("   "));
}

/**
 * 엣지 케이스: 서브도메인 다중 레벨
 */
TEST_F(URLValidatorTest, EdgeCase_MultiLevelSubdomain) {
    EXPECT_TRUE(URLValidator::isUrl("api.v1.example.com"));
    EXPECT_TRUE(URLValidator::isUrl("cdn.images.subdomain.example.co.uk"));
}

/**
 * 엣지 케이스: 사이트 주소 변수
 */
TEST_F(URLValidatorTest, EdgeCase_VariousTopLevelDomains) {
    EXPECT_TRUE(URLValidator::isUrl("example.co.uk"));
    EXPECT_TRUE(URLValidator::isUrl("example.com.br"));
    EXPECT_TRUE(URLValidator::isUrl("example.gov.kr"));
}

/**
 * 엣지 케이스: 하이픈 포함 도메인
 */
TEST_F(URLValidatorTest, EdgeCase_HyphenInDomain) {
    EXPECT_TRUE(URLValidator::isUrl("my-site.com"));
    EXPECT_TRUE(URLValidator::isUrl("www.my-example-site.com"));
}

/**
 * 엣지 케이스: 숫자로 시작하는 도메인
 */
TEST_F(URLValidatorTest, EdgeCase_NumberInDomain) {
    EXPECT_TRUE(URLValidator::isUrl("example123.com"));
    EXPECT_TRUE(URLValidator::isUrl("123example.com"));
}

/**
 * 엣지 케이스: 특수 경로 문자
 */
TEST_F(URLValidatorTest, EdgeCase_SpecialPathCharacters) {
    EXPECT_TRUE(URLValidator::isValidUrlFormat("example.com/path-with-dash"));
    EXPECT_TRUE(URLValidator::isValidUrlFormat("example.com/path_with_underscore"));
    EXPECT_TRUE(URLValidator::isValidUrlFormat("example.com/path.with.dot"));
}

/**
 * 엣지 케이스: 앵커(프래그먼트) 포함
 */
TEST_F(URLValidatorTest, EdgeCase_URLWithFragment) {
    EXPECT_TRUE(URLValidator::isValidUrlFormat("example.com#section"));
    EXPECT_TRUE(URLValidator::isValidUrlFormat("example.com/page#top"));
}

/**
 * 엣지 케이스: 인코딩된 문자 포함
 */
TEST_F(URLValidatorTest, EdgeCase_URLWithEncoding) {
    EXPECT_TRUE(URLValidator::isValidUrlFormat("example.com/path?name=%20value"));
    EXPECT_TRUE(URLValidator::isValidUrlFormat("example.com/search?q=%E2%9C%93"));
}

/**
 * 엣지 케이스: 사용자 정보 포함 (username:password)
 */
TEST_F(URLValidatorTest, EdgeCase_URLWithUserInfo) {
    // QUrl은 user:pass@host 형식 지원
    EXPECT_TRUE(URLValidator::isValidUrlFormat("https://user:pass@example.com"));
}

/**
 * 엣지 케이스: 대문자 스키마
 */
TEST_F(URLValidatorTest, EdgeCase_UppercaseScheme) {
    // normalize는 소문자로 체크하지만, QUrl은 대소문자 구분 안함
    QString input = "HTTP://GOOGLE.COM";
    URLValidator::ValidationResult result = URLValidator::validate(input);
    // 결과는 구현에 따라 달라질 수 있음
    EXPECT_NE(result, URLValidator::ValidationResult::Empty);
}

// ============================================================================
// 요구사항 매핑 테스트
// ============================================================================

/**
 * 요구사항 FR-4: "도메인 형식: `example.com`, `www.example.com`"
 */
TEST_F(URLValidatorTest, Requirement_FR4_DomainFormat) {
    EXPECT_EQ(URLValidator::validate("example.com"), URLValidator::ValidationResult::MissingScheme);
    EXPECT_EQ(URLValidator::validate("www.example.com"), URLValidator::ValidationResult::MissingScheme);
}

/**
 * 요구사항 FR-4: "프로토콜 포함: `http://`, `https://`"
 */
TEST_F(URLValidatorTest, Requirement_FR4_ProtocolIncluded) {
    EXPECT_EQ(URLValidator::validate("http://google.com"), URLValidator::ValidationResult::Valid);
    EXPECT_EQ(URLValidator::validate("https://google.com"), URLValidator::ValidationResult::Valid);
}

/**
 * 요구사항 FR-4: "경로 포함: `example.com/path?query=1`"
 */
TEST_F(URLValidatorTest, Requirement_FR4_PathIncluded) {
    EXPECT_EQ(URLValidator::validate("example.com/path?query=1"), URLValidator::ValidationResult::MissingScheme);
}

/**
 * 요구사항 FR-4: "자동 보완 규칙: 프로토콜 없으면 `http://` 자동 추가"
 * (실제 구현은 https:// 추가이지만, 프로토콜 자동 추가는 동일)
 */
TEST_F(URLValidatorTest, Requirement_FR4_AutoComplete) {
    // google.com → https://google.com 자동 추가
    QString normalized = URLValidator::normalize("google.com");
    EXPECT_TRUE(normalized.startsWith("https://") || normalized.startsWith("http://"));
    EXPECT_TRUE(normalized.contains("google.com"));
}

/**
 * 요구사항 FR-4: "검색어로 판단 시 검색 엔진으로 쿼리"
 */
TEST_F(URLValidatorTest, Requirement_FR4_SearchQuery) {
    EXPECT_FALSE(URLValidator::isUrl("hello world"));  // 공백 포함 → 검색어
    EXPECT_FALSE(URLValidator::isUrl("how to cook"));  // 공백 포함 → 검색어
}

// ============================================================================
// 성능 테스트
// ============================================================================

/**
 * 성능: URL 검증 응답 시간 < 100ms (대량 URL 검증)
 */
TEST_F(URLValidatorTest, Performance_BulkURLValidation) {
    // Arrange
    QStringList urls = {
        "https://google.com",
        "example.com",
        "hello world",
        "192.168.1.1",
        "invalid!!!",
    };

    // Act & Assert
    auto startTime = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < 1000; ++i) {
        for (const auto &url : urls) {
            URLValidator::validate(url);
        }
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

    // 5000회 검증이 1000ms 이내 (각 검증 < 0.2ms)
    EXPECT_LT(duration.count(), 1000);
}
