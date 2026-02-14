/**
 * @file BrowserWindow.cpp
 * @brief 브라우저 메인 윈도우 구현
 */

#include "BrowserWindow.h"
#include "WebView.h"
#include "TabManager.h"
#include "../ui/URLBar.h"
#include "../ui/NavigationBar.h"
#include "../ui/LoadingIndicator.h"
#include "../ui/HistoryPanel.h"
#include "../services/StorageService.h"
#include "../services/HistoryService.h"
#include "../utils/Logger.h"
#include <QDebug>
#include <QApplication>
#include <QScreen>
#include <QLabel>
#include <QStatusBar>

namespace webosbrowser {

BrowserWindow::BrowserWindow(QWidget *parent)
    : QMainWindow(parent)
    , centralWidget_(new QWidget(this))
    , mainLayout_(new QVBoxLayout(centralWidget_))
    , urlBar_(new URLBar(centralWidget_))
    , navigationBar_(new NavigationBar(centralWidget_))
    , loadingIndicator_(new LoadingIndicator(centralWidget_))
    , webView_(new WebView(centralWidget_))
    , statusLabel_(new QLabel("준비", this))
    , historyPanel_(nullptr)
    , tabManager_(new TabManager(this))
    , storageService_(new StorageService(this))
    , historyService_(nullptr)
{
    qDebug() << "BrowserWindow: 생성 중...";

    // 스토리지 서비스 초기화
    if (!storageService_->initialize()) {
        Logger::error("BrowserWindow: StorageService 초기화 실패");
    }

    // 히스토리 서비스 초기화
    historyService_ = new HistoryService(storageService_, this);

    // 히스토리 패널 생성 (오버레이)
    historyPanel_ = new HistoryPanel(historyService_, this);
    historyPanel_->setGeometry(
        width() - 600, 0,  // 우측 정렬
        600, height()
    );

    setupUI();
    setupConnections();

    qDebug() << "BrowserWindow: 생성 완료";
}

BrowserWindow::~BrowserWindow() {
    qDebug() << "BrowserWindow: 소멸";
}

void BrowserWindow::setupUI() {
    // 윈도우 기본 설정
    setWindowTitle("webOS Browser");
    resize(1920, 1080);  // webOS 기본 해상도

    // 중앙 위젯 설정
    setCentralWidget(centralWidget_);

    // 메인 레이아웃 설정
    mainLayout_->setContentsMargins(0, 0, 0, 0);
    mainLayout_->setSpacing(0);

    // URLBar 추가 (최상단)
    mainLayout_->addWidget(urlBar_);

    // NavigationBar 추가 (URLBar 아래)
    mainLayout_->addWidget(navigationBar_);

    // LoadingIndicator 추가 (NavigationBar 아래, 얇은 프로그레스바)
    mainLayout_->addWidget(loadingIndicator_);

    // WebView 추가 (중간, stretch=1로 남은 공간 모두 차지)
    mainLayout_->addWidget(webView_, 1);

    // NavigationBar와 WebView 연결
    navigationBar_->setWebView(webView_);

    // TabManager에 현재 탭 등록
    tabManager_->setCurrentTab(webView_);

    // 상태 바 설정
    statusLabel_->setStyleSheet(
        "QLabel {"
        "  padding: 5px;"
        "  background-color: #f0f0f0;"
        "  font-size: 14px;"
        "}"
    );
    statusBar()->addPermanentWidget(statusLabel_, 1);

    // 초기 페이지 로드 (테스트용)
    webView_->load("https://www.google.com");

    qDebug() << "BrowserWindow: UI 설정 완료";
}

void BrowserWindow::setupConnections() {
    // URLBar → WebView 연결
    connect(urlBar_, &URLBar::urlSubmitted, webView_,
            static_cast<void(WebView::*)(const QUrl&)>(&WebView::load));

    // WebView → LoadingIndicator 연결
    connect(webView_, &WebView::loadStarted, loadingIndicator_, &LoadingIndicator::startLoading);
    connect(webView_, &WebView::loadProgress, loadingIndicator_, &LoadingIndicator::setProgress);
    connect(webView_, &WebView::loadFinished, loadingIndicator_, &LoadingIndicator::finishLoading);

    // WebView 로딩 시작 이벤트
    connect(webView_, &WebView::loadStarted, this, [this]() {
        statusLabel_->setText("로딩 중...");
        setWindowTitle("로딩 중... - webOS Browser");
        qDebug() << "BrowserWindow: 페이지 로딩 시작";
    });

    // WebView 로딩 진행률 이벤트
    connect(webView_, &WebView::loadProgress, this, [this](int progress) {
        statusLabel_->setText(QString("로딩 중... %1%").arg(progress));
    });

    // WebView 로딩 완료 이벤트
    connect(webView_, &WebView::loadFinished, this, [this](bool success) {
        if (success) {
            statusLabel_->setText("완료");
            QString title = webView_->title();
            if (title.isEmpty()) {
                title = webView_->url().toString();
            }
            setWindowTitle(title + " - webOS Browser");
            qDebug() << "BrowserWindow: 페이지 로딩 완료";
        } else {
            statusLabel_->setText("로딩 실패");
            setWindowTitle("로딩 실패 - webOS Browser");
            qDebug() << "BrowserWindow: 페이지 로딩 실패";
        }
    });

    // WebView 제목 변경 이벤트
    connect(webView_, &WebView::titleChanged, this, [this](const QString &title) {
        if (!title.isEmpty()) {
            setWindowTitle(title + " - webOS Browser");
        }
    });

    // WebView URL 변경 이벤트 (URLBar + StatusLabel 동시 업데이트)
    connect(webView_, &WebView::urlChanged, this, [this](const QUrl &url) {
        urlBar_->setText(url.toString());
        statusLabel_->setText(url.toString());
        qDebug() << "BrowserWindow: URL 변경 -" << url.toString();
    });

    // WebView 에러 이벤트 (URLBar + StatusLabel 동시 업데이트)
    connect(webView_, &WebView::loadError, this, [this](const QString &errorString) {
        urlBar_->showError(errorString);
        statusLabel_->setText("에러: " + errorString);
        qDebug() << "BrowserWindow: 로딩 에러 -" << errorString;
    });

    // WebView 타임아웃 이벤트
    connect(webView_, &WebView::loadTimeout, this, [this]() {
        statusLabel_->setText("타임아웃: 30초 초과");
        qDebug() << "BrowserWindow: 로딩 타임아웃";
    });

    // WebView 로딩 완료 → 히스토리 자동 기록
    connect(webView_, &WebView::loadFinished, this, &BrowserWindow::onPageLoadFinished);

    // NavigationBar 히스토리 버튼 → 히스토리 패널 열기
    // TODO: NavigationBar에 historyButtonClicked 시그널 추가 필요
    // connect(navigationBar_, &NavigationBar::historyButtonClicked, this, &BrowserWindow::onHistoryButtonClicked);

    // HistoryPanel 시그널 연결
    if (historyPanel_) {
        connect(historyPanel_, &HistoryPanel::historySelected, this, &BrowserWindow::onHistorySelected);
    }

    qDebug() << "BrowserWindow: 시그널/슬롯 연결 완료";
}

void BrowserWindow::onPageLoadFinished(bool success) {
    if (!success || !historyService_) {
        return;
    }

    // 페이지 로딩 성공 시 히스토리 자동 기록
    QString url = webView_->url().toString();
    QString title = webView_->title();

    if (url.isEmpty()) {
        return;
    }

    // 히스토리 기록
    historyService_->recordVisit(url, title);
    Logger::info(QString("히스토리 자동 기록: %1").arg(url));
}

void BrowserWindow::onHistoryButtonClicked() {
    if (historyPanel_) {
        historyPanel_->togglePanel();
    }
}

void BrowserWindow::onHistorySelected(const QString &url, const QString &title) {
    Q_UNUSED(title)

    // 히스토리에서 선택한 URL로 페이지 로드
    webView_->load(url);
    Logger::info(QString("히스토리에서 페이지 열기: %1").arg(url));
}

} // namespace webosbrowser
