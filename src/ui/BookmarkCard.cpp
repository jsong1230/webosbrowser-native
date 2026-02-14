/**
 * @file BookmarkCard.cpp
 * @brief 홈 화면의 북마크 카드 UI 구현
 */

#include "BookmarkCard.h"
#include <QDebug>
#include <QPixmap>
#include <QPainter>
#include <Qt>

namespace webosbrowser {

BookmarkCard::BookmarkCard(const QString &id, const QString &title,
                           const QString &url, QWidget *parent)
    : QWidget(parent)
    , id_(id)
    , title_(title)
    , url_(url)
    , layout_(nullptr)
    , faviconLabel_(nullptr)
    , titleLabel_(nullptr)
    , isFocused_(false)
{
    qDebug() << "BookmarkCard: 생성 - ID:" << id_ << "Title:" << title_;

    setupUI();
}

BookmarkCard::~BookmarkCard() {
    qDebug() << "BookmarkCard: 소멸 - ID:" << id_;
}

void BookmarkCard::setupUI() {
    // 메인 레이아웃
    layout_ = new QVBoxLayout(this);
    layout_->setContentsMargins(12, 12, 12, 12);
    layout_->setSpacing(8);
    layout_->setAlignment(Qt::AlignCenter);

    // 파비콘 (기본 아이콘)
    faviconLabel_ = new QLabel(this);

    // 기본 파비콘 생성 (64x64 회색 원)
    QPixmap faviconPixmap(64, 64);
    faviconPixmap.fill(Qt::transparent);

    // 간단한 회색 원 그리기
    QPainter painter(&faviconPixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(QBrush(QColor(100, 100, 100)));
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(8, 8, 48, 48);

    faviconLabel_->setPixmap(faviconPixmap);
    faviconLabel_->setFixedSize(64, 64);
    faviconLabel_->setAlignment(Qt::AlignCenter);
    layout_->addWidget(faviconLabel_);

    // 제목 (말줄임표 처리)
    QString displayTitle = elideText(title_, 20);
    titleLabel_ = new QLabel(displayTitle, this);
    titleLabel_->setStyleSheet("font-size: 20px; font-weight: bold; color: #FFFFFF;");
    titleLabel_->setAlignment(Qt::AlignCenter);
    titleLabel_->setWordWrap(false);
    layout_->addWidget(titleLabel_);

    // 카드 크기 설정
    setFixedSize(200, 180);

    // 포커스 정책
    setFocusPolicy(Qt::StrongFocus);

    // 초기 스타일 적용
    updateStyle(false);
}

void BookmarkCard::updateStyle(bool focused) {
    QString styleSheet;

    if (focused) {
        styleSheet = R"(
            BookmarkCard {
                border: 2px solid #0078D7;
                border-radius: 8px;
                padding: 10px;
                background-color: #3A3A3A;
            }
        )";
    } else {
        styleSheet = R"(
            BookmarkCard {
                border: 2px solid transparent;
                border-radius: 8px;
                padding: 10px;
                background-color: #2A2A2A;
            }
        )";
    }

    setStyleSheet(styleSheet);
}

QString BookmarkCard::elideText(const QString &text, int maxLength) {
    if (text.length() <= maxLength) {
        return text;
    }
    return text.left(maxLength - 3) + "...";
}

void BookmarkCard::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        qDebug() << "BookmarkCard: 마우스 클릭 - URL:" << url_;
        emit clicked(url_);
    }
    QWidget::mousePressEvent(event);
}

void BookmarkCard::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        qDebug() << "BookmarkCard: Enter 키 - URL:" << url_;
        emit clicked(url_);
        event->accept();
    } else {
        QWidget::keyPressEvent(event);
    }
}

void BookmarkCard::focusInEvent(QFocusEvent *event) {
    isFocused_ = true;
    updateStyle(true);
    qDebug() << "BookmarkCard: 포커스 인 - ID:" << id_;
    QWidget::focusInEvent(event);
}

void BookmarkCard::focusOutEvent(QFocusEvent *event) {
    isFocused_ = false;
    updateStyle(false);
    qDebug() << "BookmarkCard: 포커스 아웃 - ID:" << id_;
    QWidget::focusOutEvent(event);
}

} // namespace webosbrowser
