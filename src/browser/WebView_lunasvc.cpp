/**
 * @file WebView_lunasvc.cpp
 * @brief Luna Service API 기반 WebView 구현
 *
 * webOS Luna Service를 사용하여 시스템 브라우저를 호출합니다.
 * com.webos.service.applicationmanager의 launch 메소드를 사용합니다.
 */

#include "WebView.h"
#include "../utils/Logger.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QProcess>
#include <QJsonDocument>
#include <QJsonObject>

namespace webosbrowser {

/**
 * @brief WebViewPrivate - Luna Service 기반
 */
class WebViewPrivate {
public:
    QLabel *statusLabel;
    QUrl currentUrl;
    QProcess *lunaProcess;

    WebViewPrivate()
        : statusLabel(nullptr)
        , currentUrl("about:blank")
        , lunaProcess(nullptr)
    {}

    ~WebViewPrivate() {
        if (lunaProcess) {
            lunaProcess->kill();
            delete lunaProcess;
        }
    }
};

/**
 * @brief 생성자
 */
WebView::WebView(QWidget *parent)
    : QWidget(parent)
    , d_ptr(new WebViewPrivate())
{
    Logger::info("[WebView] Luna Service 기반 WebView 초기화");

    // UI 레이아웃
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(10, 10, 10, 10);

    // 상태 표시 레이블
    d_ptr->statusLabel = new QLabel("Luna Service WebView 준비됨", this);
    d_ptr->statusLabel->setStyleSheet(
        "QLabel {"
        "  font-size: 18px;"
        "  color: #4285f4;"
        "  padding: 10px;"
        "  background-color: #f0f0f0;"
        "  border-radius: 5px;"
        "}"
    );
    layout->addWidget(d_ptr->statusLabel);

    layout->addStretch();
    setLayout(layout);
}

/**
 * @brief 소멸자
 */
WebView::~WebView()
{
    Logger::debug("[WebView] Luna Service 기반 WebView 소멸");
    delete d_ptr;
}

/**
 * @brief URL 로드 (Luna Service 사용)
 */
void WebView::load(const QUrl &url)
{
    d_ptr->currentUrl = url;
    Logger::info("[WebView] Luna Service로 URL 로드: " + url.toString());

    // luna-send 명령으로 시스템 브라우저 실행
    QString lunaCmd = QString(
        "luna-send -n 1 -f luna://com.webos.service.applicationmanager/launch "
        "'{\"id\":\"com.webos.app.browser\",\"params\":{\"target\":\"%1\"}}'"
    ).arg(url.toString());

    if (!d_ptr->lunaProcess) {
        d_ptr->lunaProcess = new QProcess(this);
    }

    // 상태 업데이트
    d_ptr->statusLabel->setText(QString("로딩 중: %1").arg(url.toString()));

    // Luna Service 호출
    d_ptr->lunaProcess->start("sh", QStringList() << "-c" << lunaCmd);

    connect(d_ptr->lunaProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this](int exitCode, QProcess::ExitStatus exitStatus) {
        if (exitCode == 0) {
            Logger::info("[WebView] Luna Service 호출 성공");
            d_ptr->statusLabel->setText(QString("로드 완료: %1").arg(d_ptr->currentUrl.toString()));
            emit loadFinished(true);
        } else {
            Logger::error("[WebView] Luna Service 호출 실패: " + QString::number(exitCode));
            d_ptr->statusLabel->setText(QString("로드 실패: %1").arg(d_ptr->currentUrl.toString()));
            emit loadFinished(false);
        }
    });

    emit loadStarted();
}

/**
 * @brief 뒤로 가기 (제한적 지원)
 */
void WebView::back()
{
    Logger::warning("[WebView] Luna Service 모드에서는 history.back() 지원 안 됨");
    d_ptr->statusLabel->setText("뒤로 가기: Luna Service 제약으로 지원 안 됨");
}

/**
 * @brief 앞으로 가기 (제한적 지원)
 */
void WebView::forward()
{
    Logger::warning("[WebView] Luna Service 모드에서는 history.forward() 지원 안 됨");
    d_ptr->statusLabel->setText("앞으로 가기: Luna Service 제약으로 지원 안 됨");
}

/**
 * @brief 새로고침
 */
void WebView::reload()
{
    Logger::info("[WebView] 페이지 새로고침: " + d_ptr->currentUrl.toString());
    load(d_ptr->currentUrl);
}

/**
 * @brief URL 반환
 */
QUrl WebView::url() const
{
    return d_ptr->currentUrl;
}

/**
 * @brief 페이지 제목 (Luna Service에서는 제한적)
 */
QString WebView::title() const
{
    return d_ptr->currentUrl.toString();
}

/**
 * @brief 로딩 상태 (항상 false)
 */
bool WebView::isLoading() const
{
    return false;
}

/**
 * @brief 히스토리 - 뒤로 가능 여부 (항상 false)
 */
bool WebView::canGoBack() const
{
    return false;
}

/**
 * @brief 히스토리 - 앞으로 가능 여부 (항상 false)
 */
bool WebView::canGoForward() const
{
    return false;
}

} // namespace webosbrowser
