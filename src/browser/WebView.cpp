/**
 * @file WebView.cpp
 * @brief WebView 컴포넌트 구현
 */

#include "WebView.h"
#include <QWebEngineView>
#include <QWebEnginePage>
#include <QVBoxLayout>
#include <QTimer>
#include <QDebug>

namespace webosbrowser {

/**
 * @class WebViewPrivate
 * @brief WebView의 PIMPL 구현 클래스
 *
 * QWebEngineView와 관련된 구현 세부사항을 캡슐화합니다.
 */
class WebViewPrivate {
public:
    WebViewPrivate(WebView *q)
        : q_ptr(q)
        , webEngineView(new QWebEngineView(q))
        , loadingState(LoadingState::Idle)
        , currentProgress(0)
        , loadingTimer(new QTimer(q))
    {
        loadingTimer->setSingleShot(true);
        loadingTimer->setInterval(30000);  // 30초 타임아웃
    }

    ~WebViewPrivate() {
        // 타이머 정리 (활성 상태일 경우 정지)
        if (loadingTimer && loadingTimer->isActive()) {
            loadingTimer->stop();
        }
        // QWebEngineView와 QTimer는 부모(q_ptr)에 의해 자동 삭제됨
    }

    WebView *q_ptr;                        ///< 공개 인터페이스 포인터
    QWebEngineView *webEngineView;         ///< Qt WebEngine 뷰
    LoadingState loadingState;             ///< 현재 로딩 상태
    int currentProgress;                   ///< 로딩 진행률 (0~100)
    QTimer *loadingTimer;                  ///< 타임아웃 타이머

    /**
     * @brief 시그널/슬롯 연결 설정
     */
    void setupConnections();

    /**
     * @brief 로딩 상태 변경
     * @param newState 새 상태
     */
    void setLoadingState(LoadingState newState);
};

void WebViewPrivate::setupConnections() {
    Q_Q(WebView);

    // 로딩 시작 이벤트
    QObject::connect(webEngineView, &QWebEngineView::loadStarted, q, [this, q]() {
        qDebug() << "WebView: 로딩 시작";
        setLoadingState(LoadingState::Loading);
        currentProgress = 0;
        loadingTimer->start();  // 타임아웃 타이머 시작
        emit q->loadStarted();
    });

    // 로딩 진행률 이벤트
    QObject::connect(webEngineView, &QWebEngineView::loadProgress, q, [this, q](int progress) {
        currentProgress = progress;
        emit q->loadProgress(progress);
    });

    // 로딩 완료 이벤트
    QObject::connect(webEngineView, &QWebEngineView::loadFinished, q, [this, q](bool success) {
        loadingTimer->stop();  // 타임아웃 타이머 정지

        if (success) {
            qDebug() << "WebView: 로딩 완료 -" << webEngineView->url().toString();
            setLoadingState(LoadingState::Loaded);
            currentProgress = 100;
        } else {
            qDebug() << "WebView: 로딩 실패 -" << webEngineView->url().toString();
            setLoadingState(LoadingState::Error);
            emit q->loadError("페이지 로딩 실패");
        }

        emit q->loadFinished(success);
    });

    // 제목 변경 이벤트
    QObject::connect(webEngineView, &QWebEngineView::titleChanged, q, [q](const QString &title) {
        qDebug() << "WebView: 제목 변경 -" << title;
        emit q->titleChanged(title);
    });

    // URL 변경 이벤트
    QObject::connect(webEngineView, &QWebEngineView::urlChanged, q, [q](const QUrl &url) {
        qDebug() << "WebView: URL 변경 -" << url.toString();
        emit q->urlChanged(url);
    });

    // 타임아웃 이벤트
    QObject::connect(loadingTimer, &QTimer::timeout, q, [this, q]() {
        qDebug() << "WebView: 로딩 타임아웃 (30초 초과)";
        setLoadingState(LoadingState::Error);
        webEngineView->stop();
        emit q->loadTimeout();
        emit q->loadError("로딩 타임아웃 (30초 초과)");
    });
}

void WebViewPrivate::setLoadingState(LoadingState newState) {
    if (loadingState != newState) {
        loadingState = newState;
        qDebug() << "WebView: 상태 변경 -" << static_cast<int>(newState);
    }
}

// WebView 구현

WebView::WebView(QWidget *parent)
    : QWidget(parent)
    , d_ptr(new WebViewPrivate(this))
{
    Q_D(WebView);

    qDebug() << "WebView: 생성 중...";

    // 레이아웃 설정
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(d->webEngineView);
    setLayout(layout);

    // 시그널/슬롯 연결
    d->setupConnections();

    qDebug() << "WebView: 생성 완료";
}

WebView::~WebView() {
    qDebug() << "WebView: 소멸";
}

void WebView::load(const QUrl &url) {
    Q_D(WebView);
    qDebug() << "WebView: URL 로드 요청 -" << url.toString();

    if (!url.isValid()) {
        qWarning() << "WebView: 유효하지 않은 URL -" << url.toString();
        emit loadError("유효하지 않은 URL: " + url.toString());
        return;
    }

    d->webEngineView->load(url);
}

void WebView::load(const QString &url) {
    load(QUrl(url));
}

void WebView::reload() {
    Q_D(WebView);
    qDebug() << "WebView: 새로고침";
    d->webEngineView->reload();
}

void WebView::stop() {
    Q_D(WebView);
    qDebug() << "WebView: 로딩 중지";
    d->loadingTimer->stop();
    d->webEngineView->stop();
    d->setLoadingState(LoadingState::Idle);
    d->currentProgress = 0;  // 진행률 리셋
}

void WebView::goBack() {
    Q_D(WebView);
    if (canGoBack()) {
        qDebug() << "WebView: 뒤로 가기";
        d->webEngineView->back();
    }
}

void WebView::goForward() {
    Q_D(WebView);
    if (canGoForward()) {
        qDebug() << "WebView: 앞으로 가기";
        d->webEngineView->forward();
    }
}

bool WebView::canGoBack() const {
    Q_D(const WebView);
    return d->webEngineView->history()->canGoBack();
}

bool WebView::canGoForward() const {
    Q_D(const WebView);
    return d->webEngineView->history()->canGoForward();
}

QUrl WebView::url() const {
    Q_D(const WebView);
    return d->webEngineView->url();
}

QString WebView::title() const {
    Q_D(const WebView);
    return d->webEngineView->title();
}

LoadingState WebView::loadingState() const {
    Q_D(const WebView);
    return d->loadingState;
}

int WebView::loadProgress() const {
    Q_D(const WebView);
    return d->currentProgress;
}

} // namespace webosbrowser
