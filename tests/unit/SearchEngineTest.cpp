/**
 * @file SearchEngineTest.cpp
 * @brief SearchEngine 클래스 단위 테스트
 */

#include <gtest/gtest.h>
#include "../../src/services/SearchEngine.h"
#include <QSettings>

using namespace webosbrowser;

class SearchEngineTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 테스트 전 설정 초기화
        QSettings settings("LG", "webOSBrowser");
        settings.remove("defaultSearchEngine");
    }

    void TearDown() override {
        // 테스트 후 정리
        QSettings settings("LG", "webOSBrowser");
        settings.remove("defaultSearchEngine");
    }
};

// buildSearchUrl() 테스트
TEST_F(SearchEngineTest, BuildSearchUrl_Google) {
    QString url = SearchEngine::buildSearchUrl("youtube", "google");
    EXPECT_EQ(url, "https://www.google.com/search?q=youtube");
}

TEST_F(SearchEngineTest, BuildSearchUrl_Naver) {
    QString url = SearchEngine::buildSearchUrl("네이버", "naver");
    // URL 인코딩 확인 (한글)
    EXPECT_TRUE(url.startsWith("https://search.naver.com/search.naver?query="));
    EXPECT_TRUE(url.contains("%EB%84%A4%EC%9D%B4%EB%B2%84"));
}

TEST_F(SearchEngineTest, BuildSearchUrl_WithSpaces) {
    QString url = SearchEngine::buildSearchUrl("고양이 동영상", "google");
    // 공백이 %20으로 인코딩되는지 확인
    EXPECT_TRUE(url.contains("%EA%B3%A0%EC%96%91%EC%9D%B4%20%EB%8F%99%EC%98%81%EC%83%81"));
}

TEST_F(SearchEngineTest, BuildSearchUrl_EmptyQuery) {
    QString url = SearchEngine::buildSearchUrl("", "google");
    EXPECT_TRUE(url.isEmpty());
}

TEST_F(SearchEngineTest, BuildSearchUrl_UnknownEngine_FallbackToGoogle) {
    QString url = SearchEngine::buildSearchUrl("test", "unknown");
    // 폴백: Google 사용
    EXPECT_EQ(url, "https://www.google.com/search?q=test");
}

TEST_F(SearchEngineTest, BuildSearchUrl_SpecialCharacters) {
    QString url = SearchEngine::buildSearchUrl("a+b=c", "google");
    // 특수문자 인코딩 확인
    EXPECT_TRUE(url.contains("a%2Bb%3Dc"));
}

// getDefaultSearchEngine() 테스트
TEST_F(SearchEngineTest, GetDefaultSearchEngine_SavedValue) {
    QSettings settings("LG", "webOSBrowser");
    settings.setValue("defaultSearchEngine", "naver");

    QString engine = SearchEngine::getDefaultSearchEngine();
    EXPECT_EQ(engine, "naver");
}

TEST_F(SearchEngineTest, GetDefaultSearchEngine_DefaultValue) {
    // 저장된 값이 없으면 기본값(google) 반환
    QString engine = SearchEngine::getDefaultSearchEngine();
    EXPECT_EQ(engine, "google");
}

// setDefaultSearchEngine() 테스트
TEST_F(SearchEngineTest, SetDefaultSearchEngine_Success) {
    bool result = SearchEngine::setDefaultSearchEngine("bing");
    EXPECT_TRUE(result);

    QSettings settings("LG", "webOSBrowser");
    QString saved = settings.value("defaultSearchEngine").toString();
    EXPECT_EQ(saved, "bing");
}

TEST_F(SearchEngineTest, SetDefaultSearchEngine_InvalidEngine) {
    bool result = SearchEngine::setDefaultSearchEngine("invalid");
    EXPECT_FALSE(result);
}

// getAllSearchEngines() 테스트
TEST_F(SearchEngineTest, GetAllSearchEngines_ReturnsAllEngines) {
    QList<SearchEngineInfo> engines = SearchEngine::getAllSearchEngines();
    EXPECT_EQ(engines.size(), 4);  // Google, Naver, Bing, DuckDuckGo

    // 각 엔진이 포함되어 있는지 확인
    QStringList engineIds;
    for (const auto& engine : engines) {
        engineIds.append(engine.id);
    }
    EXPECT_TRUE(engineIds.contains("google"));
    EXPECT_TRUE(engineIds.contains("naver"));
    EXPECT_TRUE(engineIds.contains("bing"));
    EXPECT_TRUE(engineIds.contains("duckduckgo"));
}

// getSearchEngineName() 테스트
TEST_F(SearchEngineTest, GetSearchEngineName_ValidEngine) {
    QString name = SearchEngine::getSearchEngineName("google");
    EXPECT_EQ(name, "Google");
}

TEST_F(SearchEngineTest, GetSearchEngineName_InvalidEngine) {
    QString name = SearchEngine::getSearchEngineName("invalid");
    EXPECT_EQ(name, "Unknown");
}

// isSearchQuery() 테스트
TEST_F(SearchEngineTest, IsSearchQuery_SingleWord) {
    EXPECT_TRUE(SearchEngine::isSearchQuery("youtube"));
}

TEST_F(SearchEngineTest, IsSearchQuery_MultipleWords) {
    EXPECT_TRUE(SearchEngine::isSearchQuery("고양이 동영상"));
}

TEST_F(SearchEngineTest, IsSearchQuery_UrlWithProtocol) {
    EXPECT_FALSE(SearchEngine::isSearchQuery("https://google.com"));
}

TEST_F(SearchEngineTest, IsSearchQuery_DomainFormat) {
    EXPECT_FALSE(SearchEngine::isSearchQuery("google.com"));
}

TEST_F(SearchEngineTest, IsSearchQuery_Localhost) {
    EXPECT_FALSE(SearchEngine::isSearchQuery("localhost"));
}

TEST_F(SearchEngineTest, IsSearchQuery_IPAddress) {
    EXPECT_FALSE(SearchEngine::isSearchQuery("192.168.1.1"));
}

TEST_F(SearchEngineTest, IsSearchQuery_Empty) {
    EXPECT_FALSE(SearchEngine::isSearchQuery(""));
}
