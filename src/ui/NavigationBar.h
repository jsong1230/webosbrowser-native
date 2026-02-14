/**
 * @file NavigationBar.h
 * @brief 네비게이션 바 컴포넌트 - 브라우저 네비게이션 버튼 UI
 */

#pragma once

#include <QWidget>
#include <QPushButton>
#include <QHBoxLayout>

namespace webosbrowser {

// Forward declaration
class WebView;

/**
 * @class NavigationBar
 * @brief 뒤로/앞으로/새로고침/홈 버튼을 제공하는 네비게이션 바
 *
 * WebView와 시그널/슬롯으로 연동하여 버튼 상태를 자동 동기화합니다.
 * Qt Focus Policy로 리모컨 방향키 네비게이션을 지원합니다.
 */
class NavigationBar : public QWidget {
    Q_OBJECT

public:
    /**
     * @brief 생성자
     * @param parent 부모 위젯
     */
    explicit NavigationBar(QWidget *parent = nullptr);

    /**
     * @brief 소멸자
     */
    ~NavigationBar() override;

    // 복사 생성자 및 대입 연산자 삭제
    NavigationBar(const NavigationBar&) = delete;
    NavigationBar& operator=(const NavigationBar&) = delete;

    /**
     * @brief WebView 인스턴스 설정 (시그널/슬롯 연결)
     * @param webView WebView 포인터 (nullptr 가능, 연결 해제 시)
     */
    void setWebView(WebView *webView);

public slots:
    /**
     * @brief 버튼 상태 업데이트 (WebView 히스토리 상태 기반)
     */
    void updateButtonStates();

private slots:
    /**
     * @brief 뒤로 버튼 클릭 핸들러
     */
    void onBackClicked();

    /**
     * @brief 앞으로 버튼 클릭 핸들러
     */
    void onForwardClicked();

    /**
     * @brief 새로고침 버튼 클릭 핸들러
     */
    void onReloadClicked();

    /**
     * @brief 홈 버튼 클릭 핸들러
     */
    void onHomeClicked();

    /**
     * @brief 북마크 버튼 클릭 핸들러
     */
    void onBookmarkClicked();

signals:
    /**
     * @brief 북마크 버튼 클릭 시그널
     */
    void bookmarkButtonClicked();

private:
    /**
     * @brief UI 초기화 (버튼 생성, 레이아웃 설정)
     */
    void setupUI();

    /**
     * @brief 시그널/슬롯 연결
     */
    void setupConnections();

    /**
     * @brief 스타일시트 적용 (QSS)
     */
    void applyStyles();

    /**
     * @brief 포커스 순서 설정 (리모컨 네비게이션)
     */
    void setupFocusOrder();

private:
    // UI 컴포넌트
    QHBoxLayout *layout_;           ///< 가로 레이아웃
    QPushButton *backButton_;       ///< 뒤로 버튼
    QPushButton *forwardButton_;    ///< 앞으로 버튼
    QPushButton *reloadButton_;     ///< 새로고침 버튼
    QPushButton *homeButton_;       ///< 홈 버튼
    QPushButton *bookmarkButton_;   ///< 북마크 버튼

    // 데이터
    WebView *webView_;              ///< WebView 인스턴스 (약한 참조)
    const QString DEFAULT_HOME_URL = "https://www.google.com";  ///< 기본 홈페이지 URL
};

} // namespace webosbrowser
