/**
 * @file TabManager.h
 * @brief 탭 관리자 - 브라우저 탭 상태 관리
 */

#pragma once

#include <QObject>
#include <QString>
#include <QUrl>

namespace webosbrowser {

// 전방 선언 (Forward declaration)
class WebView;

/**
 * @class TabManager
 * @brief 브라우저 탭 상태 관리 서비스
 *
 * 간소화 버전: 현재는 단일 탭만 지원.
 * 향후 다중 탭 UI 추가 시 확장 예정 (QVector<WebView*>).
 */
class TabManager : public QObject {
    Q_OBJECT

public:
    explicit TabManager(QObject *parent = nullptr);
    ~TabManager() override;

    TabManager(const TabManager&) = delete;
    TabManager& operator=(const TabManager&) = delete;

    /**
     * @brief 현재 활성 탭 (WebView) 설정
     * @param webView WebView 포인터
     */
    void setCurrentTab(WebView *webView);

    /**
     * @brief 현재 활성 탭 반환
     * @return WebView 포인터 (nullptr 가능)
     */
    WebView* getCurrentTab() const;

    /**
     * @brief 현재 탭 개수 반환 (현재는 항상 1)
     * @return 탭 개수
     */
    int getTabCount() const;

    /**
     * @brief 현재 탭의 제목 반환
     * @return 탭 제목
     */
    QString getCurrentTabTitle() const;

    /**
     * @brief 현재 탭의 URL 반환
     * @return 탭 URL
     */
    QUrl getCurrentTabUrl() const;

signals:
    /**
     * @brief 탭 전환 시그널 (향후 다중 탭 지원 시)
     * @param index 탭 인덱스
     */
    void tabChanged(int index);

private:
    WebView *currentTab_;  ///< 현재 활성 탭 (단일 탭)
};

} // namespace webosbrowser
