/**
 * @file TabManager.cpp
 * @brief 탭 관리자 구현
 */

#include "TabManager.h"
#include "WebView.h"
#include <QDebug>

namespace webosbrowser {

TabManager::TabManager(QObject *parent)
    : QObject(parent)
    , currentTab_(nullptr)
{
    qDebug() << "TabManager: 생성 (단일 탭 모드)";
}

TabManager::~TabManager() {
    qDebug() << "TabManager: 소멸";
}

void TabManager::setCurrentTab(WebView *webView) {
    // 중복 신호 방지: 이미 동일한 WebView가 설정되어 있으면 무시
    // (성능 최적화 + 무한 루프 방지)
    if (currentTab_ != webView) {
        currentTab_ = webView;
        qDebug() << "TabManager: 현재 탭 설정";
        emit tabChanged(0);  // 단일 탭이므로 항상 인덱스 0
    } else {
        qDebug() << "TabManager: 같은 탭 재설정 시도 - 신호 무시";
    }
}

WebView* TabManager::getCurrentTab() const {
    return currentTab_;
}

int TabManager::getTabCount() const {
    return currentTab_ ? 1 : 0;  // 단일 탭: 0 또는 1
}

QString TabManager::getCurrentTabTitle() const {
    if (currentTab_) {
        return currentTab_->title();
    }
    return QString();
}

QUrl TabManager::getCurrentTabUrl() const {
    if (currentTab_) {
        return currentTab_->url();
    }
    return QUrl();
}

} // namespace webosbrowser
