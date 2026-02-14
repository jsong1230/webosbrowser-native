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
#include "../ui/BookmarkPanel.h"
#include "../ui/ErrorPage.h"
#include "../ui/DownloadPanel.h"
#include "../services/StorageService.h"
#include "../services/HistoryService.h"
#include "../services/BookmarkService.h"
#include "../services/DownloadManager.h"
#include "../utils/Logger.h"
#include "../utils/SecurityClassifier.h"
#include <QDebug>
#include <QApplication>
#include <QScreen>
#include <QLabel>
#include <QStatusBar>
#include <QPropertyAnimation>
#include <QMessageBox>
#include <QPushButton>
#include <QCheckBox>

namespace webosbrowser {

BrowserWindow::BrowserWindow(QWidget *parent)
    : QMainWindow(parent)
    , centralWidget_(new QWidget(this))
    , mainLayout_(new QVBoxLayout(centralWidget_))
    , urlBar_(new URLBar(centralWidget_))
    , navigationBar_(new NavigationBar(centralWidget_))
    , loadingIndicator_(new LoadingIndicator(centralWidget_))
    , contentWidget_(new QWidget(centralWidget_))
    , stackedLayout_(new QStackedLayout(contentWidget_))
    , webView_(new WebView(contentWidget_))
    , errorPage_(new ErrorPage(contentWidget_))
    , statusLabel_(new QLabel("준비", this))
    , bookmarkPanel_(nullptr)
    , historyPanel_(nullptr)
    , downloadPanel_(nullptr)
    , tabManager_(new TabManager(this))
    , storageService_(new StorageService(this))
    , bookmarkService_(nullptr)
    , historyService_(nullptr)
    , downloadManager_(nullptr)
    , currentUrl_("")
    , currentTitle_("")
    , warningTimer_(new QTimer(this))
    , pendingUrl_()
{
    qDebug() << "BrowserWindow: 생성 중...";

    // 경고 타이머 초기화
    warningTimer_->setSingleShot(true);
    connect(warningTimer_, &QTimer::timeout, this, &BrowserWindow::onWarningTimerTimeout);

    // 스토리지 서비스 초기화
    if (!storageService_->initialize()) {
        Logger::error("BrowserWindow: StorageService 초기화 실패");
    }

    // 북마크 서비스 초기화
    bookmarkService_ = new BookmarkService(storageService_, this);

    // 북마크 패널 생성
    bookmarkPanel_ = new BookmarkPanel(bookmarkService_, centralWidget_);
    bookmarkPanel_->setGeometry(1320, 0, 600, 1080);  // 우측 고정
    bookmarkPanel_->hide();  // 초기 숨김

    // 히스토리 서비스 초기화
    historyService_ = new HistoryService(storageService_, this);

    // 히스토리 패널 생성 (오버레이)
    historyPanel_ = new HistoryPanel(historyService_, this);
    historyPanel_->setGeometry(
        width() - 600, 0,  // 우측 정렬
        600, height()
    );

    // 다운로드 관리자 생성
    downloadManager_ = new DownloadManager(this);

    // 다운로드 패널 생성
    downloadPanel_ = new DownloadPanel(downloadManager_, this);
    downloadPanel_->setGeometry(
        (width() - 800) / 2, height() - 500,  // 중앙 하단
        800, 450
    );
    downloadPanel_->hide();  // 초기 숨김

    // WebView에 다운로드 핸들러 설정
    webView_->setupDownloadHandler(downloadManager_);

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

    // WebView/ErrorPage 컨테이너 설정 (QStackedLayout)
    stackedLayout_->setStackingMode(QStackedLayout::StackOne);
    stackedLayout_->addWidget(webView_);
    stackedLayout_->addWidget(errorPage_);
    stackedLayout_->setCurrentWidget(webView_);  // 기본값: WebView 표시

    // 컨테이너를 메인 레이아웃에 추가 (stretch=1로 남은 공간 모두 차지)
    mainLayout_->addWidget(contentWidget_, 1);

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
    connect(webView_, static_cast<void(WebView::*)(int)>(&WebView::loadProgress),
            loadingIndicator_, &LoadingIndicator::setProgress);
    connect(webView_, &WebView::loadFinished, loadingIndicator_, &LoadingIndicator::finishLoading);

    // WebView 로딩 시작 이벤트
    connect(webView_, &WebView::loadStarted, this, [this]() {
        statusLabel_->setText("로딩 중...");
        setWindowTitle("로딩 중... - webOS Browser");
        qDebug() << "BrowserWindow: 페이지 로딩 시작";
    });

    // WebView 로딩 진행률 이벤트
    connect(webView_, static_cast<void(WebView::*)(int)>(&WebView::loadProgress),
            this, [this](int progress) {
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

    // WebView 에러 이벤트 → ErrorPage 표시
    connect(webView_, &WebView::loadError, this, &BrowserWindow::onLoadError);

    // WebView 타임아웃 이벤트 → ErrorPage 표시
    connect(webView_, &WebView::loadTimeout, this, &BrowserWindow::onLoadTimeout);

    // ErrorPage 시그널 연결
    connect(errorPage_, &ErrorPage::retryRequested, this, &BrowserWindow::onRetryRequested);
    connect(errorPage_, &ErrorPage::homeRequested, this, &BrowserWindow::onHomeRequested);

    // WebView 로딩 성공 시 WebView로 전환 (에러 화면 숨김)
    connect(webView_, &WebView::loadFinished, this, [this](bool success) {
        if (success) {
            stackedLayout_->setCurrentWidget(webView_);
        }
        // 실패 시는 onLoadError에서 처리
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

    // DownloadManager 시그널 연결
    if (downloadManager_) {
        connect(downloadManager_, &DownloadManager::downloadCompleted,
                this, &BrowserWindow::onDownloadCompleted);
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

void BrowserWindow::onBookmarkButtonClicked() {
    if (!bookmarkPanel_) {
        Logger::warning("[BrowserWindow] 북마크 패널이 초기화되지 않았습니다");
        return;
    }

    Logger::info("[BrowserWindow] 북마크 버튼 클릭");
    
    // 현재 페이지 정보 설정
    bookmarkPanel_->setCurrentPage(currentUrl_, currentTitle_);
    
    // 패널 토글
    if (bookmarkPanel_->isVisible()) {
        bookmarkPanel_->hide();
    } else {
        bookmarkPanel_->show();
        bookmarkPanel_->raise();
    }
}

void BrowserWindow::onBookmarkSelected(const QString& url, const QString& title) {
    Logger::info(QString("[BrowserWindow] 북마크 선택: url=%1, title=%2").arg(url, title));
    
    currentUrl_ = url;
    currentTitle_ = title;
    webView_->load(QUrl(url));
}

void BrowserWindow::onUrlChanged(const QString& url) {
    currentUrl_ = url;
    if (bookmarkPanel_) {
        bookmarkPanel_->setCurrentPage(currentUrl_, currentTitle_);
    }

    // [F-14] 보안 등급 분류
    QUrl qurl(url);
    SecurityLevel level = SecurityClassifier::classify(qurl);

    // [F-14] URLBar 보안 아이콘 업데이트
    urlBar_->updateSecurityIndicator(level);

    // [F-14] HTTP 경고 체크
    checkHttpWarning(qurl);
}

void BrowserWindow::onLoadError(const QString &errorString) {
    // 에러 메시지 파싱으로 ErrorType 추론
    ErrorType type = ErrorType::NetworkError;  // 기본값

    if (errorString.contains("timeout", Qt::CaseInsensitive)) {
        type = ErrorType::Timeout;
    } else if (errorString.contains("cors", Qt::CaseInsensitive) ||
               errorString.contains("cross-origin", Qt::CaseInsensitive)) {
        type = ErrorType::CorsError;
    }

    // ErrorInfo 생성
    ErrorInfo errorInfo;
    errorInfo.type = type;
    errorInfo.errorMessage = errorString;
    errorInfo.url = webView_->url();
    errorInfo.timestamp = QDateTime::currentDateTime();

    // ErrorPage 표시
    errorPage_->showError(errorInfo);
    stackedLayout_->setCurrentWidget(errorPage_);

    // URLBar와 StatusLabel 업데이트
    urlBar_->showError(errorString);
    statusLabel_->setText("에러: " + errorString);

    // 로그 기록
    qCritical() << "Page load error:"
                << "type=" << static_cast<int>(type)
                << "message=" << errorString
                << "url=" << errorInfo.url.toString();
}

void BrowserWindow::onLoadTimeout() {
    ErrorInfo errorInfo;
    errorInfo.type = ErrorType::Timeout;
    errorInfo.errorMessage = "페이지 로딩 시간이 초과되었습니다 (30초)";
    errorInfo.url = webView_->url();
    errorInfo.timestamp = QDateTime::currentDateTime();

    // ErrorPage 표시
    errorPage_->showError(errorInfo);
    stackedLayout_->setCurrentWidget(errorPage_);

    // StatusLabel 업데이트
    statusLabel_->setText("타임아웃: 30초 초과");

    // 로그 기록
    qCritical() << "Page load timeout:" << errorInfo.url.toString();
}

void BrowserWindow::onRetryRequested() {
    qDebug() << "BrowserWindow: 재시도 요청";

    // ErrorPage 페이드아웃 애니메이션
    QPropertyAnimation *fadeOut = new QPropertyAnimation(errorPage_, "windowOpacity");
    fadeOut->setDuration(200);
    fadeOut->setStartValue(1.0);
    fadeOut->setEndValue(0.0);
    fadeOut->setEasingCurve(QEasingCurve::InCubic);

    // 애니메이션 완료 후 WebView로 전환 및 재시도
    connect(fadeOut, &QPropertyAnimation::finished, this, [this]() {
        stackedLayout_->setCurrentWidget(webView_);
        webView_->reload();
        errorPage_->hide();
    });

    fadeOut->start(QAbstractAnimation::DeleteWhenStopped);
}

void BrowserWindow::onHomeRequested() {
    qDebug() << "BrowserWindow: 홈으로 이동 요청";

    // ErrorPage 페이드아웃 애니메이션
    QPropertyAnimation *fadeOut = new QPropertyAnimation(errorPage_, "windowOpacity");
    fadeOut->setDuration(200);
    fadeOut->setStartValue(1.0);
    fadeOut->setEndValue(0.0);
    fadeOut->setEasingCurve(QEasingCurve::InCubic);

    // 애니메이션 완료 후 WebView로 전환 및 홈페이지 이동
    connect(fadeOut, &QPropertyAnimation::finished, this, [this]() {
        stackedLayout_->setCurrentWidget(webView_);
        // 홈 URL은 하드코딩 (향후 SettingsService 연동)
        webView_->load(QUrl("https://www.google.com"));
        errorPage_->hide();
    });

    fadeOut->start(QAbstractAnimation::DeleteWhenStopped);
}

// [F-14] HTTPS 보안 표시 기능

void BrowserWindow::checkHttpWarning(const QUrl &url) {
    // Insecure가 아니면 무시
    SecurityLevel level = SecurityClassifier::classify(url);
    if (level != SecurityLevel::Insecure) {
        warningTimer_->stop();
        return;
    }

    // 이미 무시한 도메인이면 무시
    QString host = url.host();
    if (ignoredDomains_.contains(host)) {
        qDebug() << "BrowserWindow: 경고 무시 (도메인:" << host << ")";
        return;
    }

    // 500ms 디바운싱
    pendingUrl_ = url;
    warningTimer_->stop();
    warningTimer_->start(500);  // 500ms 후 onWarningTimerTimeout 호출
}

void BrowserWindow::onWarningTimerTimeout() {
    if (!pendingUrl_.isEmpty() && pendingUrl_.isValid()) {
        showSecurityWarningDialog(pendingUrl_);
        pendingUrl_.clear();
    }
}

bool BrowserWindow::showSecurityWarningDialog(const QUrl &url) {
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("안전하지 않은 사이트");
    msgBox.setText("이 사이트는 보안 연결을 사용하지 않습니다.");
    msgBox.setInformativeText("개인 정보(비밀번호, 신용카드 번호 등)를 입력하지 마세요.");
    msgBox.setIcon(QMessageBox::Warning);

    // 버튼 설정
    QPushButton *continueBtn = msgBox.addButton("계속 진행", QMessageBox::AcceptRole);
    QPushButton *backBtn = msgBox.addButton("돌아가기", QMessageBox::RejectRole);
    msgBox.setDefaultButton(continueBtn);

    // 체크박스 (선택적)
    QCheckBox *dontShowAgain = new QCheckBox("이 세션 동안 이 사이트에 대해 다시 표시하지 않기");
    msgBox.setCheckBox(dontShowAgain);

    // 다이얼로그 표시
    msgBox.exec();

    if (msgBox.clickedButton() == continueBtn) {
        // 체크박스 선택 시 도메인 저장
        if (dontShowAgain->isChecked()) {
            QString host = url.host();
            ignoredDomains_.insert(host);
            qDebug() << "BrowserWindow: 경고 무시 도메인 추가 -" << host;

            // 최대 100개 제한 (메모리 관리)
            if (ignoredDomains_.size() > 100) {
                qWarning() << "BrowserWindow: 경고 무시 도메인 목록 초과 (100개 제한)";
                // 가장 오래된 항목 제거 (QSet은 순서 없음 → 임의 제거)
                ignoredDomains_.erase(ignoredDomains_.begin());
            }
        }
        return true;  // 계속 진행
    } else {
        // 돌아가기
        webView_->goBack();
        return false;
    }
}

// [F-12] 다운로드 관리 기능

void BrowserWindow::showDownloadPanel()
{
    if (downloadPanel_) {
        downloadPanel_->show();
        downloadPanel_->raise();  // 최상단으로
        qDebug() << "BrowserWindow: 다운로드 패널 표시";
    }
}

void BrowserWindow::hideDownloadPanel()
{
    if (downloadPanel_) {
        downloadPanel_->hide();
        qDebug() << "BrowserWindow: 다운로드 패널 숨김";
    }
}

void BrowserWindow::toggleDownloadPanel()
{
    if (downloadPanel_) {
        if (downloadPanel_->isVisible()) {
            hideDownloadPanel();
        } else {
            showDownloadPanel();
        }
    }
}

void BrowserWindow::onDownloadCompleted(const DownloadItem& item)
{
    qDebug() << "BrowserWindow: 다운로드 완료 알림 -" << item.fileName;

    // StatusLabel 업데이트
    statusLabel_->setText("다운로드 완료: " + item.fileName);

    // 토스트 알림은 DownloadPanel에서 처리
    // 여기서는 추가 작업 (예: 상태바 업데이트)만 수행
}

} // namespace webosbrowser
