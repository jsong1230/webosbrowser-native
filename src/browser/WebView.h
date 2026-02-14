/**
 * @file WebView.h
 * @brief WebView 컴포넌트 헤더 - webOS WebView API 래퍼
 */

#pragma once

#include <QWidget>
#include <QString>
#include <QUrl>
#include <QScopedPointer>

namespace webosbrowser {

/**
 * @enum LoadingState
 * @brief 웹 페이지 로딩 상태
 */
enum class LoadingState {
    Idle,       ///< 대기 중 (로딩 전)
    Loading,    ///< 로딩 중
    Loaded,     ///< 로딩 완료
    Error       ///< 로딩 실패
};

// Forward declaration
class WebViewPrivate;
class DownloadManager;

/**
 * @class WebView
 * @brief Qt WebEngineView 기반 웹 페이지 렌더링 위젯
 *
 * PIMPL 패턴을 사용하여 QWebEngineView의 구현 세부사항을 캡슐화합니다.
 * 시그널/슬롯을 통해 로딩 상태, URL 변경, 에러를 외부에 알립니다.
 */
class WebView : public QWidget {
    Q_OBJECT

public:
    /**
     * @brief 생성자
     * @param parent 부모 위젯 (기본값: nullptr)
     */
    explicit WebView(QWidget *parent = nullptr);

    /**
     * @brief 소멸자
     */
    ~WebView() override;

    // 복사 생성자 및 대입 연산자 삭제 (QObject 규칙)
    WebView(const WebView&) = delete;
    WebView& operator=(const WebView&) = delete;

    /**
     * @brief URL 로드
     * @param url 로드할 URL
     */
    void load(const QUrl &url);

    /**
     * @brief URL 로드 (QString 오버로드)
     * @param url 로드할 URL 문자열
     */
    void load(const QString &url);

    /**
     * @brief 현재 페이지 새로고침
     */
    void reload();

    /**
     * @brief 로딩 중지
     */
    void stop();

    /**
     * @brief 뒤로 가기
     */
    void goBack();

    /**
     * @brief 앞으로 가기
     */
    void goForward();

    /**
     * @brief 뒤로 가기 가능 여부
     * @return 뒤로 갈 수 있으면 true
     */
    bool canGoBack() const;

    /**
     * @brief 앞으로 가기 가능 여부
     * @return 앞으로 갈 수 있으면 true
     */
    bool canGoForward() const;

    /**
     * @brief 현재 URL 반환
     * @return 현재 페이지 URL
     */
    QUrl url() const;

    /**
     * @brief 현재 페이지 제목 반환
     * @return 페이지 제목
     */
    QString title() const;

    /**
     * @brief 현재 로딩 상태 반환
     * @return 로딩 상태 (LoadingState enum)
     */
    LoadingState loadingState() const;

    /**
     * @brief 로딩 진행률 반환
     * @return 진행률 (0~100)
     */
    int loadProgress() const;

    /**
     * @brief 다운로드 핸들러 설정
     * @param downloadManager 다운로드 관리자
     */
    void setupDownloadHandler(DownloadManager* downloadManager);

signals:
    /**
     * @brief 페이지 로딩 시작 시그널
     */
    void loadStarted();

    /**
     * @brief 페이지 로딩 진행 시그널
     * @param progress 진행률 (0~100)
     */
    void loadProgress(int progress);

    /**
     * @brief 페이지 로딩 완료 시그널
     * @param success 성공 여부
     */
    void loadFinished(bool success);

    /**
     * @brief 페이지 제목 변경 시그널
     * @param title 새 제목
     */
    void titleChanged(const QString &title);

    /**
     * @brief URL 변경 시그널
     * @param url 새 URL
     */
    void urlChanged(const QUrl &url);

    /**
     * @brief 로딩 에러 시그널
     * @param errorString 에러 메시지
     */
    void loadError(const QString &errorString);

    /**
     * @brief 로딩 타임아웃 시그널 (30초 초과)
     */
    void loadTimeout();

private:
    QScopedPointer<WebViewPrivate> d_ptr;  ///< PIMPL 패턴 구현 포인터
    Q_DECLARE_PRIVATE(WebView)             ///< Qt PIMPL 매크로
};

} // namespace webosbrowser
