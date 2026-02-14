/**
 * @file TabManager.h
 * @brief 탭 관리자 헤더 (스켈레톤)
 */

#pragma once

#include <QObject>

namespace webosbrowser {

class TabManager : public QObject {
    Q_OBJECT

public:
    explicit TabManager(QObject *parent = nullptr);
    ~TabManager() override;

    TabManager(const TabManager&) = delete;
    TabManager& operator=(const TabManager&) = delete;
};

} // namespace webosbrowser
