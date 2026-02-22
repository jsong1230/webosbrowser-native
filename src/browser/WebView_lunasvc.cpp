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
    LoadingState loadingState;
    int currentProgress;

    WebViewPrivate()
        : statusLabel(nullptr)
        , currentUrl("about:blank")
        , lunaProcess(nullptr)
        , loadingState(LoadingState::Idle)
        , currentProgress(0)
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
 * @brief 소멸자 - QScopedPointer가 d_ptr 자동 해제
 */
WebView::~WebView()
{
    Logger::debug("[WebView] Luna Service 기반 WebView 소멸");
}

/**
 * @brief URL 로드 (Luna Service 사용)
 */
void WebView::load(const QUrl &url)
{
    d_ptr->currentUrl = url;
    d_ptr->loadingState = LoadingState::Loading;
    d_ptr->currentProgress = 0;
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
            this, [this](int exitCode, QProcess::ExitStatus) {
        if (exitCode == 0) {
            Logger::info("[WebView] Luna Service 호출 성공");
            d_ptr->statusLabel->setText(QString("로드 완료: %1").arg(d_ptr->currentUrl.toString()));
            d_ptr->loadingState = LoadingState::Loaded;
            d_ptr->currentProgress = 100;
            emit loadFinished(true);
        } else {
            Logger::error("[WebView] Luna Service 호출 실패: " + QString::number(exitCode));
            d_ptr->statusLabel->setText(QString("로드 실패: %1").arg(d_ptr->currentUrl.toString()));
            d_ptr->loadingState = LoadingState::Error;
            emit loadError("Luna Service 호출 실패");
            emit loadFinished(false);
        }
    });

    emit loadStarted();
}

/**
 * @brief URL 로드 (QString 오버로드)
 */
void WebView::load(const QString &url)
{
    load(QUrl(url));
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
 * @brief 로딩 중지
 */
void WebView::stop()
{
    if (d_ptr->lunaProcess && d_ptr->lunaProcess->state() != QProcess::NotRunning) {
        d_ptr->lunaProcess->kill();
        Logger::info("[WebView] 로딩 중지");
    }
    d_ptr->loadingState = LoadingState::Idle;
    d_ptr->currentProgress = 0;
}

/**
 * @brief 뒤로 가기 (Luna Service 제약으로 미지원)
 */
void WebView::goBack()
{
    Logger::warning("[WebView] Luna Service 모드에서는 goBack() 미지원");
    d_ptr->statusLabel->setText("뒤로 가기: Luna Service 제약으로 미지원");
}

/**
 * @brief 앞으로 가기 (Luna Service 제약으로 미지원)
 */
void WebView::goForward()
{
    Logger::warning("[WebView] Luna Service 모드에서는 goForward() 미지원");
    d_ptr->statusLabel->setText("앞으로 가기: Luna Service 제약으로 미지원");
}

/**
 * @brief 뒤로 가기 가능 여부 (항상 false)
 */
bool WebView::canGoBack() const
{
    return false;
}

/**
 * @brief 앞으로 가기 가능 여부 (항상 false)
 */
bool WebView::canGoForward() const
{
    return false;
}

/**
 * @brief 현재 URL 반환
 */
QUrl WebView::url() const
{
    return d_ptr->currentUrl;
}

/**
 * @brief 페이지 제목 반환
 */
QString WebView::title() const
{
    return d_ptr->currentUrl.toString();
}

/**
 * @brief 현재 로딩 상태 반환
 */
LoadingState WebView::loadingState() const
{
    return d_ptr->loadingState;
}

/**
 * @brief 로딩 진행률 반환 (0~100)
 */
int WebView::loadProgress() const
{
    return d_ptr->currentProgress;
}

/**
 * @brief 다운로드 핸들러 설정 (Luna Service 모드에서는 미지원)
 */
void WebView::setupDownloadHandler(DownloadManager* downloadManager)
{
    Q_UNUSED(downloadManager)
    Logger::warning("[WebView] Luna Service 모드에서는 다운로드 핸들러 미지원");
}

} // namespace webosbrowser
