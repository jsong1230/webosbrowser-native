/**
 * @file StorageService.h
 * @brief 스토리지 서비스 헤더 (스켈레톤)
 */

#pragma once

#include <QObject>

namespace webosbrowser {

class StorageService : public QObject {
    Q_OBJECT

public:
    explicit StorageService(QObject *parent = nullptr);
    ~StorageService() override;

    StorageService(const StorageService&) = delete;
    StorageService& operator=(const StorageService&) = delete;
};

} // namespace webosbrowser
