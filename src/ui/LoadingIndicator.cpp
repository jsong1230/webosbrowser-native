/**
 * @file LoadingIndicator.cpp
 * @brief 로딩 인디케이터 구현
 */

#include "LoadingIndicator.h"
#include <QVBoxLayout>
#include <QTimer>
#include <QDebug>

namespace webosbrowser {

LoadingIndicator::LoadingIndicator(QWidget *parent)
    : QWidget(parent)
    , progressBar_(new QProgressBar(this))
{
    qDebug() << "LoadingIndicator: 생성 중...";

    setupUI();
    applyStyles();

    // 초기에는 숨김
    hide();

    qDebug() << "LoadingIndicator: 생성 완료";
}

LoadingIndicator::~LoadingIndicator() {
    qDebug() << "LoadingIndicator: 소멸";
}

void LoadingIndicator::setProgress(int progress) {
    progressBar_->setValue(progress);
    qDebug() << "LoadingIndicator: 진행률 -" << progress << "%";
}

void LoadingIndicator::startLoading() {
    qDebug() << "LoadingIndicator: 로딩 시작";
    progressBar_->setValue(0);
    show();
}

void LoadingIndicator::finishLoading(bool success) {
    qDebug() << "LoadingIndicator: 로딩 완료 - 성공:" << success;

    if (success) {
        progressBar_->setValue(100);
    }

    // 0.5초 후 숨김
    QTimer::singleShot(500, this, &LoadingIndicator::hide);
}

void LoadingIndicator::setupUI() {
    // 레이아웃 생성
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // 프로그레스바 설정
    progressBar_->setMinimum(0);
    progressBar_->setMaximum(100);
    progressBar_->setValue(0);
    progressBar_->setTextVisible(false);  // 텍스트 숨김 (시각적으로 깔끔)
    progressBar_->setMaximumHeight(5);    // 얇은 프로그레스바

    layout->addWidget(progressBar_);
    setLayout(layout);

    // 전체 높이를 프로그레스바 높이로 제한
    setMaximumHeight(5);
}

void LoadingIndicator::applyStyles() {
    // QSS 스타일 정의
    QString styleSheet = R"(
        QProgressBar {
            border: none;
            background-color: #2a2a2a;
            border-radius: 0px;
        }
        QProgressBar::chunk {
            background-color: #00aaff;  /* 파란색 진행 바 */
            border-radius: 0px;
        }
    )";

    progressBar_->setStyleSheet(styleSheet);
}

} // namespace webosbrowser
