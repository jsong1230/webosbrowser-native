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
#include <QLabel>
#include <QTextBrowser>

namespace webosbrowser {

/**
 * @brief WebViewPrivate 스텁 (시각화 추가)
 *
 * PIMPL 패턴을 위한 클래스. 실제 웹 렌더링은 없지만 URL과 상태를 표시합니다.
 */
class WebViewPrivate {
public:
    QLabel *statusLabel;
    QTextBrowser *contentBrowser;
    QUrl currentUrl;

    WebViewPrivate()
        : statusLabel(nullptr)
        , contentBrowser(nullptr)
        , currentUrl("about:blank")
    {}

    ~WebViewPrivate() {}
};

/**
 * @brief 생성자 (스텁 - 시각화 추가)
 */
WebView::WebView(QWidget *parent)
    : QWidget(parent)
    , d_ptr(new WebViewPrivate())
{
    Logger::warning("[WebView] 스텁 구현 - 실제 웹 렌더링 기능 없음 (시각화 표시)");

    // 메인 레이아웃
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // 상태 표시 라벨
    d_ptr->statusLabel = new QLabel(this);
    d_ptr->statusLabel->setStyleSheet(
        "QLabel { "
        "  background-color: #f0f0f0; "
        "  color: #333; "
        "  padding: 10px; "
        "  font-size: 14px; "
        "  border-bottom: 2px solid #ddd; "
        "}"
    );
    d_ptr->statusLabel->setAlignment(Qt::AlignCenter);
    d_ptr->statusLabel->setText("🔧 WebView Stub - 실제 렌더링 없음");
    layout->addWidget(d_ptr->statusLabel);

    // 컨텐츠 영역 (URL 정보 표시)
    d_ptr->contentBrowser = new QTextBrowser(this);
    d_ptr->contentBrowser->setStyleSheet(
        "QTextBrowser { "
        "  background-color: white; "
        "  border: none; "
        "  padding: 20px; "
        "  font-size: 16px; "
        "}"
    );
    d_ptr->contentBrowser->setHtml(
        "<div style='text-align: center; padding-top: 100px;'>"
        "<h2 style='color: #666;'>⚠️ WebView Stub Mode</h2>"
        "<p style='color: #999; margin-top: 20px;'>실제 웹 렌더링 기능이 없습니다.</p>"
        "<p style='color: #999;'>webOS 디바이스에서는 정상 동작합니다.</p>"
        "</div>"
    );
    layout->addWidget(d_ptr->contentBrowser);

    setLayout(layout);
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
 * @brief URL 로드 (스텁 - 시각화 추가)
 */
void WebView::load(const QUrl &url)
{
    Logger::info(QString("[WebView] 스텁 load(QUrl): %1").arg(url.toString()));

    d_ptr->currentUrl = url;

    // 상태 라벨 업데이트 (색상 변경으로 눈에 띄게)
    static int loadCount = 0;
    loadCount++;
    QString colors[] = {"#4285f4", "#ea4335", "#34a853", "#fbbc04"};
    QString color = colors[loadCount % 4];

    d_ptr->statusLabel->setStyleSheet(
        QString("QLabel { "
        "  background-color: %1; "
        "  color: white; "
        "  padding: 10px; "
        "  font-size: 14px; "
        "  font-weight: bold; "
        "  border-bottom: 2px solid #333; "
        "}").arg(color)
    );
    d_ptr->statusLabel->setText(QString("🌐 로드 #%1: %2").arg(loadCount).arg(url.toString()));

    // 컨텐츠 영역 업데이트
    // 도메인 추출
    QString domain = url.host();
    if (domain.isEmpty()) domain = "Unknown";

    // 랜덤 배경색으로 변화 강조
    QStringList bgColors = {"#e3f2fd", "#fff3e0", "#e8f5e9", "#fce4ec"};
    QString bgColor = bgColors[loadCount % bgColors.size()];

    QString html = QString(
        "<div style='padding: 40px; font-family: sans-serif; background: %2;'>"
        "<div style='text-align: center; margin-bottom: 40px;'>"
        "<h1 style='color: #4285f4; font-size: 48px;'>🌐 로드 #%3</h1>"
        "<p style='color: #666; font-size: 18px;'>WebView Stub Mode</p>"
        "</div>"
        "<div style='background: white; padding: 30px; border-radius: 12px; margin-bottom: 20px; box-shadow: 0 2px 8px rgba(0,0,0,0.1);'>"
        "<h2 style='color: #333; margin-top: 0; text-align: center;'>📍 %4</h2>"
        "<p style='color: #4285f4; font-size: 24px; word-wrap: break-word; text-align: center; font-weight: bold;'>%1</p>"
        "</div>"
        "<div style='background: #fff3cd; padding: 15px; border-radius: 8px; border-left: 4px solid #ffc107;'>"
        "<h4 style='color: #856404; margin-top: 0;'>⚠️ 주의사항</h4>"
        "<ul style='color: #856404; margin: 0;'>"
        "<li>이것은 <b>스텁(Stub) 구현</b>입니다</li>"
        "<li>Qt WebEngine이 없어 실제 웹 페이지를 표시할 수 없습니다</li>"
        "<li>webOS 프로젝터에서는 <b>정상 작동</b>합니다</li>"
        "</ul>"
        "</div>"
        "<div style='margin-top: 30px; text-align: center; color: #999; font-size: 12px;'>"
        "<p>💡 UI 구조와 기능은 정상 동작하고 있습니다</p>"
        "</div>"
        "</div>"
    ).arg(url.toString()).arg(bgColor).arg(loadCount).arg(domain);

    d_ptr->contentBrowser->setHtml(html);

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
    return d_ptr->currentUrl;
}

/**
 * @brief 페이지 제목 반환 (스텁)
 */
QString WebView::title() const
{
    if (d_ptr->currentUrl.isEmpty() || d_ptr->currentUrl == QUrl("about:blank")) {
        return "WebView Stub";
    }
    return d_ptr->currentUrl.host();
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
