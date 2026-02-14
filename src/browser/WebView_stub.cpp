/**
 * @file WebView_stub.cpp
 * @brief WebView 스텁 구현 (WebEngine 없는 빌드용)
 *
 * 주의: 이것은 빌드 전용 스텁입니다. 실제 웹 렌더링 기능은 없습니다.
 * 실제 webOS 환경에서는 webOS WebView API를 사용하는 진짜 구현으로 교체해야 합니다.
 */

#include "WebView.h"
#include "../utils/Logger.h"
#include <QVBoxLayout>

namespace webosbrowser {

/**
 * @brief WebViewPrivate 스텁 (빈 구현)
 *
 * PIMPL 패턴을 위한 빈 클래스. 실제 구현은 없습니다.
 */
class WebViewPrivate {
public:
    WebViewPrivate() {}
    ~WebViewPrivate() {}
};

/**
 * @brief 생성자 (스텁)
 */
WebView::WebView(QWidget *parent)
    : QWidget(parent)
    , d_ptr(new WebViewPrivate())
{
    Logger::warning("[WebView] 스텁 구현 - 실제 웹 렌더링 기능 없음");

    // 기본 레이아웃만 생성
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);

    // 포커스 정책 설정
    setFocusPolicy(Qt::StrongFocus);
}

/**
 * @brief 소멸자 (스텁)
 */
WebView::~WebView()
{
    Logger::debug("[WebView] 스텁 소멸");
}

/**
 * @brief URL 로드 (스텁)
 */
void WebView::load(const QUrl &url)
{
    Logger::info(QString("[WebView] 스텁 load(QUrl): %1").arg(url.toString()));
    emit urlChanged(url);
    emit loadStarted();
    emit loadProgress(100);
    emit loadFinished(true);
}

/**
 * @brief URL 로드 QString 오버로드 (스텁)
 */
void WebView::load(const QString &url)
{
    load(QUrl(url));
}

/**
 * @brief 새로고침 (스텁)
 */
void WebView::reload()
{
    Logger::debug("[WebView] 스텁 reload()");
    emit loadStarted();
    emit loadProgress(100);
    emit loadFinished(true);
}

/**
 * @brief 로딩 중지 (스텁)
 */
void WebView::stop()
{
    Logger::debug("[WebView] 스텁 stop()");
}

/**
 * @brief 뒤로가기 (스텁)
 */
void WebView::goBack()
{
    Logger::debug("[WebView] 스텁 goBack()");
}

/**
 * @brief 앞으로가기 (스텁)
 */
void WebView::goForward()
{
    Logger::debug("[WebView] 스텁 goForward()");
}

/**
 * @brief 뒤로가기 가능 여부 (스텁)
 */
bool WebView::canGoBack() const
{
    return false;  // 스텁은 히스토리 없음
}

/**
 * @brief 앞으로가기 가능 여부 (스텁)
 */
bool WebView::canGoForward() const
{
    return false;  // 스텁은 히스토리 없음
}

/**
 * @brief 현재 URL 반환 (스텁)
 */
QUrl WebView::url() const
{
    return QUrl("about:blank");
}

/**
 * @brief 페이지 제목 반환 (스텁)
 */
QString WebView::title() const
{
    return "WebView Stub";
}

/**
 * @brief 다운로드 핸들러 설정 (스텁)
 */
void WebView::setupDownloadHandler(DownloadManager* downloadManager)
{
    Q_UNUSED(downloadManager);
    Logger::debug("[WebView] 스텁 setupDownloadHandler() - 다운로드 기능 없음");
}

} // namespace webosbrowser
