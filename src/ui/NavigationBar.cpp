/**
 * @file NavigationBar.cpp
 * @brief 네비게이션 바 구현
 */

#include "NavigationBar.h"
#include "../browser/WebView.h"
#include <QDebug>

namespace webosbrowser {

NavigationBar::NavigationBar(QWidget *parent)
    : QWidget(parent)
    , layout_(nullptr)
    , backButton_(nullptr)
    , forwardButton_(nullptr)
    , reloadButton_(nullptr)
    , homeButton_(nullptr)
    , bookmarkButton_(nullptr)
    , webView_(nullptr)
{
    qDebug() << "NavigationBar: 생성 중...";

    setupUI();
    setupConnections();
    applyStyles();
    setupFocusOrder();

    qDebug() << "NavigationBar: 생성 완료";
}

NavigationBar::~NavigationBar() {
    qDebug() << "NavigationBar: 소멸";
}

void NavigationBar::setWebView(WebView *webView) {
    // 기존 연결 해제
    if (webView_) {
        disconnect(webView_, nullptr, this, nullptr);
    }

    webView_ = webView;

    // 새 WebView 연결
    if (webView_) {
        qDebug() << "NavigationBar: WebView 연결";
        connect(webView_, &WebView::urlChanged, this, &NavigationBar::updateButtonStates);
        connect(webView_, &WebView::loadStarted, this, &NavigationBar::updateButtonStates);
        connect(webView_, &WebView::loadFinished, this, &NavigationBar::updateButtonStates);

        // 초기 상태 업데이트
        updateButtonStates();
    } else {
        qDebug() << "NavigationBar: WebView 연결 해제";
        backButton_->setEnabled(false);
        forwardButton_->setEnabled(false);
    }
}

void NavigationBar::updateButtonStates() {
    if (!webView_) {
        return;
    }

    // 뒤로/앞으로 버튼 상태 업데이트
    backButton_->setEnabled(webView_->canGoBack());
    forwardButton_->setEnabled(webView_->canGoForward());

    qDebug() << "NavigationBar: 버튼 상태 업데이트 - Back:" << webView_->canGoBack()
             << ", Forward:" << webView_->canGoForward();
}

void NavigationBar::onBackClicked() {
    qDebug() << "NavigationBar: 뒤로 버튼 클릭";
    if (webView_) {
        webView_->goBack();
    }
}

void NavigationBar::onForwardClicked() {
    qDebug() << "NavigationBar: 앞으로 버튼 클릭";
    if (webView_) {
        webView_->goForward();
    }
}

void NavigationBar::onReloadClicked() {
    qDebug() << "NavigationBar: 새로고침 버튼 클릭";
    if (webView_) {
        webView_->reload();
    }
}

void NavigationBar::onHomeClicked() {
    qDebug() << "NavigationBar: 홈 버튼 클릭";
    if (webView_) {
        webView_->load(DEFAULT_HOME_URL);
    }
}

void NavigationBar::onBookmarkClicked() {
    qDebug() << "NavigationBar: 북마크 버튼 클릭";
    emit bookmarkButtonClicked();
}

void NavigationBar::setupUI() {
    // 레이아웃 생성
    layout_ = new QHBoxLayout(this);
    layout_->setContentsMargins(10, 10, 10, 10);
    layout_->setSpacing(20);  // 버튼 간격 20px

    // 버튼 생성 (유니코드 심볼 사용)
    backButton_ = new QPushButton("←", this);     // 뒤로 (U+2190)
    forwardButton_ = new QPushButton("→", this);  // 앞으로 (U+2192)
    reloadButton_ = new QPushButton("↻", this);   // 새로고침 (U+21BB)
    homeButton_ = new QPushButton("⌂", this);     // 홈 (U+2302)
    bookmarkButton_ = new QPushButton("★", this); // 북마크 (U+2605)

    // 버튼 크기 설정 (100x80px 이상)
    QSize minSize(100, 80);
    backButton_->setMinimumSize(minSize);
    forwardButton_->setMinimumSize(minSize);
    reloadButton_->setMinimumSize(minSize);
    homeButton_->setMinimumSize(minSize);
    bookmarkButton_->setMinimumSize(minSize);

    // 포커스 정책 설정 (리모컨 포커스 네비게이션)
    backButton_->setFocusPolicy(Qt::StrongFocus);
    forwardButton_->setFocusPolicy(Qt::StrongFocus);
    reloadButton_->setFocusPolicy(Qt::StrongFocus);
    homeButton_->setFocusPolicy(Qt::StrongFocus);
    bookmarkButton_->setFocusPolicy(Qt::StrongFocus);

    // 접근성 설정
    backButton_->setAccessibleName("뒤로 가기");
    backButton_->setAccessibleDescription("이전 페이지로 이동합니다");
    forwardButton_->setAccessibleName("앞으로 가기");
    forwardButton_->setAccessibleDescription("다음 페이지로 이동합니다");
    reloadButton_->setAccessibleName("새로고침");
    reloadButton_->setAccessibleDescription("현재 페이지를 다시 로드합니다");
    homeButton_->setAccessibleName("홈");
    homeButton_->setAccessibleDescription("홈페이지로 이동합니다");
    bookmarkButton_->setAccessibleName("북마크");
    bookmarkButton_->setAccessibleDescription("북마크 관리 패널을 엽니다");

    // 레이아웃에 버튼 추가
    layout_->addWidget(backButton_);
    layout_->addWidget(forwardButton_);
    layout_->addWidget(reloadButton_);
    layout_->addWidget(homeButton_);
    layout_->addWidget(bookmarkButton_);
    layout_->addStretch();  // 우측 공간 확보

    // 초기 상태: WebView 없으므로 뒤로/앞으로 비활성
    backButton_->setEnabled(false);
    forwardButton_->setEnabled(false);
}

void NavigationBar::setupConnections() {
    // 버튼 클릭 시그널 연결
    connect(backButton_, &QPushButton::clicked, this, &NavigationBar::onBackClicked);
    connect(forwardButton_, &QPushButton::clicked, this, &NavigationBar::onForwardClicked);
    connect(reloadButton_, &QPushButton::clicked, this, &NavigationBar::onReloadClicked);
    connect(homeButton_, &QPushButton::clicked, this, &NavigationBar::onHomeClicked);
    connect(bookmarkButton_, &QPushButton::clicked, this, &NavigationBar::onBookmarkClicked);
}

void NavigationBar::applyStyles() {
    // QSS 스타일 정의 (리모컨 최적화 UX)
    QString styleSheet = R"(
        QPushButton {
            min-width: 100px;
            min-height: 80px;
            font-size: 32pt;          /* 아이콘 크기 (유니코드) */
            background-color: #333333; /* 어두운 배경 */
            color: white;
            border: 2px solid transparent;
            border-radius: 5px;
        }
        QPushButton:focus {
            border: 3px solid #00aaff;  /* 포커스 시 파란 테두리 */
        }
        QPushButton:pressed {
            background-color: #1a1a1a;  /* 클릭 시 더 어두운 배경 */
        }
        QPushButton:disabled {
            opacity: 0.5;
            color: #888888;
        }
    )";

    setStyleSheet(styleSheet);
}

void NavigationBar::setupFocusOrder() {
    // 리모컨 좌/우 방향키로 포커스 이동 (Back → Forward → Reload → Home → Bookmark)
    setTabOrder(backButton_, forwardButton_);
    setTabOrder(forwardButton_, reloadButton_);
    setTabOrder(reloadButton_, homeButton_);
    setTabOrder(homeButton_, bookmarkButton_);
}

} // namespace webosbrowser
