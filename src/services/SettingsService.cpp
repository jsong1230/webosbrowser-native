/**
 * @file SettingsService.cpp
 * @brief 브라우저 설정 관리 서비스 구현
 */

#include "SettingsService.h"
#include "StorageService.h"
#include "SearchEngine.h"
#include "../utils/URLValidator.h"
#include <QJsonObject>
#include <QDebug>

namespace webosbrowser {

SettingsService::SettingsService(StorageService *storageService, QObject *parent)
    : QObject(parent)
    , storageService_(storageService)
    , searchEngine_(DEFAULT_SEARCH_ENGINE)
    , homepage_(DEFAULT_HOMEPAGE)
    , theme_(DEFAULT_THEME)
{
}

SettingsService::~SettingsService() = default;

bool SettingsService::loadSettings() {
    if (!storageService_) {
        emit settingsLoadFailed("StorageService가 초기화되지 않았습니다.");
        return false;
    }

    QJsonObject data = storageService_->getData(DataKind::Settings, STORAGE_ID);

    if (data.isEmpty()) {
        // 첫 실행 또는 설정 없음 → 기본값 사용
        qDebug() << "[SettingsService] 설정이 없습니다. 기본값을 사용합니다.";
        searchEngine_ = DEFAULT_SEARCH_ENGINE;
        homepage_ = DEFAULT_HOMEPAGE;
        theme_ = DEFAULT_THEME;

        // 기본값을 저장하여 다음 로드 시 성공하도록 함
        saveToStorage();
        emit settingsLoaded();
        return true;
    }

    // JSON 파싱
    searchEngine_ = data.value(KEY_SEARCH_ENGINE).toString(DEFAULT_SEARCH_ENGINE);
    homepage_ = data.value(KEY_HOMEPAGE).toString(DEFAULT_HOMEPAGE);
    theme_ = data.value(KEY_THEME).toString(DEFAULT_THEME);

    qDebug() << "[SettingsService] 설정 로드 완료:"
             << "검색 엔진=" << searchEngine_
             << "홈페이지=" << homepage_
             << "테마=" << theme_;

    emit settingsLoaded();
    return true;
}

QString SettingsService::searchEngine() const {
    return searchEngine_;
}

QString SettingsService::homepage() const {
    return homepage_;
}

QString SettingsService::theme() const {
    return theme_;
}

bool SettingsService::setSearchEngine(const QString &engineId) {
    // 유효성 검증 (SearchEngine::getAllSearchEngines()로 확인)
    QList<SearchEngineInfo> engines = SearchEngine::getAllSearchEngines();
    bool valid = false;
    for (const auto &engine : engines) {
        if (engine.id == engineId) {
            valid = true;
            break;
        }
    }

    if (!valid) {
        qWarning() << "[SettingsService] 유효하지 않은 검색 엔진:" << engineId;
        return false;
    }

    if (searchEngine_ == engineId) {
        // 변경 사항 없음
        return true;
    }

    searchEngine_ = engineId;

    // SearchEngine의 기본 엔진도 업데이트
    SearchEngine::setDefaultSearchEngine(engineId);

    bool success = saveToStorage();
    if (success) {
        qDebug() << "[SettingsService] 검색 엔진 변경:" << engineId;
        emit searchEngineChanged(engineId);
    } else {
        qWarning() << "[SettingsService] 검색 엔진 저장 실패";
    }

    return success;
}

bool SettingsService::setHomepage(const QString &url) {
    // URL 검증 (URLValidator 사용)
    // about: URL 허용
    if (!url.startsWith("about:") && !URLValidator::isUrl(url)) {
        qWarning() << "[SettingsService] 유효하지 않은 URL:" << url;
        return false;
    }

    // 위험한 프로토콜 차단
    if (url.contains("javascript:", Qt::CaseInsensitive) || url.startsWith("file://")) {
        qWarning() << "[SettingsService] 위험한 프로토콜이 포함된 URL:" << url;
        return false;
    }

    if (homepage_ == url) {
        // 변경 사항 없음
        return true;
    }

    homepage_ = url;
    bool success = saveToStorage();

    if (success) {
        qDebug() << "[SettingsService] 홈페이지 변경:" << url;
        emit homepageChanged(url);
    } else {
        qWarning() << "[SettingsService] 홈페이지 저장 실패";
    }

    return success;
}

bool SettingsService::setTheme(const QString &themeId) {
    // 유효성 검증
    if (themeId != "dark" && themeId != "light") {
        qWarning() << "[SettingsService] 유효하지 않은 테마:" << themeId;
        return false;
    }

    if (theme_ == themeId) {
        // 변경 사항 없음
        return true;
    }

    theme_ = themeId;
    bool success = saveToStorage();

    if (success) {
        qDebug() << "[SettingsService] 테마 변경:" << themeId;
        emit themeChanged(themeId);
    } else {
        qWarning() << "[SettingsService] 테마 저장 실패";
    }

    return success;
}

bool SettingsService::saveToStorage() {
    if (!storageService_) {
        qWarning() << "[SettingsService] StorageService가 초기화되지 않았습니다.";
        return false;
    }

    QJsonObject data;
    data[KEY_SEARCH_ENGINE] = searchEngine_;
    data[KEY_HOMEPAGE] = homepage_;
    data[KEY_THEME] = theme_;

    bool success = storageService_->putData(DataKind::Settings, STORAGE_ID, data);

    if (success) {
        qDebug() << "[SettingsService] 설정 저장 완료";
    } else {
        qWarning() << "[SettingsService] 설정 저장 실패";
    }

    return success;
}

} // namespace webosbrowser
