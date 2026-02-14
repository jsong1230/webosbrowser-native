/**
 * @file BrowserWindow.h
 * @brief 브라우저 메인 윈도우 헤더
 */

#pragma once

#include <QMainWindow>
#include <QLabel>
#include <QVBoxLayout>

namespace webosbrowser {

// Forward declarations
class WebView;
class URLBar;
class NavigationBar;
class LoadingIndicator;
class TabManager;

/**
 * @class BrowserWindow
 * @brief 브라우저 메인 윈도우 클래스
 *
 * Qt 기반 메인 윈도우. URL 바, 네비게이션 바, WebView 등을
 * 포함하는 최상위 UI 컨테이너.
 */
class BrowserWindow : public QMainWindow {
    Q_OBJECT

public:
    /**
     * @brief 생성자
     * @param parent 부모 위젯 (기본값: nullptr)
     */
    explicit BrowserWindow(QWidget *parent = nullptr);

    /**
     * @brief 소멸자
     */
    ~BrowserWindow() override;

    // 복사 생성자 및 대입 연산자 삭제 (QObject 규칙)
    BrowserWindow(const BrowserWindow&) = delete;
    BrowserWindow& operator=(const BrowserWindow&) = delete;

private:
    /**
     * @brief UI 초기화
     */
    void setupUI();

    /**
     * @brief 시그널/슬롯 연결
     */
    void setupConnections();

private:
    // UI 컴포넌트
    QWidget *centralWidget_;         ///< 중앙 위젯
    QVBoxLayout *mainLayout_;        ///< 메인 레이아웃
    URLBar *urlBar_;                 ///< URL 입력 바
    NavigationBar *navigationBar_;   ///< 네비게이션 바
    LoadingIndicator *loadingIndicator_;  ///< 로딩 인디케이터
    WebView *webView_;               ///< 웹뷰 컴포넌트
    QLabel *statusLabel_;            ///< 상태 라벨 (하단)

    // 서비스
    TabManager *tabManager_;         ///< 탭 관리자
};

} // namespace webosbrowser
