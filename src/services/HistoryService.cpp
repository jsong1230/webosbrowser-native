/**
 * @file HistoryService.cpp
 * @brief 히스토리 관리 서비스 구현
 */

#include "HistoryService.h"
#include "StorageService.h"
#include "utils/Logger.h"
#include <QUuid>
#include <algorithm>

namespace webosbrowser {

// HistoryEntry 구조체 구현

QJsonObject HistoryEntry::toJson() const {
    QJsonObject json;
    json["id"] = id;
    json["url"] = url;
    json["title"] = title;
    json["favicon"] = favicon;
    json["visitedAt"] = visitedAt.toString(Qt::ISODate);
    json["visitCount"] = visitCount;
    return json;
}

HistoryEntry HistoryEntry::fromJson(const QJsonObject &json) {
    HistoryEntry entry;
    entry.id = json["id"].toString();
    entry.url = json["url"].toString();
    entry.title = json["title"].toString();
    entry.favicon = json["favicon"].toString();
    entry.visitedAt = QDateTime::fromString(json["visitedAt"].toString(), Qt::ISODate);
    entry.visitCount = json["visitCount"].toInt(1);
    return entry;
}

// HistoryService 구현

HistoryService::HistoryService(StorageService *storageService, QObject *parent)
    : QObject(parent)
    , storageService_(storageService)
    , isLoaded_(false)
{
    if (!storageService_) {
        Logger::error("HistoryService: StorageService가 null입니다.");
        return;
    }

    // 스토리지에서 히스토리 로드
    loadHistoryFromStorage();
}

HistoryService::~HistoryService() {
}

void HistoryService::recordVisit(const QString &url, const QString &title, const QString &favicon) {
    if (url.isEmpty()) {
        Logger::warning("HistoryService::recordVisit: URL이 비어있습니다.");
        return;
    }

    // 1분 내 동일 URL 방문 체크 (중복 방지)
    HistoryEntry *recentVisit = findRecentVisit(url);

    if (recentVisit) {
        // 중복: visitedAt, visitCount 업데이트
        recentVisit->visitedAt = QDateTime::currentDateTime();
        recentVisit->visitCount++;

        // 제목이 변경되었으면 업데이트
        if (!title.isEmpty() && title != recentVisit->title) {
            recentVisit->title = title;
        }

        // 스토리지에 저장
        storageService_->putData(DataKind::History, recentVisit->id, recentVisit->toJson());

        emit historyUpdated(*recentVisit);
        Logger::info(QString("히스토리 업데이트 (중복 방지): %1 (visitCount: %2)")
                     .arg(url).arg(recentVisit->visitCount));
        return;
    }

    // 신규 히스토리 추가
    HistoryEntry newEntry;
    newEntry.id = generateUUID();
    newEntry.url = url;
    newEntry.title = title.isEmpty() ? url : title;
    newEntry.favicon = favicon;
    newEntry.visitedAt = QDateTime::currentDateTime();
    newEntry.visitCount = 1;

    // 메모리 캐시에 추가
    historyCache_.append(newEntry);

    // 스토리지에 저장
    storageService_->putData(DataKind::History, newEntry.id, newEntry.toJson());

    emit historyAdded(newEntry);
    Logger::info(QString("히스토리 추가: %1").arg(url));

    // 용량 제한 체크 (5,000개 초과 시 오래된 항목 삭제)
    pruneOldHistory();
}

QList<HistoryEntry> HistoryService::getAllHistory() const {
    // visitedAt 역순 정렬 (최신순)
    QList<HistoryEntry> sortedHistory = historyCache_;
    std::sort(sortedHistory.begin(), sortedHistory.end(), [](const HistoryEntry &a, const HistoryEntry &b) {
        return a.visitedAt > b.visitedAt;
    });
    return sortedHistory;
}

QList<HistoryEntry> HistoryService::getHistoryByDateRange(const QDateTime &startDate, const QDateTime &endDate) const {
    QList<HistoryEntry> result;

    for (const HistoryEntry &entry : historyCache_) {
        if (entry.visitedAt >= startDate && entry.visitedAt <= endDate) {
            result.append(entry);
        }
    }

    // visitedAt 역순 정렬
    std::sort(result.begin(), result.end(), [](const HistoryEntry &a, const HistoryEntry &b) {
        return a.visitedAt > b.visitedAt;
    });

    return result;
}

HistoryEntry HistoryService::getHistoryById(const QString &id) const {
    for (const HistoryEntry &entry : historyCache_) {
        if (entry.id == id) {
            return entry;
        }
    }
    return HistoryEntry(); // 빈 항목 반환
}

QList<HistoryEntry> HistoryService::searchHistory(const QString &query) const {
    if (query.isEmpty()) {
        return getAllHistory();
    }

    QList<HistoryEntry> result;
    QString lowerQuery = query.toLower();

    for (const HistoryEntry &entry : historyCache_) {
        if (entry.title.toLower().contains(lowerQuery) ||
            entry.url.toLower().contains(lowerQuery)) {
            result.append(entry);
        }
    }

    // visitedAt 역순 정렬
    std::sort(result.begin(), result.end(), [](const HistoryEntry &a, const HistoryEntry &b) {
        return a.visitedAt > b.visitedAt;
    });

    return result;
}

QMap<HistoryDateGroup, QList<HistoryEntry>> HistoryService::getGroupedHistory() const {
    QMap<HistoryDateGroup, QList<HistoryEntry>> grouped;

    for (const HistoryEntry &entry : getAllHistory()) {
        HistoryDateGroup group = calculateDateGroup(entry.visitedAt);
        grouped[group].append(entry);
    }

    return grouped;
}

bool HistoryService::deleteHistory(const QString &id) {
    // 메모리 캐시에서 제거
    auto it = std::find_if(historyCache_.begin(), historyCache_.end(), [&id](const HistoryEntry &entry) {
        return entry.id == id;
    });

    if (it == historyCache_.end()) {
        Logger::warning(QString("HistoryService::deleteHistory: ID를 찾을 수 없습니다: %1").arg(id));
        return false;
    }

    historyCache_.erase(it);

    // 스토리지에서 삭제
    bool success = storageService_->deleteData(DataKind::History, id);

    if (success) {
        emit historyDeleted(id);
        Logger::info(QString("히스토리 삭제 완료: ID=%1").arg(id));
    }

    return success;
}

int HistoryService::deleteHistoryByDateRange(const QDateTime &startDate, const QDateTime &endDate) {
    QList<QString> idsToDelete;

    // 삭제할 항목 ID 수집
    for (const HistoryEntry &entry : historyCache_) {
        if (entry.visitedAt >= startDate && entry.visitedAt <= endDate) {
            idsToDelete.append(entry.id);
        }
    }

    // 삭제 실행
    int deletedCount = 0;
    for (const QString &id : idsToDelete) {
        if (deleteHistory(id)) {
            deletedCount++;
        }
    }

    Logger::info(QString("기간별 히스토리 삭제 완료: %1개").arg(deletedCount));
    return deletedCount;
}

bool HistoryService::deleteAllHistory() {
    int count = storageService_->deleteAllData(DataKind::History);

    // 메모리 캐시 초기화
    historyCache_.clear();

    if (count >= 0) {
        emit allHistoryDeleted();
        Logger::info(QString("전체 히스토리 삭제 완료: %1개").arg(count));
        return true;
    }

    return false;
}

int HistoryService::getHistoryCount() const {
    return historyCache_.size();
}

void HistoryService::loadHistoryFromStorage() {
    if (!storageService_) {
        Logger::error("HistoryService::loadHistoryFromStorage: StorageService가 null입니다.");
        return;
    }

    QJsonArray data = storageService_->findAllData(DataKind::History);

    historyCache_.clear();

    for (const QJsonValue &value : data) {
        QJsonObject json = value.toObject();
        HistoryEntry entry = HistoryEntry::fromJson(json);
        historyCache_.append(entry);
    }

    isLoaded_ = true;
    emit historyLoaded();
    Logger::info(QString("히스토리 로드 완료: %1개").arg(historyCache_.size()));
}

void HistoryService::saveHistoryToStorage() {
    // 현재 구현에서는 각 추가/삭제 시 즉시 저장하므로 이 함수는 불필요
    // 나중에 일괄 저장이 필요하면 사용
    Logger::debug("HistoryService::saveHistoryToStorage: 일괄 저장 (현재 미사용)");
}

HistoryEntry* HistoryService::findRecentVisit(const QString &url) {
    QDateTime now = QDateTime::currentDateTime();
    QDateTime threshold = now.addMSecs(-DUPLICATE_THRESHOLD_MS);

    for (HistoryEntry &entry : historyCache_) {
        if (entry.url == url && entry.visitedAt >= threshold) {
            return &entry;
        }
    }

    return nullptr;
}

void HistoryService::pruneOldHistory() {
    if (historyCache_.size() <= MAX_HISTORY_COUNT) {
        return;
    }

    // visitedAt 오름차순 정렬 (오래된 순)
    QList<HistoryEntry> sorted = historyCache_;
    std::sort(sorted.begin(), sorted.end(), [](const HistoryEntry &a, const HistoryEntry &b) {
        return a.visitedAt < b.visitedAt;
    });

    // 오래된 항목 삭제
    int deleteCount = historyCache_.size() - MAX_HISTORY_COUNT;
    for (int i = 0; i < deleteCount; ++i) {
        deleteHistory(sorted[i].id);
    }

    Logger::info(QString("용량 제한으로 오래된 히스토리 삭제: %1개").arg(deleteCount));
}

QString HistoryService::generateUUID() const {
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

HistoryDateGroup HistoryService::calculateDateGroup(const QDateTime &visitedAt) const {
    QDateTime now = QDateTime::currentDateTime();
    qint64 diffSeconds = visitedAt.secsTo(now);

    const qint64 oneDay = 24 * 60 * 60;

    if (diffSeconds < oneDay) {
        return HistoryDateGroup::Today;
    } else if (diffSeconds < oneDay * 2) {
        return HistoryDateGroup::Yesterday;
    } else if (diffSeconds < oneDay * 7) {
        return HistoryDateGroup::Last7Days;
    } else if (diffSeconds < oneDay * 30) {
        return HistoryDateGroup::ThisMonth;
    } else {
        return HistoryDateGroup::Older;
    }
}

} // namespace webosbrowser
