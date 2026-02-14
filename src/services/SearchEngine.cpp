/**
 * @file SearchEngine.cpp
 * @brief 검색 엔진 통합 서비스 구현
 */

#include "SearchEngine.h"
#include <QUrl>
#include <QSettings>
#include <QRegularExpression>
#include <QDebug>

namespace webosbrowser {

// 검색 엔진 정의 (정적 초기화)
const QMap<QString, SearchEngineInfo>& SearchEngine::getSearchEngines() {
    static const QMap<QString, SearchEngineInfo> engines = {
        {
            "google",
            {
                "google",
                "Google",
                "https://www.google.com/search?q={query}"
            }
        },
        {
            "naver",
            {
                "naver",
                "Naver",
                "https://search.naver.com/search.naver?query={query}"
            }
        },
        {
            "bing",
            {
                "bing",
                "Bing",
                "https://www.bing.com/search?q={query}"
            }
        },
        {
            "duckduckgo",
            {
                "duckduckgo",
                "DuckDuckGo",
                "https://duckduckgo.com/?q={query}"
            }
        }
    };
    return engines;
}

QString SearchEngine::buildSearchUrl(const QString &query, const QString &engineId) {
    // 검색어가 비어있으면 빈 문자열 반환
    const QString trimmedQuery = query.trimmed();
    if (trimmedQuery.isEmpty()) {
        qWarning() << "[SearchEngine] 검색어가 비어있음";
        return QString();
    }

    // 검색 엔진 결정 (파라미터 → 설정 → 기본값)
    QString targetEngineId = engineId;
    if (targetEngineId.isEmpty()) {
        targetEngineId = getDefaultSearchEngine();
    }

    // 검색 엔진 정의 조회
    const auto& engines = getSearchEngines();
    if (!engines.contains(targetEngineId)) {
        qWarning() << "[SearchEngine] 알 수 없는 검색 엔진:" << targetEngineId;
        // 폴백: Google 사용
        targetEngineId = DEFAULT_ENGINE_ID;
    }

    const SearchEngineInfo& engine = engines[targetEngineId];

    // URL 인코딩 (특수문자, 공백, 다국어 처리)
    QString encodedQuery = QUrl::toPercentEncoding(trimmedQuery);

    // URL 템플릿에 쿼리 삽입
    QString searchUrl = engine.urlTemplate;
    searchUrl.replace("{query}", encodedQuery);

    qDebug() << "[SearchEngine] 검색 URL 생성:"
             << "query=" << trimmedQuery
             << "engine=" << targetEngineId
             << "url=" << searchUrl;

    return searchUrl;
}

QString SearchEngine::getDefaultSearchEngine() {
    QSettings settings("LG", "webOSBrowser");

    // localStorage에서 저장된 값 조회
    QString savedEngine = settings.value(STORAGE_KEY, DEFAULT_ENGINE_ID).toString();

    // 유효한 검색 엔진 ID인지 확인
    const auto& engines = getSearchEngines();
    if (engines.contains(savedEngine)) {
        qDebug() << "[SearchEngine] 저장된 검색 엔진 사용:" << savedEngine;
        return savedEngine;
    }

    // 기본값 반환
    qDebug() << "[SearchEngine] 기본 검색 엔진 사용:" << DEFAULT_ENGINE_ID;
    return DEFAULT_ENGINE_ID;
}

bool SearchEngine::setDefaultSearchEngine(const QString &engineId) {
    // 유효한 검색 엔진인지 확인
    const auto& engines = getSearchEngines();
    if (!engines.contains(engineId)) {
        qWarning() << "[SearchEngine] 유효하지 않은 검색 엔진:" << engineId;
        return false;
    }

    QSettings settings("LG", "webOSBrowser");
    settings.setValue(STORAGE_KEY, engineId);

    qDebug() << "[SearchEngine] 기본 검색 엔진 설정:" << engineId;
    return true;
}

QList<SearchEngineInfo> SearchEngine::getAllSearchEngines() {
    const auto& engines = getSearchEngines();
    return engines.values();
}

QString SearchEngine::getSearchEngineName(const QString &engineId) {
    const auto& engines = getSearchEngines();
    if (engines.contains(engineId)) {
        return engines[engineId].name;
    }
    return "Unknown";
}

bool SearchEngine::isSearchQuery(const QString &input) {
    const QString trimmed = input.trimmed();

    if (trimmed.isEmpty()) {
        return false;
    }

    // 1. 프로토콜 포함 시 URL로 판단
    if (trimmed.startsWith("http://") || trimmed.startsWith("https://") ||
        trimmed.startsWith("ftp://")) {
        return false;
    }

    // 2. 도메인 형식 (점 포함, TLD 필수) → URL로 판단
    // 예: google.com, www.example.com
    QRegularExpression domainPattern(
        R"(^(?:[a-zA-Z0-9](?:[a-zA-Z0-9-]{0,61}[a-zA-Z0-9])?\.)+[a-zA-Z]{2,}(?:/.*)?$)",
        QRegularExpression::CaseInsensitiveOption
    );
    if (domainPattern.match(trimmed).hasMatch()) {
        return false;
    }

    // 3. localhost 또는 IP 주소 → URL로 판단
    if (trimmed.startsWith("localhost") ||
        QRegularExpression(R"(^\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3})").match(trimmed).hasMatch()) {
        return false;
    }

    // 4. 위의 모든 URL 형식에 해당하지 않으면 검색어로 간주
    return true;
}

} // namespace webosbrowser
