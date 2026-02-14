/**
 * @file NavigationBar.h
 * @brief 네비게이션 바 헤더 (스켈레톤)
 */

#pragma once

#include <QWidget>

namespace webosbrowser {

class NavigationBar : public QWidget {
    Q_OBJECT

public:
    explicit NavigationBar(QWidget *parent = nullptr);
    ~NavigationBar() override;

    NavigationBar(const NavigationBar&) = delete;
    NavigationBar& operator=(const NavigationBar&) = delete;
};

} // namespace webosbrowser
