/**
 * @file SettingsService.h
 * @brief 브라우저 설정 관리 서비스
 */

#pragma once

#include <QObject>
#include <QString>

namespace webosbrowser {

class StorageService;

/**
 * @class SettingsService
 * @brief 브라우저 설정 관리 서비스
 *
 * 검색 엔진, 홈페이지, 테마 등의 설정을 LS2 API로 저장/로드합니다.
 * 설정 변경 시 시그널을 발생시켜 관련 컴포넌트에 통지합니다.
 */
class SettingsService : public QObject {
    Q_OBJECT

public:
    explicit SettingsService(StorageService *storageService, QObject *parent = nullptr);
    ~SettingsService() override;

    SettingsService(const SettingsService&) = delete;
    SettingsService& operator=(const SettingsService&) = delete;

    /**
     * @brief 설정 로드 (앱 시작 시 호출)
     * @return 로드 성공 여부
     */
    bool loadSettings();

    // Getters
    QString searchEngine() const;
    QString homepage() const;
    QString theme() const;

    // Setters (LS2 저장 포함)
    bool setSearchEngine(const QString &engineId);
    bool setHomepage(const QString &url);
    bool setTheme(const QString &themeId);

signals:
    /**
     * @brief 검색 엔진 변경 시그널
     * @param engineId 새 검색 엔진 ID
     */
    void searchEngineChanged(const QString &engineId);

    /**
     * @brief 홈페이지 변경 시그널
     * @param url 새 홈페이지 URL
     */
    void homepageChanged(const QString &url);

    /**
     * @brief 테마 변경 시그널
     * @param themeId 새 테마 ID ("dark" 또는 "light")
     */
    void themeChanged(const QString &themeId);

    /**
     * @brief 설정 로드 완료 시그널
     */
    void settingsLoaded();

    /**
     * @brief 설정 로드 실패 시그널
     * @param errorMessage 에러 메시지
     */
    void settingsLoadFailed(const QString &errorMessage);

private:
    /**
     * @brief 설정을 LS2 API에 저장
     * @return 저장 성공 여부
     */
    bool saveToStorage();

    StorageService *storageService_;

    // 설정 캐시 (메모리)
    QString searchEngine_;
    QString homepage_;
    QString theme_;

    // 기본값
    static constexpr const char* DEFAULT_SEARCH_ENGINE = "google";
    static constexpr const char* DEFAULT_HOMEPAGE = "https://www.google.com";
    static constexpr const char* DEFAULT_THEME = "dark";

    // LS2 저장 키
    static constexpr const char* STORAGE_ID = "browser_settings";
    static constexpr const char* KEY_SEARCH_ENGINE = "searchEngine";
    static constexpr const char* KEY_HOMEPAGE = "homepage";
    static constexpr const char* KEY_THEME = "theme";
};

} // namespace webosbrowser
