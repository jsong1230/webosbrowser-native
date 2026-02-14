/**
 * @file BrowserWindow.h
 * @brief 브라우저 메인 윈도우 헤더
 */

#pragma once

#include <QMainWindow>
#include <QLabel>
#include <QVBoxLayout>
#include <QStackedLayout>
#include <QSet>
#include <QTimer>
#include <QMap>
#include <QKeyEvent>

namespace webosbrowser {

// Forward declarations
class WebView;
class URLBar;
class NavigationBar;
class LoadingIndicator;
class TabManager;
class StorageService;
class BookmarkPanel;
class BookmarkService;
class HistoryService;
class HistoryPanel;
class ErrorPage;
class DownloadManager;
class DownloadPanel;

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

    /**
     * @brief 다운로드 패널 표시
     */
    void showDownloadPanel();

    /**
     * @brief 다운로드 패널 숨김
     */
    void hideDownloadPanel();

    /**
     * @brief 다운로드 패널 토글
     */
    void toggleDownloadPanel();

protected:
    /**
     * @brief 리모컨 키 이벤트 처리 (오버라이드)
     * @param event 키 이벤트
     */
    void keyPressEvent(QKeyEvent *event) override;

private:
    /**
     * @brief UI 초기화
     */
    void setupUI();

    /**
     * @brief 시그널/슬롯 연결
     */
    void setupConnections();

    /**
     * @brief 디바운스 체크 (0.5초 이내 중복 입력 방지)
     * @param keyCode 키 코드
     * @return true이면 중복 입력 (무시), false이면 처리 허용
     */
    bool shouldDebounce(int keyCode);

    /**
     * @brief 채널 버튼 처리 (탭 전환)
     * @param keyCode Qt::Key_ChannelUp 또는 Qt::Key_ChannelDown
     */
    void handleChannelButton(int keyCode);

    /**
     * @brief 컬러 버튼 처리 (패널 열기, 새 탭)
     * @param keyCode Qt::Key_Red, Green, Yellow, Blue
     */
    void handleColorButton(int keyCode);

    /**
     * @brief 숫자 버튼 처리 (직접 탭 선택)
     * @param keyCode Qt::Key_1 ~ Qt::Key_5
     */
    void handleNumberButton(int keyCode);

    /**
     * @brief 설정 버튼 처리
     * @param keyCode Qt::Key_Menu
     */
    void handleMenuButton(int keyCode);

    /**
     * @brief 재생 버튼 처리 (M3 이후)
     * @param keyCode Play, Pause, FastForward, Rewind
     */
    void handlePlaybackButton(int keyCode);

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

    /**
     * @brief 페이지 로딩 완료 시 히스토리 자동 기록
     * @param success 로딩 성공 여부
     */
    void onPageLoadFinished(bool success);

    /**
     * @brief 히스토리 버튼 클릭 처리
     */
    void onHistoryButtonClicked();

    /**
     * @brief 히스토리 선택 처리 (페이지 열기)
     * @param url 선택한 URL
     * @param title 페이지 제목
     */
    void onHistorySelected(const QString& url, const QString& title);

    /**
     * @brief WebView 로딩 에러 핸들러
     * @param errorString 에러 메시지
     */
    void onLoadError(const QString &errorString);

    /**
     * @brief WebView 타임아웃 핸들러
     */
    void onLoadTimeout();

    /**
     * @brief ErrorPage 재시도 핸들러
     */
    void onRetryRequested();

    /**
     * @brief ErrorPage 홈 이동 핸들러
     */
    void onHomeRequested();

    /**
     * @brief HTTP 경고 다이얼로그 표시 (디바운싱 후 호출)
     */
    void onWarningTimerTimeout();

    /**
     * @brief 다운로드 완료 알림
     * @param item 완료된 다운로드 항목
     */
    void onDownloadCompleted(const DownloadItem& item);

private:
    /**
     * @brief HTTP 경고 필요 여부 체크 및 타이머 시작
     * @param url 체크할 URL
     */
    void checkHttpWarning(const QUrl &url);

    /**
     * @brief 보안 경고 다이얼로그 표시
     * @param url 경고 대상 URL
     * @return true면 계속 진행, false면 돌아가기
     */
    bool showSecurityWarningDialog(const QUrl &url);

private:
    // UI 컴포넌트
    QWidget *centralWidget_;         ///< 중앙 위젯
    QVBoxLayout *mainLayout_;        ///< 메인 레이아웃
    URLBar *urlBar_;                 ///< URL 입력 바
    NavigationBar *navigationBar_;   ///< 네비게이션 바
    LoadingIndicator *loadingIndicator_;  ///< 로딩 인디케이터
    QWidget *contentWidget_;         ///< WebView/ErrorPage 컨테이너
    QStackedLayout *stackedLayout_;  ///< 전환 레이아웃
    WebView *webView_;               ///< 웹뷰 컴포넌트
    ErrorPage *errorPage_;           ///< 에러 화면
    QLabel *statusLabel_;            ///< 상태 라벨 (하단)
    BookmarkPanel *bookmarkPanel_;   ///< 북마크 패널
    HistoryPanel *historyPanel_;     ///< 히스토리 패널 (오버레이)
    DownloadPanel *downloadPanel_;   ///< 다운로드 패널

    // 서비스
    TabManager *tabManager_;         ///< 탭 관리자
    StorageService *storageService_; ///< 스토리지 서비스
    BookmarkService *bookmarkService_; ///< 북마크 서비스
    HistoryService *historyService_; ///< 히스토리 서비스
    DownloadManager *downloadManager_; ///< 다운로드 관리자

    // 현재 페이지 정보
    QString currentUrl_;             ///< 현재 URL
    QString currentTitle_;           ///< 현재 페이지 제목

    // 보안 관련 상태
    QSet<QString> ignoredDomains_;   ///< 경고 무시 도메인 목록 (세션 단위)
    QTimer *warningTimer_;           ///< 경고 디바운싱 타이머 (500ms)
    QUrl pendingUrl_;                ///< 경고 대기 중인 URL

    // 리모컨 단축키 관리
    QMap<int, qint64> lastKeyPressTime_;  ///< 키별 마지막 입력 시각 (ms)
    static constexpr int DEBOUNCE_MS = 500;  ///< 디바운스 지속 시간 (0.5초)

    // 자동 스크롤 (M3 이후)
    // QTimer *autoScrollTimer_;  ///< 자동 스크롤 타이머
    // bool isAutoScrolling_;     ///< 자동 스크롤 중 여부
    // static constexpr int AUTO_SCROLL_INTERVAL_MS = 10;  ///< 스크롤 간격 (10ms)
    // static constexpr int AUTO_SCROLL_STEP_PX = 5;       ///< 스크롤 단계 (5px)
};

} // namespace webosbrowser
