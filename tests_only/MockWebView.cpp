/**
 * @file MockWebView.cpp
 * @brief 테스트용 Mock WebView 구현
 */

#include "../src/browser/WebView.h"
#include <QWidget>

namespace webosbrowser {

// WebViewPrivate의 완전한 정의
class WebViewPrivate {
public:
    WebViewPrivate() {}
    ~WebViewPrivate() {}
};

// 모든 메서드의 최소 구현
WebView::WebView(QWidget *parent) : QWidget(parent), d_ptr(nullptr) {}
WebView::~WebView() {}

void WebView::load(const QUrl &url) {}
void WebView::load(const QString &url) {}
void WebView::reload() {}
void WebView::stop() {}
void WebView::goBack() {}
void WebView::goForward() {}

bool WebView::canGoBack() const { return false; }
bool WebView::canGoForward() const { return false; }

QUrl WebView::url() const { return QUrl(); }
QString WebView::title() const { return QString(); }

LoadingState WebView::loadingState() const { return LoadingState::Idle; }
int WebView::loadProgress() const { return 0; }

} // namespace webosbrowser
