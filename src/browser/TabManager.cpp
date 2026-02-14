/**
 * @file TabManager.cpp
 * @brief 탭 관리자 구현 (다중 탭 지원)
 */

#include "TabManager.h"
#include "WebView.h"
#include "../utils/Logger.h"
#include <QDebug>

namespace webosbrowser {

TabManager::TabManager(QObject *parent)
    : QObject(parent)
    , tabs_()
    , currentTabIndex_(-1)
    , webView_(nullptr)
{
    Logger::debug("[TabManager] 생성 (다중 탭 모드, 최대 5개)");
}

TabManager::~TabManager() {
    Logger::debug("[TabManager] 소멸");
}

void TabManager::setCurrentTab(WebView *webView) {
    webView_ = webView;
    Logger::debug("[TabManager] WebView 설정");

    // 하위 호환성: 탭이 없으면 기본 탭 생성
    if (tabs_.isEmpty() && webView_) {
        TabData defaultTab;
        defaultTab.url = "https://www.google.com";
        defaultTab.title = "새 탭";
        tabs_.append(defaultTab);
        currentTabIndex_ = 0;

        // 기본 URL 로드
        webView_->load(QUrl(defaultTab.url));

        Logger::info("[TabManager] 기본 탭 생성 완료");
        emit tabCreated(0);
        emit tabSwitched(0, defaultTab.url, defaultTab.title);
        emit tabChanged(0);  // 하위 호환성
    }
}

WebView* TabManager::getCurrentTab() const {
    return webView_;
}

int TabManager::getTabCount() const {
    return tabs_.size();
}

int TabManager::getCurrentTabIndex() const {
    return currentTabIndex_;
}

QString TabManager::getCurrentTabTitle() const {
    if (currentTabIndex_ >= 0 && currentTabIndex_ < tabs_.size()) {
        return tabs_[currentTabIndex_].title;
    }
    return QString();
}

QUrl TabManager::getCurrentTabUrl() const {
    if (currentTabIndex_ >= 0 && currentTabIndex_ < tabs_.size()) {
        return QUrl(tabs_[currentTabIndex_].url);
    }
    return QUrl();
}

bool TabManager::createTab(const QString& url) {
    // 최대 탭 수 체크
    if (tabs_.size() >= MAX_TABS) {
        Logger::warning("[TabManager] 새 탭 생성 실패: 최대 5개 제한");
        return false;
    }

    // WebView 확인
    if (!webView_) {
        Logger::error("[TabManager] WebView가 설정되지 않음");
        return false;
    }

    // 현재 탭 상태 저장
    if (currentTabIndex_ >= 0) {
        saveCurrentTabState();
    }

    // 새 탭 생성
    TabData newTab;
    newTab.url = url;
    newTab.title = "새 탭";
    tabs_.append(newTab);

    int newIndex = tabs_.size() - 1;
    currentTabIndex_ = newIndex;

    // WebView에 새 URL 로드
    webView_->load(QUrl(url));

    Logger::info(QString("[TabManager] 새 탭 생성 완료: index=%1, url=%2")
                     .arg(newIndex)
                     .arg(url));

    emit tabCreated(newIndex);
    emit tabSwitched(newIndex, url, newTab.title);
    emit tabChanged(newIndex);  // 하위 호환성

    return true;
}

bool TabManager::switchToTab(int index) {
    // 유효성 검사
    if (index < 0 || index >= tabs_.size()) {
        Logger::warning(QString("[TabManager] 잘못된 탭 인덱스: %1").arg(index));
        return false;
    }

    // 이미 활성화된 탭
    if (index == currentTabIndex_) {
        Logger::debug(QString("[TabManager] 이미 활성화된 탭: %1").arg(index));
        return false;
    }

    // WebView 확인
    if (!webView_) {
        Logger::error("[TabManager] WebView가 설정되지 않음");
        return false;
    }

    // 현재 탭 상태 저장
    if (currentTabIndex_ >= 0) {
        saveCurrentTabState();
    }

    // 새 탭으로 전환
    currentTabIndex_ = index;
    restoreTabState(index);

    const TabData& tab = tabs_[index];
    Logger::info(QString("[TabManager] 탭 전환 완료: index=%1, url=%2")
                     .arg(index)
                     .arg(tab.url));

    emit tabSwitched(index, tab.url, tab.title);
    emit tabChanged(index);  // 하위 호환성

    return true;
}

bool TabManager::previousTab() {
    if (tabs_.isEmpty()) {
        return false;
    }

    int newIndex = currentTabIndex_ - 1;
    if (newIndex < 0) {
        newIndex = tabs_.size() - 1;  // 순환: 첫 탭 → 마지막 탭
    }

    return switchToTab(newIndex);
}

bool TabManager::nextTab() {
    if (tabs_.isEmpty()) {
        return false;
    }

    int newIndex = currentTabIndex_ + 1;
    if (newIndex >= tabs_.size()) {
        newIndex = 0;  // 순환: 마지막 탭 → 첫 탭
    }

    return switchToTab(newIndex);
}

bool TabManager::closeTab(int index) {
    // 유효성 검사
    if (index < 0 || index >= tabs_.size()) {
        Logger::warning(QString("[TabManager] 잘못된 탭 인덱스: %1").arg(index));
        return false;
    }

    // 탭 제거
    tabs_.removeAt(index);
    Logger::info(QString("[TabManager] 탭 닫힘: index=%1").arg(index));

    emit tabClosed(index);

    // 현재 탭 인덱스 조정
    if (tabs_.isEmpty()) {
        currentTabIndex_ = -1;
        if (webView_) {
            webView_->load(QUrl("about:blank"));
        }
    } else {
        // 닫은 탭이 현재 탭이었으면 다음 탭으로 이동
        if (index == currentTabIndex_) {
            // 마지막 탭을 닫았으면 이전 탭으로
            if (currentTabIndex_ >= tabs_.size()) {
                currentTabIndex_ = tabs_.size() - 1;
            }
            restoreTabState(currentTabIndex_);
            const TabData& tab = tabs_[currentTabIndex_];
            emit tabSwitched(currentTabIndex_, tab.url, tab.title);
            emit tabChanged(currentTabIndex_);
        }
        // 닫은 탭이 현재 탭보다 앞에 있으면 인덱스 조정
        else if (index < currentTabIndex_) {
            currentTabIndex_--;
        }
    }

    return true;
}

void TabManager::saveCurrentTabState() {
    if (currentTabIndex_ < 0 || currentTabIndex_ >= tabs_.size() || !webView_) {
        return;
    }

    TabData& tab = tabs_[currentTabIndex_];
    tab.url = webView_->url().toString();
    tab.title = webView_->title();

    Logger::debug(QString("[TabManager] 탭 상태 저장: index=%1, url=%2")
                      .arg(currentTabIndex_)
                      .arg(tab.url));
}

void TabManager::restoreTabState(int index) {
    if (index < 0 || index >= tabs_.size() || !webView_) {
        return;
    }

    const TabData& tab = tabs_[index];

    // WebView에 URL 로드
    webView_->load(QUrl(tab.url));

    Logger::debug(QString("[TabManager] 탭 상태 복원: index=%1, url=%2")
                      .arg(index)
                      .arg(tab.url));
}

} // namespace webosbrowser
