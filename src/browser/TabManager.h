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
 * 다중 탭 지원 (최대 5개).
 * F-13 (리모컨 단축키) 기능을 위해 리팩토링됨.
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
     * @brief 탭 개수 반환
     * @return 탭 개수 (0~5)
     */
    int getTabCount() const;

    /**
     * @brief 현재 활성 탭 인덱스 반환
     * @return 탭 인덱스 (0~4), 탭이 없으면 -1
     */
    int getCurrentTabIndex() const;

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

    /**
     * @brief 새 탭 생성 (최대 5개 제한)
     * @param url 초기 URL (기본값: Google 홈페이지)
     * @return 생성 성공 여부
     */
    bool createTab(const QString& url = "https://www.google.com");

    /**
     * @brief 탭 전환 (인덱스 지정)
     * @param index 탭 인덱스 (0~4)
     * @return 전환 성공 여부
     */
    bool switchToTab(int index);

    /**
     * @brief 이전 탭으로 전환 (순환)
     * @return 전환 성공 여부
     */
    bool previousTab();

    /**
     * @brief 다음 탭으로 전환 (순환)
     * @return 전환 성공 여부
     */
    bool nextTab();

    /**
     * @brief 탭 닫기
     * @param index 탭 인덱스
     * @return 닫기 성공 여부
     */
    bool closeTab(int index);

signals:
    /**
     * @brief 탭 전환 시그널
     * @param index 새 탭 인덱스
     * @param url 새 탭 URL
     * @param title 새 탭 제목
     */
    void tabSwitched(int index, const QString& url, const QString& title);

    /**
     * @brief 새 탭 생성 시그널
     * @param index 새 탭 인덱스
     */
    void tabCreated(int index);

    /**
     * @brief 탭 닫힘 시그널
     * @param index 닫힌 탭 인덱스
     */
    void tabClosed(int index);

    /**
     * @brief 탭 전환 시그널 (하위 호환성)
     * @param index 탭 인덱스
     */
    void tabChanged(int index);

private:
    /**
     * @brief 탭 데이터 구조체
     */
    struct TabData {
        QString url;              ///< 탭 URL
        QString title;            ///< 탭 제목
        // QByteArray historyState;  ///< 웹 히스토리 상태 (향후 구현)
    };

    QVector<TabData> tabs_;       ///< 탭 목록 (최대 5개)
    int currentTabIndex_;         ///< 현재 활성 탭 인덱스 (-1: 탭 없음)
    WebView *webView_;            ///< 공유 WebView 인스턴스

    static constexpr int MAX_TABS = 5;  ///< 최대 탭 개수

    /**
     * @brief 탭 데이터 저장 (전환 전)
     */
    void saveCurrentTabState();

    /**
     * @brief 탭 데이터 복원 (전환 후)
     * @param index 탭 인덱스
     */
    void restoreTabState(int index);
};

} // namespace webosbrowser
