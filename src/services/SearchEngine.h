/**
 * @file SearchEngine.h
 * @brief 검색 엔진 통합 서비스
 */

#pragma once

#include <QString>
#include <QMap>

namespace webosbrowser {

/**
 * @struct SearchEngineInfo
 * @brief 검색 엔진 정보
 */
struct SearchEngineInfo {
    QString id;             ///< 검색 엔진 ID
    QString name;           ///< 표시 이름
    QString urlTemplate;    ///< URL 템플릿 (예: "https://www.google.com/search?q={query}")
};

/**
 * @class SearchEngine
 * @brief 검색 엔진 관리 서비스 (정적 메서드)
 *
 * URL이 아닌 검색어 입력 시 선택된 검색 엔진의 검색 URL을 생성합니다.
 * 지원 검색 엔진: Google, Naver, Bing, DuckDuckGo
 *
 * @example
 * QString searchUrl = SearchEngine::buildSearchUrl("youtube", "google");
 * // → "https://www.google.com/search?q=youtube"
 */
class SearchEngine {
public:
    /**
     * @brief 검색 URL 생성
     * @param query 검색어
     * @param engineId 검색 엔진 ID (기본: 현재 설정된 엔진)
     * @return 검색 URL (실패 시 빈 문자열)
     *
     * @example
     * buildSearchUrl("youtube")
     * // → "https://www.google.com/search?q=youtube"
     *
     * buildSearchUrl("고양이 동영상", "naver")
     * // → "https://search.naver.com/search.naver?query=%EA%B3%A0%EC%96%91%EC%9D%B4%20%EB%8F%99%EC%98%81%EC%83%81"
     */
    static QString buildSearchUrl(const QString &query, const QString &engineId = QString());

    /**
     * @brief 현재 설정된 기본 검색 엔진 조회
     * @return 검색 엔진 ID (기본: "google")
     *
     * @example
     * getDefaultSearchEngine() // → "google"
     */
    static QString getDefaultSearchEngine();

    /**
     * @brief 기본 검색 엔진 설정
     * @param engineId 검색 엔진 ID
     * @return 설정 성공 여부
     *
     * @example
     * setDefaultSearchEngine("naver") // → true
     */
    static bool setDefaultSearchEngine(const QString &engineId);

    /**
     * @brief 모든 검색 엔진 목록 조회 (F-11 설정 UI에서 사용)
     * @return 검색 엔진 목록
     *
     * @example
     * getAllSearchEngines()
     * // → [{ id: "google", name: "Google", ... }, ...]
     */
    static QList<SearchEngineInfo> getAllSearchEngines();

    /**
     * @brief 검색 엔진 이름 조회
     * @param engineId 검색 엔진 ID
     * @return 검색 엔진 이름 (없으면 "Unknown")
     *
     * @example
     * getSearchEngineName("google") // → "Google"
     */
    static QString getSearchEngineName(const QString &engineId);

    /**
     * @brief 검색어인지 판단
     * @param input 사용자 입력 문자열
     * @return true면 검색어, false면 URL로 간주
     */
    static bool isSearchQuery(const QString &input);

private:
    // 지원하는 검색 엔진 목록 (정적 초기화)
    static const QMap<QString, SearchEngineInfo>& getSearchEngines();

    // 기본 검색 엔진
    static constexpr const char* DEFAULT_ENGINE_ID = "google";

    // localStorage 키 (Qt Settings 사용)
    static constexpr const char* STORAGE_KEY = "defaultSearchEngine";
};

} // namespace webosbrowser
