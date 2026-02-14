/**
 * @file HistoryService.h
 * @brief 히스토리 서비스 헤더 (스켈레톤)
 */

#pragma once

#include <QObject>

namespace webosbrowser {

class HistoryService : public QObject {
    Q_OBJECT

public:
    explicit HistoryService(QObject *parent = nullptr);
    ~HistoryService() override;

    HistoryService(const HistoryService&) = delete;
    HistoryService& operator=(const HistoryService&) = delete;
};

} // namespace webosbrowser
