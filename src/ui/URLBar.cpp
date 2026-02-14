/**
 * @file URLBar.cpp
 * @brief URL 입력 바 위젯 구현
 */

#include "URLBar.h"
#include "SecurityIndicator.h"
#include "../utils/URLValidator.h"
#include "../services/SearchEngine.h"
#include <QDebug>

namespace webosbrowser {

URLBar::URLBar(QWidget *parent)
    : QWidget(parent)
    , securityIndicator_(new SecurityIndicator(this))
    , inputField_(new QLineEdit(this))
    , errorLabel_(new QLabel(this))
    , mainLayout_(new QVBoxLayout(this))
    , inputLayout_(new QHBoxLayout())
    , previousUrl_()
{
    qDebug() << "URLBar: 생성 중...";

    setupUI();
    setupConnections();
    applyStyles();

    qDebug() << "URLBar: 생성 완료";
}

URLBar::~URLBar() {
    qDebug() << "URLBar: 소멸";
}

QString URLBar::text() const {
    return inputField_->text();
}

void URLBar::setText(const QString &url) {
    inputField_->setText(url);
}

void URLBar::setFocusToInput() {
    inputField_->setFocus();
}

void URLBar::showError(const QString &message) {
    qDebug() << "URLBar: 에러 표시 -" << message;
    errorLabel_->setText(message);
    errorLabel_->show();

    // 입력 필드 테두리를 빨간색으로 변경
    inputField_->setStyleSheet(
        "QLineEdit {"
        "    border: 2px solid #ff4444;"
        "    padding: 8px;"
        "    font-size: 18px;"
        "    background: #2a2a2a;"
        "    color: #ffffff;"
        "}"
    );
}

void URLBar::hideError() {
    errorLabel_->hide();
    // 기본 스타일로 복원
    applyStyles();
}

void URLBar::keyPressEvent(QKeyEvent *event) {
    switch (event->key()) {
        case Qt::Key_Enter:
        case Qt::Key_Return:
            qDebug() << "URLBar: Enter 키 입력";
            onReturnPressed();
            break;

        case Qt::Key_Escape:
        case Qt::Key_Back:
            qDebug() << "URLBar: ESC/Back 키 입력 - 입력 취소";
            // 이전 URL 복원
            inputField_->setText(previousUrl_);
            hideError();
            emit editingCancelled();
            break;

        default:
            QWidget::keyPressEvent(event);
            break;
    }
}

void URLBar::focusInEvent(QFocusEvent *event) {
    qDebug() << "URLBar: 포커스 인";
    // 현재 URL 저장 (취소 시 복원용)
    previousUrl_ = inputField_->text();

    // 입력 필드에 포커스 전달
    inputField_->setFocus();

    QWidget::focusInEvent(event);
}

void URLBar::focusOutEvent(QFocusEvent *event) {
    qDebug() << "URLBar: 포커스 아웃";
    hideError();
    QWidget::focusOutEvent(event);
}

void URLBar::onReturnPressed() {
    QString input = inputField_->text().trimmed();

    if (input.isEmpty()) {
        showError("URL을 입력하세요");
        return;
    }

    // URL 검증 및 자동 보완
    QUrl url = validateAndCompleteUrl(input);

    if (url.isValid() && !url.host().isEmpty()) {
        qDebug() << "URLBar: URL 제출 -" << url.toString();
        hideError();
        emit urlSubmitted(url);
    } else {
        showError("유효한 URL을 입력하세요");
    }
}

void URLBar::setupUI() {
    // 메인 레이아웃 설정
    mainLayout_->setContentsMargins(10, 10, 10, 10);
    mainLayout_->setSpacing(5);

    // 입력 필드 레이아웃 (보안 아이콘 + URL 입력 필드)
    inputLayout_->setSpacing(8);
    inputLayout_->addWidget(securityIndicator_);  // 보안 아이콘 (좌측)

    inputField_->setPlaceholderText("URL을 입력하세요...");
    inputField_->setClearButtonEnabled(true);  // X 버튼으로 지우기 가능
    inputLayout_->addWidget(inputField_);
    mainLayout_->addLayout(inputLayout_);

    // 에러 라벨 설정
    errorLabel_->setObjectName("errorLabel");
    errorLabel_->setWordWrap(true);
    errorLabel_->hide();  // 초기에는 숨김
    mainLayout_->addWidget(errorLabel_);

    setLayout(mainLayout_);
}

void URLBar::setupConnections() {
    // QLineEdit의 returnPressed 시그널 연결
    connect(inputField_, &QLineEdit::returnPressed,
            this, &URLBar::onReturnPressed);

    // 텍스트 변경 시 에러 숨김
    connect(inputField_, &QLineEdit::textChanged,
            this, [this]() { hideError(); });
}

void URLBar::applyStyles() {
    // URLBar 전체 스타일
    setStyleSheet(
        "URLBar {"
        "    background: #1a1a1a;"
        "}"
    );

    // 입력 필드 스타일
    inputField_->setStyleSheet(
        "QLineEdit {"
        "    border: 2px solid #555555;"
        "    padding: 8px;"
        "    font-size: 18px;"
        "    background: #2a2a2a;"
        "    color: #ffffff;"
        "    border-radius: 5px;"
        "}"
        "QLineEdit:focus {"
        "    border: 3px solid #00aaff;"
        "}"
    );

    // 에러 라벨 스타일
    errorLabel_->setStyleSheet(
        "QLabel {"
        "    color: #ff4444;"
        "    font-size: 14px;"
        "    padding: 5px;"
        "}"
    );
}

QUrl URLBar::validateAndCompleteUrl(const QString &input) {
    // 1. 검색어인지 확인 (F-09)
    if (URLValidator::isSearchQuery(input)) {
        qDebug() << "URLBar: 검색어로 인식 -" << input;
        QString searchUrl = SearchEngine::buildSearchUrl(input);
        if (!searchUrl.isEmpty()) {
            qDebug() << "URLBar: 검색 URL 생성 -" << searchUrl;
            return QUrl(searchUrl);
        } else {
            // 검색 URL 생성 실패 (빈 검색어)
            return QUrl();
        }
    }

    // 2. URL로 검증 및 자동 보완 (기존 F-03 로직)
    URLValidator::ValidationResult result = URLValidator::validate(input);

    QUrl url;
    switch (result) {
        case URLValidator::ValidationResult::Valid:
            url = QUrl(input);
            break;

        case URLValidator::ValidationResult::MissingScheme:
            // 프로토콜 누락 → 자동 보완
            url = QUrl(URLValidator::normalize(input));
            break;

        case URLValidator::ValidationResult::InvalidFormat:
        case URLValidator::ValidationResult::Empty:
        default:
            return QUrl();  // 빈 QUrl 반환
    }

    // XSS 방어: 위험한 스키마 필터링
    QStringList dangerousSchemes = {"javascript", "data", "vbscript", "file"};
    if (dangerousSchemes.contains(url.scheme().toLower())) {
        showError("허용되지 않는 URL 형식입니다");
        return QUrl();
    }

    return url;
}

void URLBar::updateSecurityIndicator(SecurityLevel level) {
    securityIndicator_->setSecurityLevel(level);
}

} // namespace webosbrowser
