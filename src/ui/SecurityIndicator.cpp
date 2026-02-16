/**
 * @file SecurityIndicator.cpp
 * @brief 보안 상태 아이콘 표시 위젯 구현
 */

#include "SecurityIndicator.h"
#include <QToolTip>
#include <QDebug>

namespace webosbrowser {

SecurityIndicator::SecurityIndicator(QWidget *parent)
    : QLabel(parent)
    , currentLevel_(SecurityLevel::Unknown)
{
    setupUI();
    applyStyles();
    loadIconCache();
}

SecurityIndicator::~SecurityIndicator() = default;

void SecurityIndicator::setupUI() {
    // 고정 크기 설정 (32x32px 아이콘)
    setFixedSize(40, 40);

    // 정렬 설정
    setAlignment(Qt::AlignCenter);

    // 포커스 정책 설정 (리모컨 접근 가능)
    setFocusPolicy(Qt::StrongFocus);

    // 초기 상태: 아이콘 없음
    clear();
}

void SecurityIndicator::applyStyles() {
    // QSS 스타일 적용
    setStyleSheet(
        "QLabel {"
        "    border: 2px solid transparent;"
        "    border-radius: 4px;"
        "    padding: 4px;"
        "}"
        "QLabel:focus {"
        "    border: 2px solid #00aaff;"  // 포커스 테두리
        "}"
    );
}

void SecurityIndicator::loadIconCache() {
    // 텍스트 기반 아이콘 사용 (PNG 리소스 불필요)
    // 아이콘 파일 로드 시도 (있으면 사용, 없으면 텍스트로 fallback)
    secureIcon_ = QPixmap(":/icons/lock_secure.png");
    insecureIcon_ = QPixmap(":/icons/lock_insecure.png");
    localIcon_ = QPixmap(":/icons/info.png");

    // 아이콘 크기 조정 (32x32px)
    if (!secureIcon_.isNull()) {
        secureIcon_ = secureIcon_.scaled(32, 32, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
    if (!insecureIcon_.isNull()) {
        insecureIcon_ = insecureIcon_.scaled(32, 32, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
    if (!localIcon_.isNull()) {
        localIcon_ = localIcon_.scaled(32, 32, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    qDebug() << "SecurityIndicator: 텍스트 기반 아이콘 모드 준비 완료";
}

void SecurityIndicator::setSecurityLevel(SecurityLevel level) {
    currentLevel_ = level;

    // 아이콘 변경
    switch (level) {
        case SecurityLevel::Secure:
            if (!secureIcon_.isNull()) {
                setPixmap(secureIcon_);
            } else {
                // 텍스트 기반 fallback
                setText("🔒");
                setStyleSheet(styleSheet() + "QLabel { color: #4caf50; font-size: 24px; }");
            }
            setToolTip(SecurityClassifier::securityLevelToTooltip(level));
            break;
        case SecurityLevel::Insecure:
            if (!insecureIcon_.isNull()) {
                setPixmap(insecureIcon_);
            } else {
                // 텍스트 기반 fallback
                setText("⚠️");
                setStyleSheet(styleSheet() + "QLabel { color: #ff9800; font-size: 24px; }");
            }
            setToolTip(SecurityClassifier::securityLevelToTooltip(level));
            break;
        case SecurityLevel::Local:
            if (!localIcon_.isNull()) {
                setPixmap(localIcon_);
            } else {
                // 텍스트 기반 fallback
                setText("ℹ️");
                setStyleSheet(styleSheet() + "QLabel { color: #2196f3; font-size: 24px; }");
            }
            setToolTip(SecurityClassifier::securityLevelToTooltip(level));
            break;
        case SecurityLevel::Unknown:
            clear();  // 아이콘 숨김
            setToolTip("");
            break;
    }

    qDebug() << "SecurityIndicator: 보안 등급 변경 -"
             << SecurityClassifier::securityLevelToString(level);
}

SecurityLevel SecurityIndicator::securityLevel() const {
    return currentLevel_;
}

void SecurityIndicator::mousePressEvent(QMouseEvent *event) {
    Q_UNUSED(event);
    // 향후 FR-5 (상세 정보 다이얼로그) 구현 시 사용
    emit clicked();
}

void SecurityIndicator::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        // 향후 FR-5 (상세 정보 다이얼로그) 구현 시 사용
        emit clicked();
        event->accept();
        return;
    }
    QLabel::keyPressEvent(event);
}

void SecurityIndicator::focusInEvent(QFocusEvent *event) {
    QLabel::focusInEvent(event);

    // 포커스 시 툴팁 자동 표시
    if (!toolTip().isEmpty()) {
        QToolTip::showText(mapToGlobal(rect().bottomLeft()), toolTip(), this);
    }
}

} // namespace webosbrowser
