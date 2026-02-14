/**
 * @file ErrorPage.cpp
 * @brief 에러 화면 위젯 구현
 */

#include "ErrorPage.h"
#include <QHBoxLayout>
#include <QPropertyAnimation>
#include <QKeyEvent>
#include <QShowEvent>
#include <QDebug>

namespace webosbrowser {

ErrorPage::ErrorPage(QWidget *parent)
    : QWidget(parent)
    , mainLayout_(new QVBoxLayout(this))
    , iconLabel_(new QLabel(this))
    , titleLabel_(new QLabel(this))
    , messageLabel_(new QLabel(this))
    , urlLabel_(new QLabel(this))
    , retryButton_(new QPushButton("재시도", this))
    , homeButton_(new QPushButton("홈으로 이동", this))
{
    qDebug() << "ErrorPage: 생성 중...";

    setupUI();
    applyStyles();
    setupConnections();

    qDebug() << "ErrorPage: 생성 완료";
}

ErrorPage::~ErrorPage() {
    qDebug() << "ErrorPage: 소멸";
}

void ErrorPage::setupUI() {
    // 메인 레이아웃 설정
    mainLayout_->setAlignment(Qt::AlignCenter);
    mainLayout_->setSpacing(20);
    mainLayout_->setContentsMargins(50, 50, 50, 50);

    // 아이콘 레이블 설정
    iconLabel_->setAlignment(Qt::AlignCenter);
    iconLabel_->setFixedSize(80, 80);
    iconLabel_->setText("⚠");  // 임시 텍스트 아이콘 (향후 이미지로 교체)

    // 제목 레이블 설정
    titleLabel_->setAlignment(Qt::AlignCenter);
    titleLabel_->setWordWrap(false);

    // 메시지 레이블 설정
    messageLabel_->setAlignment(Qt::AlignCenter);
    messageLabel_->setWordWrap(true);
    messageLabel_->setMinimumWidth(600);

    // URL 레이블 설정
    urlLabel_->setAlignment(Qt::AlignCenter);
    urlLabel_->setWordWrap(false);
    urlLabel_->setTextFormat(Qt::PlainText);  // XSS 방지

    // 버튼 레이아웃 설정
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(30);
    buttonLayout->setAlignment(Qt::AlignCenter);

    retryButton_->setMinimumSize(200, 60);
    homeButton_->setMinimumSize(200, 60);

    buttonLayout->addWidget(retryButton_);
    buttonLayout->addWidget(homeButton_);

    // 위젯들을 메인 레이아웃에 추가
    mainLayout_->addWidget(iconLabel_);
    mainLayout_->addWidget(titleLabel_);
    mainLayout_->addWidget(messageLabel_);
    mainLayout_->addWidget(urlLabel_);
    mainLayout_->addSpacing(30);
    mainLayout_->addLayout(buttonLayout);

    // 탭 오더 설정 (리모컨 좌우 방향키)
    setTabOrder(retryButton_, homeButton_);
    setTabOrder(homeButton_, retryButton_);  // 순환

    qDebug() << "ErrorPage: UI 초기화 완료";
}

void ErrorPage::applyStyles() {
    // 배경 스타일
    setStyleSheet(R"(
        ErrorPage {
            background-color: rgba(0, 0, 0, 230);
        }
    )");

    // 아이콘 스타일
    iconLabel_->setStyleSheet(R"(
        QLabel {
            font-size: 80px;
            color: #FFA500;
        }
    )");

    // 제목 스타일
    titleLabel_->setStyleSheet(R"(
        QLabel {
            font-size: 48px;
            font-weight: bold;
            color: #FFFFFF;
        }
    )");

    // 메시지 스타일
    messageLabel_->setStyleSheet(R"(
        QLabel {
            font-size: 28px;
            color: #FFFFFF;
        }
    )");

    // URL 스타일
    urlLabel_->setStyleSheet(R"(
        QLabel {
            font-size: 22px;
            color: #999999;
        }
    )");

    // 버튼 공통 스타일
    QString buttonStyle = R"(
        QPushButton {
            background-color: #1E90FF;
            color: #FFFFFF;
            font-size: 24px;
            border: 2px solid #1E90FF;
            border-radius: 8px;
            padding: 10px 20px;
            min-width: 200px;
            min-height: 60px;
        }
        QPushButton:hover {
            background-color: #4169E1;
        }
        QPushButton:pressed {
            background-color: #104E8B;
        }
        QPushButton:focus {
            border: 4px solid #FFD700;
        }
    )";

    retryButton_->setStyleSheet(buttonStyle);
    homeButton_->setStyleSheet(buttonStyle);

    // 포커스 정책 설정 (리모컨 포커스 필수)
    retryButton_->setFocusPolicy(Qt::StrongFocus);
    homeButton_->setFocusPolicy(Qt::StrongFocus);

    qDebug() << "ErrorPage: 스타일 적용 완료";
}

void ErrorPage::setupConnections() {
    // 재시도 버튼 클릭 시그널 연결
    connect(retryButton_, &QPushButton::clicked, this, &ErrorPage::retryRequested);

    // 홈 버튼 클릭 시그널 연결
    connect(homeButton_, &QPushButton::clicked, this, &ErrorPage::homeRequested);

    qDebug() << "ErrorPage: 시그널/슬롯 연결 완료";
}

void ErrorPage::showError(ErrorType type, const QString &errorMessage, const QUrl &url) {
    // 에러 정보 저장
    currentError_.type = type;
    currentError_.errorMessage = errorMessage;
    currentError_.url = url;
    currentError_.timestamp = QDateTime::currentDateTime();

    // 에러 타입별 제목 설정
    QString title;
    switch (type) {
        case ErrorType::NetworkError:
            title = "네트워크 연결 실패";
            break;

        case ErrorType::Timeout:
            title = "로딩 시간 초과";
            break;

        case ErrorType::CorsError:
            title = "보안 정책 오류";
            break;

        case ErrorType::UnknownError:
        default:
            title = "페이지 로딩 오류";
            break;
    }

    // UI 업데이트
    iconLabel_->setText(getErrorIcon(type));
    titleLabel_->setText(title);
    messageLabel_->setText(errorMessage);
    urlLabel_->setText(truncateUrl(url));

    // 재시도 버튼에 자동 포커스
    retryButton_->setFocus(Qt::OtherFocusReason);

    qDebug() << "ErrorPage: 에러 표시 -" << title << ":" << errorMessage;

    // 위젯 표시 (showEvent에서 페이드인 애니메이션 자동 실행)
    show();
}

void ErrorPage::showError(const ErrorInfo &errorInfo) {
    showError(errorInfo.type, errorInfo.errorMessage, errorInfo.url);
}

void ErrorPage::keyPressEvent(QKeyEvent *event) {
    // Back 키 무시 (에러 화면에서 벗어나지 못하도록)
    if (event->key() == Qt::Key_Escape || event->key() == Qt::Key_Back) {
        qDebug() << "ErrorPage: Back 키 무시";
        event->ignore();
        return;
    }

    // 나머지 키는 기본 동작 (Qt 탭 오더 시스템)
    QWidget::keyPressEvent(event);
}

void ErrorPage::showEvent(QShowEvent *event) {
    QWidget::showEvent(event);

    // 페이드인 애니메이션 시작
    startFadeInAnimation();
}

QString ErrorPage::getErrorIcon(ErrorType type) const {
    // 현재는 공통 아이콘 사용
    // 향후 에러 타입별 아이콘 분리 가능
    Q_UNUSED(type)

    // 텍스트 아이콘 반환 (향후 QPixmap으로 교체 가능)
    return "⚠";
}

QString ErrorPage::truncateUrl(const QUrl &url) const {
    QString urlString = url.toString();
    if (urlString.length() > 50) {
        return urlString.left(50) + "...";
    }
    return urlString;
}

void ErrorPage::startFadeInAnimation() {
    // QPropertyAnimation으로 페이드인 효과
    // 주의: windowOpacity는 최상위 윈도우에서만 동작할 수 있음
    // QStackedLayout 내부에서는 동작하지 않을 수 있으므로 조건부 실행

    qDebug() << "ErrorPage: 페이드인 애니메이션 시작";

    // windowOpacity 애니메이션 (동작하지 않을 수 있음)
    QPropertyAnimation *fadeIn = new QPropertyAnimation(this, "windowOpacity");
    fadeIn->setDuration(300);
    fadeIn->setStartValue(0.0);
    fadeIn->setEndValue(1.0);
    fadeIn->setEasingCurve(QEasingCurve::OutCubic);

    // 애니메이션 완료 후 자동 삭제
    fadeIn->start(QAbstractAnimation::DeleteWhenStopped);
}

} // namespace webosbrowser
