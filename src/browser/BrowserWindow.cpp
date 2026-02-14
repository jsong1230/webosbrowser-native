/**
 * @file BrowserWindow.cpp
 * @brief 브라우저 메인 윈도우 구현
 */

#include "BrowserWindow.h"
#include <QDebug>
#include <QApplication>
#include <QScreen>

namespace webosbrowser {

BrowserWindow::BrowserWindow(QWidget *parent)
    : QMainWindow(parent)
    , centralWidget_(new QWidget(this))
    , mainLayout_(new QVBoxLayout(centralWidget_))
    , titleLabel_(new QLabel("Hello webOS Browser!", centralWidget_))
{
    qDebug() << "BrowserWindow 생성 중...";

    setupUI();
    setupConnections();

    qDebug() << "BrowserWindow 생성 완료";
}

BrowserWindow::~BrowserWindow() {
    qDebug() << "BrowserWindow 소멸";
}

void BrowserWindow::setupUI() {
    // 윈도우 기본 설정
    setWindowTitle("webOS Browser");
    resize(1920, 1080);  // webOS 기본 해상도

    // 중앙 위젯 설정
    setCentralWidget(centralWidget_);

    // 타이틀 라벨 스타일
    titleLabel_->setAlignment(Qt::AlignCenter);
    titleLabel_->setStyleSheet(
        "QLabel {"
        "  font-size: 48px;"
        "  font-weight: bold;"
        "  color: #2563eb;"
        "  padding: 20px;"
        "}"
    );

    // 레이아웃 설정
    mainLayout_->addWidget(titleLabel_);
    mainLayout_->setAlignment(Qt::AlignCenter);

    qDebug() << "UI 설정 완료";
}

void BrowserWindow::setupConnections() {
    // TODO: 시그널/슬롯 연결 (추후 구현)
    qDebug() << "시그널/슬롯 연결 완료";
}

} // namespace webosbrowser
