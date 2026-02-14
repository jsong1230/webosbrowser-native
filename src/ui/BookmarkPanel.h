/**
 * @file BookmarkPanel.h
 * @brief 북마크 패널 헤더 (스켈레톤)
 */

#pragma once

#include <QWidget>

namespace webosbrowser {

class BookmarkPanel : public QWidget {
    Q_OBJECT

public:
    explicit BookmarkPanel(QWidget *parent = nullptr);
    ~BookmarkPanel() override;

    BookmarkPanel(const BookmarkPanel&) = delete;
    BookmarkPanel& operator=(const BookmarkPanel&) = delete;
};

} // namespace webosbrowser
