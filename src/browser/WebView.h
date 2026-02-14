/**
 * @file WebView.h
 * @brief WebView 컴포넌트 헤더 (스켈레톤)
 */

#pragma once

#include <QWidget>

namespace webosbrowser {

/**
 * @class WebView
 * @brief webOS WebView API 래퍼 클래스
 */
class WebView : public QWidget {
    Q_OBJECT

public:
    explicit WebView(QWidget *parent = nullptr);
    ~WebView() override;

    WebView(const WebView&) = delete;
    WebView& operator=(const WebView&) = delete;

    // TODO: WebView API 구현
};

} // namespace webosbrowser
