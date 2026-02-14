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
class BookmarkPanel;
class BookmarkService;
class StorageService;

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

private slots:
    /**
     * @brief 북마크 버튼 클릭 핸들러
     */
    void onBookmarkButtonClicked();

    /**
     * @brief 북마크 선택 핸들러 (페이지 열기)
     * @param url 북마크 URL
     * @param title 북마크 제목
     */
    void onBookmarkSelected(const QString& url, const QString& title);

    /**
     * @brief URL 변경 핸들러
     * @param url 새 URL
     */
    void onUrlChanged(const QString& url);

private:
    // UI 컴포넌트
    QWidget *centralWidget_;         ///< 중앙 위젯
    QVBoxLayout *mainLayout_;        ///< 메인 레이아웃
    URLBar *urlBar_;                 ///< URL 입력 바
    NavigationBar *navigationBar_;   ///< 네비게이션 바
    LoadingIndicator *loadingIndicator_;  ///< 로딩 인디케이터
    WebView *webView_;               ///< 웹뷰 컴포넌트
    QLabel *statusLabel_;            ///< 상태 라벨 (하단)
    BookmarkPanel *bookmarkPanel_;   ///< 북마크 패널

    // 서비스
    TabManager *tabManager_;         ///< 탭 관리자
    StorageService *storageService_; ///< 스토리지 서비스
    BookmarkService *bookmarkService_; ///< 북마크 서비스

    // 현재 페이지 정보
    QString currentUrl_;             ///< 현재 URL
    QString currentTitle_;           ///< 현재 페이지 제목
};

} // namespace webosbrowser
