/**
 * @file BookmarkService.h
 * @brief 북마크 비즈니스 로직 서비스
 */

#pragma once

#include <QObject>
#include <QVector>
#include <QString>
#include <functional>
#include "../models/Bookmark.h"

namespace webosbrowser {

class StorageService;

/**
 * @brief 북마크 관리 서비스
 *
 * 북마크 CRUD 작업, 폴더 관리, 검색/정렬 기능 제공
 * StorageService를 사용하여 데이터 영속 저장
 */
class BookmarkService : public QObject {
    Q_OBJECT

public:
    explicit BookmarkService(StorageService* storageService, QObject *parent = nullptr);
    ~BookmarkService() override;

    BookmarkService(const BookmarkService&) = delete;
    BookmarkService& operator=(const BookmarkService&) = delete;

    // 북마크 CRUD 작업

    /**
     * @brief 모든 북마크 조회
     * @param callback 완료 콜백 (성공 여부, 북마크 리스트)
     */
    void getAllBookmarks(std::function<void(bool, const QVector<Bookmark>&)> callback);

    /**
     * @brief 폴더별 북마크 조회
     * @param folderId 폴더 ID (빈 문자열이면 루트 폴더)
     * @param callback 완료 콜백
     */
    void getBookmarksByFolder(const QString& folderId, std::function<void(bool, const QVector<Bookmark>&)> callback);

    /**
     * @brief 단일 북마크 조회
     * @param id 북마크 ID
     * @param callback 완료 콜백 (성공 여부, 북마크 객체)
     */
    void getBookmarkById(const QString& id, std::function<void(bool, const Bookmark&)> callback);

    /**
     * @brief URL로 북마크 조회 (중복 체크용)
     * @param url URL
     * @param callback 완료 콜백 (성공 여부, 북마크 객체)
     */
    void getBookmarkByUrl(const QString& url, std::function<void(bool, const Bookmark&)> callback);

    /**
     * @brief 북마크 추가
     * @param bookmark 북마크 객체
     * @param callback 완료 콜백 (성공 여부, 추가된 북마크)
     */
    void addBookmark(const Bookmark& bookmark, std::function<void(bool, const Bookmark&, const QString&)> callback);

    /**
     * @brief 북마크 수정
     * @param id 북마크 ID
     * @param updates 업데이트할 필드들
     * @param callback 완료 콜백 (성공 여부)
     */
    void updateBookmark(const QString& id, const Bookmark& updates, std::function<void(bool)> callback);

    /**
     * @brief 북마크 삭제
     * @param id 북마크 ID
     * @param callback 완료 콜백 (성공 여부)
     */
    void deleteBookmark(const QString& id, std::function<void(bool)> callback);

    /**
     * @brief 방문 횟수 증가
     * @param id 북마크 ID
     * @param callback 완료 콜백 (성공 여부)
     */
    void incrementVisitCount(const QString& id, std::function<void(bool)> callback);

    // 폴더 관리

    /**
     * @brief 모든 폴더 조회
     * @param callback 완료 콜백 (성공 여부, 폴더 리스트)
     */
    void getAllFolders(std::function<void(bool, const QVector<BookmarkFolder>&)> callback);

    /**
     * @brief 서브폴더 조회
     * @param parentId 부모 폴더 ID (빈 문자열이면 루트)
     * @param callback 완료 콜백
     */
    void getFoldersByParent(const QString& parentId, std::function<void(bool, const QVector<BookmarkFolder>&)> callback);

    /**
     * @brief 폴더 추가
     * @param folder 폴더 객체
     * @param callback 완료 콜백 (성공 여부, 추가된 폴더)
     */
    void addFolder(const BookmarkFolder& folder, std::function<void(bool, const BookmarkFolder&)> callback);

    /**
     * @brief 폴더 이름 변경
     * @param id 폴더 ID
     * @param newName 새 폴더 이름
     * @param callback 완료 콜백 (성공 여부)
     */
    void updateFolder(const QString& id, const QString& newName, std::function<void(bool)> callback);

    /**
     * @brief 폴더 삭제 (하위 북마크 포함)
     * @param id 폴더 ID
     * @param callback 완료 콜백 (성공 여부, 삭제된 북마크 수)
     */
    void deleteFolder(const QString& id, std::function<void(bool, int)> callback);

    // 검색 및 정렬

    /**
     * @brief 북마크 검색 (제목, URL에서 부분 일치)
     * @param query 검색어
     * @param callback 완료 콜백 (성공 여부, 검색 결과)
     */
    void searchBookmarks(const QString& query, std::function<void(bool, const QVector<Bookmark>&)> callback);

signals:
    /**
     * @brief 북마크 추가됨 시그널
     * @param bookmark 추가된 북마크
     */
    void bookmarkAdded(const Bookmark& bookmark);

    /**
     * @brief 북마크 업데이트됨 시그널
     * @param id 북마크 ID
     */
    void bookmarkUpdated(const QString& id);

    /**
     * @brief 북마크 삭제됨 시그널
     * @param id 북마크 ID
     */
    void bookmarkDeleted(const QString& id);

    /**
     * @brief 폴더 추가됨 시그널
     * @param folder 추가된 폴더
     */
    void folderAdded(const BookmarkFolder& folder);

    /**
     * @brief 폴더 업데이트됨 시그널
     * @param id 폴더 ID
     */
    void folderUpdated(const QString& id);

    /**
     * @brief 폴더 삭제됨 시그널
     * @param id 폴더 ID
     */
    void folderDeleted(const QString& id);

    /**
     * @brief 에러 발생 시그널
     * @param message 에러 메시지
     */
    void errorOccurred(const QString& message);

private:
    StorageService* m_storageService;  // 스토리지 서비스 (의존성 주입)
    QVector<Bookmark> m_bookmarksCache;  // 북마크 캐시 (메모리)
    QVector<BookmarkFolder> m_foldersCache;  // 폴더 캐시

    /**
     * @brief 캐시 로드 (앱 시작 시)
     */
    void loadCache();

    /**
     * @brief 캐시에서 북마크 찾기
     * @param id 북마크 ID
     * @return 북마크 포인터 (없으면 nullptr)
     */
    Bookmark* findBookmarkInCache(const QString& id);

    /**
     * @brief 캐시에서 폴더 찾기
     * @param id 폴더 ID
     * @return 폴더 포인터 (없으면 nullptr)
     */
    BookmarkFolder* findFolderInCache(const QString& id);
};

} // namespace webosbrowser
