/**
 * @file URLBar.h
 * @brief URL 입력 바 헤더 (스켈레톤)
 */

#pragma once

#include <QWidget>

namespace webosbrowser {

class URLBar : public QWidget {
    Q_OBJECT

public:
    explicit URLBar(QWidget *parent = nullptr);
    ~URLBar() override;

    URLBar(const URLBar&) = delete;
    URLBar& operator=(const URLBar&) = delete;
};

} // namespace webosbrowser
