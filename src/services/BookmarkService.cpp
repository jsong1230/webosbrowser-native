/**
 * @file BookmarkService.cpp
 * @brief 북마크 비즈니스 로직 서비스 구현
 */

#include "BookmarkService.h"
#include "StorageService.h"
#include "../utils/Logger.h"
#include <QJsonArray>
#include <QJsonObject>
#include <algorithm>

namespace webosbrowser {

const QString BookmarkService::BOOKMARKS_KIND = "com.jsong.webosbrowser:1.bookmarks";
const QString BookmarkService::FOLDERS_KIND = "com.jsong.webosbrowser:1.folders";

BookmarkService::BookmarkService(StorageService* storageService, QObject *parent)
    : QObject(parent),
      m_storageService(storageService) {
    // 캐시 로드 (앱 시작 시)
    loadCache();
}

BookmarkService::~BookmarkService() {
}

void BookmarkService::loadCache() {
    Logger::info("[BookmarkService] 캐시 로드 시작");

    // 북마크 로드
    QJsonObject bookmarkQuery;
    bookmarkQuery["from"] = BOOKMARKS_KIND;

    m_storageService->findData(BOOKMARKS_KIND, bookmarkQuery, [this](bool success, const QJsonArray& results) {
        if (success) {
            m_bookmarksCache.clear();
            for (const QJsonValue& value : results) {
                Bookmark bookmark = Bookmark::fromJson(value.toObject());
                m_bookmarksCache.append(bookmark);
            }
            Logger::info(QString("[BookmarkService] 북마크 캐시 로드 완료: %1개").arg(m_bookmarksCache.size()));
        } else {
            Logger::error("[BookmarkService] 북마크 캐시 로드 실패");
        }
    });

    // 폴더 로드
    QJsonObject folderQuery;
    folderQuery["from"] = FOLDERS_KIND;

    m_storageService->findData(FOLDERS_KIND, folderQuery, [this](bool success, const QJsonArray& results) {
        if (success) {
            m_foldersCache.clear();
            for (const QJsonValue& value : results) {
                BookmarkFolder folder = BookmarkFolder::fromJson(value.toObject());
                m_foldersCache.append(folder);
            }
            Logger::info(QString("[BookmarkService] 폴더 캐시 로드 완료: %1개").arg(m_foldersCache.size()));
        } else {
            Logger::error("[BookmarkService] 폴더 캐시 로드 실패");
        }
    });
}

void BookmarkService::getAllBookmarks(std::function<void(bool, const QVector<Bookmark>&)> callback) {
    Logger::info("[BookmarkService] 모든 북마크 조회");
    callback(true, m_bookmarksCache);
}

void BookmarkService::getBookmarksByFolder(const QString& folderId, std::function<void(bool, const QVector<Bookmark>&)> callback) {
    Logger::info(QString("[BookmarkService] 폴더별 북마크 조회: folderId=%1").arg(folderId));

    QVector<Bookmark> filtered;
    for (const Bookmark& bookmark : m_bookmarksCache) {
        if (bookmark.folderId == folderId) {
            filtered.append(bookmark);
        }
    }

    callback(true, filtered);
}

void BookmarkService::getBookmarkById(const QString& id, std::function<void(bool, const Bookmark&)> callback) {
    Logger::info(QString("[BookmarkService] 단일 북마크 조회: id=%1").arg(id));

    Bookmark* bookmark = findBookmarkInCache(id);
    if (bookmark) {
        callback(true, *bookmark);
    } else {
        Logger::warning(QString("[BookmarkService] 북마크를 찾을 수 없음: id=%1").arg(id));
        callback(false, Bookmark());
    }
}

void BookmarkService::getBookmarkByUrl(const QString& url, std::function<void(bool, const Bookmark&)> callback) {
    Logger::info(QString("[BookmarkService] URL로 북마크 조회: url=%1").arg(url));

    for (const Bookmark& bookmark : m_bookmarksCache) {
        if (bookmark.url == url) {
            callback(true, bookmark);
            return;
        }
    }

    callback(false, Bookmark());
}

void BookmarkService::addBookmark(const Bookmark& bookmark, std::function<void(bool, const Bookmark&, const QString&)> callback) {
    Logger::info(QString("[BookmarkService] 북마크 추가: title=%1, url=%2").arg(bookmark.title, bookmark.url));

    // 중복 체크
    for (const Bookmark& existing : m_bookmarksCache) {
        if (existing.url == bookmark.url) {
            Logger::warning(QString("[BookmarkService] 중복 URL: %1").arg(bookmark.url));
            emit errorOccurred("이미 북마크에 추가된 페이지입니다");
            callback(false, Bookmark(), "이미 북마크에 추가된 페이지입니다");
            return;
        }
    }

    // 북마크 객체 생성
    Bookmark newBookmark = bookmark;
    newBookmark.id = StorageService::generateUuid();
    newBookmark.createdAt = QDateTime::currentDateTime();
    newBookmark.updatedAt = QDateTime::currentDateTime();
    newBookmark.visitCount = 0;

    // 유효성 검증
    if (!newBookmark.isValid()) {
        Logger::error("[BookmarkService] 유효하지 않은 북마크 데이터");
        emit errorOccurred("유효하지 않은 북마크 데이터");
        callback(false, Bookmark(), "유효하지 않은 북마크 데이터");
        return;
    }

    // StorageService에 저장
    QJsonArray data;
    data.append(newBookmark.toJson());

    m_storageService->putData(BOOKMARKS_KIND, data, [this, newBookmark, callback](bool success, const QJsonObject&) {
        if (success) {
            // 캐시에 추가
            m_bookmarksCache.append(newBookmark);
            Logger::info(QString("[BookmarkService] 북마크 추가 성공: id=%1").arg(newBookmark.id));
            emit bookmarkAdded(newBookmark);
            callback(true, newBookmark, "");
        } else {
            Logger::error("[BookmarkService] 북마크 추가 실패");
            emit errorOccurred("북마크 추가 실패");
            callback(false, Bookmark(), "북마크 추가 실패");
        }
    });
}

void BookmarkService::updateBookmark(const QString& id, const Bookmark& updates, std::function<void(bool)> callback) {
    Logger::info(QString("[BookmarkService] 북마크 수정: id=%1").arg(id));

    Bookmark* bookmark = findBookmarkInCache(id);
    if (!bookmark) {
        Logger::error(QString("[BookmarkService] 북마크를 찾을 수 없음: id=%1").arg(id));
        emit errorOccurred("북마크를 찾을 수 없습니다");
        callback(false);
        return;
    }

    // 업데이트 적용
    if (!updates.title.isEmpty()) {
        bookmark->title = updates.title;
    }
    if (!updates.folderId.isNull()) {
        bookmark->folderId = updates.folderId;
    }
    if (!updates.description.isNull()) {
        bookmark->description = updates.description;
    }
    bookmark->updatedAt = QDateTime::currentDateTime();

    // StorageService에 저장
    QJsonArray data;
    data.append(bookmark->toJson());

    m_storageService->putData(BOOKMARKS_KIND, data, [this, id, callback](bool success, const QJsonObject&) {
        if (success) {
            Logger::info(QString("[BookmarkService] 북마크 수정 성공: id=%1").arg(id));
            emit bookmarkUpdated(id);
            callback(true);
        } else {
            Logger::error("[BookmarkService] 북마크 수정 실패");
            emit errorOccurred("북마크 수정 실패");
            callback(false);
        }
    });
}

void BookmarkService::deleteBookmark(const QString& id, std::function<void(bool)> callback) {
    Logger::info(QString("[BookmarkService] 북마크 삭제: id=%1").arg(id));

    // 캐시에서 제거
    auto it = std::remove_if(m_bookmarksCache.begin(), m_bookmarksCache.end(), [&id](const Bookmark& b) {
        return b.id == id;
    });

    if (it == m_bookmarksCache.end()) {
        Logger::error(QString("[BookmarkService] 북마크를 찾을 수 없음: id=%1").arg(id));
        emit errorOccurred("북마크를 찾을 수 없습니다");
        callback(false);
        return;
    }

    m_bookmarksCache.erase(it, m_bookmarksCache.end());

    // StorageService에서 삭제
    QStringList ids;
    ids.append(id);

    m_storageService->deleteData(BOOKMARKS_KIND, ids, [this, id, callback](bool success) {
        if (success) {
            Logger::info(QString("[BookmarkService] 북마크 삭제 성공: id=%1").arg(id));
            emit bookmarkDeleted(id);
            callback(true);
        } else {
            Logger::error("[BookmarkService] 북마크 삭제 실패");
            emit errorOccurred("북마크 삭제 실패");
            callback(false);
        }
    });
}

void BookmarkService::incrementVisitCount(const QString& id, std::function<void(bool)> callback) {
    Logger::info(QString("[BookmarkService] 방문 횟수 증가: id=%1").arg(id));

    Bookmark* bookmark = findBookmarkInCache(id);
    if (!bookmark) {
        Logger::error(QString("[BookmarkService] 북마크를 찾을 수 없음: id=%1").arg(id));
        callback(false);
        return;
    }

    bookmark->visitCount++;
    bookmark->updatedAt = QDateTime::currentDateTime();

    // StorageService에 저장
    QJsonArray data;
    data.append(bookmark->toJson());

    m_storageService->putData(BOOKMARKS_KIND, data, [callback](bool success, const QJsonObject&) {
        callback(success);
    });
}

void BookmarkService::getAllFolders(std::function<void(bool, const QVector<BookmarkFolder>&)> callback) {
    Logger::info("[BookmarkService] 모든 폴더 조회");
    callback(true, m_foldersCache);
}

void BookmarkService::getFoldersByParent(const QString& parentId, std::function<void(bool, const QVector<BookmarkFolder>&)> callback) {
    Logger::info(QString("[BookmarkService] 서브폴더 조회: parentId=%1").arg(parentId));

    QVector<BookmarkFolder> filtered;
    for (const BookmarkFolder& folder : m_foldersCache) {
        if (folder.parentId == parentId) {
            filtered.append(folder);
        }
    }

    callback(true, filtered);
}

void BookmarkService::addFolder(const BookmarkFolder& folder, std::function<void(bool, const BookmarkFolder&)> callback) {
    Logger::info(QString("[BookmarkService] 폴더 추가: name=%1").arg(folder.name));

    // 폴더 객체 생성
    BookmarkFolder newFolder = folder;
    newFolder.id = StorageService::generateUuid();
    newFolder.createdAt = QDateTime::currentDateTime();

    // 유효성 검증
    if (!newFolder.isValid()) {
        Logger::error("[BookmarkService] 유효하지 않은 폴더 데이터");
        emit errorOccurred("유효하지 않은 폴더 데이터");
        callback(false, BookmarkFolder());
        return;
    }

    // StorageService에 저장
    QJsonArray data;
    data.append(newFolder.toJson());

    m_storageService->putData(FOLDERS_KIND, data, [this, newFolder, callback](bool success, const QJsonObject&) {
        if (success) {
            // 캐시에 추가
            m_foldersCache.append(newFolder);
            Logger::info(QString("[BookmarkService] 폴더 추가 성공: id=%1").arg(newFolder.id));
            emit folderAdded(newFolder);
            callback(true, newFolder);
        } else {
            Logger::error("[BookmarkService] 폴더 추가 실패");
            emit errorOccurred("폴더 추가 실패");
            callback(false, BookmarkFolder());
        }
    });
}

void BookmarkService::updateFolder(const QString& id, const QString& newName, std::function<void(bool)> callback) {
    Logger::info(QString("[BookmarkService] 폴더 수정: id=%1, newName=%2").arg(id, newName));

    BookmarkFolder* folder = findFolderInCache(id);
    if (!folder) {
        Logger::error(QString("[BookmarkService] 폴더를 찾을 수 없음: id=%1").arg(id));
        emit errorOccurred("폴더를 찾을 수 없습니다");
        callback(false);
        return;
    }

    folder->name = newName;

    // StorageService에 저장
    QJsonArray data;
    data.append(folder->toJson());

    m_storageService->putData(FOLDERS_KIND, data, [this, id, callback](bool success, const QJsonObject&) {
        if (success) {
            Logger::info(QString("[BookmarkService] 폴더 수정 성공: id=%1").arg(id));
            emit folderUpdated(id);
            callback(true);
        } else {
            Logger::error("[BookmarkService] 폴더 수정 실패");
            emit errorOccurred("폴더 수정 실패");
            callback(false);
        }
    });
}

void BookmarkService::deleteFolder(const QString& id, std::function<void(bool, int)> callback) {
    Logger::info(QString("[BookmarkService] 폴더 삭제: id=%1").arg(id));

    // 폴더 내 북마크 조회
    QVector<Bookmark> bookmarksInFolder;
    for (const Bookmark& bookmark : m_bookmarksCache) {
        if (bookmark.folderId == id) {
            bookmarksInFolder.append(bookmark);
        }
    }

    // 캐시에서 폴더 제거
    auto folderIt = std::remove_if(m_foldersCache.begin(), m_foldersCache.end(), [&id](const BookmarkFolder& f) {
        return f.id == id;
    });

    if (folderIt == m_foldersCache.end()) {
        Logger::error(QString("[BookmarkService] 폴더를 찾을 수 없음: id=%1").arg(id));
        emit errorOccurred("폴더를 찾을 수 없습니다");
        callback(false, 0);
        return;
    }

    m_foldersCache.erase(folderIt, m_foldersCache.end());

    // 캐시에서 북마크 제거
    m_bookmarksCache.erase(
        std::remove_if(m_bookmarksCache.begin(), m_bookmarksCache.end(), [&id](const Bookmark& b) {
            return b.folderId == id;
        }),
        m_bookmarksCache.end()
    );

    // StorageService에서 폴더 삭제
    QStringList folderIds;
    folderIds.append(id);

    m_storageService->deleteData(FOLDERS_KIND, folderIds, [this, id, bookmarksInFolder, callback](bool success) {
        if (!success) {
            Logger::error("[BookmarkService] 폴더 삭제 실패");
            emit errorOccurred("폴더 삭제 실패");
            callback(false, 0);
            return;
        }

        // StorageService에서 북마크 삭제
        QStringList bookmarkIds;
        for (const Bookmark& bookmark : bookmarksInFolder) {
            bookmarkIds.append(bookmark.id);
        }

        if (!bookmarkIds.isEmpty()) {
            m_storageService->deleteData(BOOKMARKS_KIND, bookmarkIds, [this, id, bookmarksInFolder, callback](bool success) {
                if (success) {
                    Logger::info(QString("[BookmarkService] 폴더 삭제 성공: id=%1, 삭제된 북마크=%2개")
                        .arg(id).arg(bookmarksInFolder.size()));
                    emit folderDeleted(id);
                    callback(true, bookmarksInFolder.size());
                } else {
                    Logger::error("[BookmarkService] 북마크 삭제 실패");
                    emit errorOccurred("북마크 삭제 실패");
                    callback(false, 0);
                }
            });
        } else {
            Logger::info(QString("[BookmarkService] 폴더 삭제 성공: id=%1 (북마크 없음)").arg(id));
            emit folderDeleted(id);
            callback(true, 0);
        }
    });
}

void BookmarkService::searchBookmarks(const QString& query, std::function<void(bool, const QVector<Bookmark>&)> callback) {
    Logger::info(QString("[BookmarkService] 북마크 검색: query=%1").arg(query));

    QString lowerQuery = query.toLower();
    QVector<Bookmark> results;

    for (const Bookmark& bookmark : m_bookmarksCache) {
        if (bookmark.title.toLower().contains(lowerQuery) ||
            bookmark.url.toLower().contains(lowerQuery)) {
            results.append(bookmark);
        }
    }

    Logger::info(QString("[BookmarkService] 검색 결과: %1개").arg(results.size()));
    callback(true, results);
}

Bookmark* BookmarkService::findBookmarkInCache(const QString& id) {
    for (Bookmark& bookmark : m_bookmarksCache) {
        if (bookmark.id == id) {
            return &bookmark;
        }
    }
    return nullptr;
}

BookmarkFolder* BookmarkService::findFolderInCache(const QString& id) {
    for (BookmarkFolder& folder : m_foldersCache) {
        if (folder.id == id) {
            return &folder;
        }
    }
    return nullptr;
}

} // namespace webosbrowser
