/**
 * @file BookmarkService.h
 * @brief 북마크 서비스 헤더 (스켈레톤)
 */

#pragma once

#include <QObject>

namespace webosbrowser {

class BookmarkService : public QObject {
    Q_OBJECT

public:
    explicit BookmarkService(QObject *parent = nullptr);
    ~BookmarkService() override;

    BookmarkService(const BookmarkService&) = delete;
    BookmarkService& operator=(const BookmarkService&) = delete;
};

} // namespace webosbrowser
