/**
 * @file Bookmark.h
 * @brief 북마크 데이터 모델
 */

#pragma once

#include <QString>
#include <QDateTime>
#include <QJsonObject>

namespace webosbrowser {

/**
 * @brief 북마크 데이터 구조체
 */
struct Bookmark {
    QString id;              // 고유 식별자 (UUID)
    QString title;           // 북마크 제목
    QString url;             // 북마크 URL
    QString folderId;        // 폴더 ID (빈 문자열이면 루트 폴더)
    QString favicon;         // 파비콘 URL (선택적)
    QString description;     // 북마크 설명 (선택적)
    QDateTime createdAt;     // 생성 시각
    QDateTime updatedAt;     // 수정 시각
    int visitCount;          // 방문 횟수

    /**
     * @brief 기본 생성자
     */
    Bookmark()
        : visitCount(0),
          createdAt(QDateTime::currentDateTime()),
          updatedAt(QDateTime::currentDateTime()) {}

    /**
     * @brief JSON 객체로 변환
     * @return JSON 객체
     */
    QJsonObject toJson() const {
        QJsonObject json;
        json["id"] = id;
        json["title"] = title;
        json["url"] = url;
        json["folderId"] = folderId;
        json["favicon"] = favicon;
        json["description"] = description;
        json["createdAt"] = createdAt.toString(Qt::ISODate);
        json["updatedAt"] = updatedAt.toString(Qt::ISODate);
        json["visitCount"] = visitCount;
        return json;
    }

    /**
     * @brief JSON 객체에서 생성
     * @param json JSON 객체
     * @return 북마크 객체
     */
    static Bookmark fromJson(const QJsonObject& json) {
        Bookmark bookmark;
        bookmark.id = json["id"].toString();
        bookmark.title = json["title"].toString();
        bookmark.url = json["url"].toString();
        bookmark.folderId = json["folderId"].toString();
        bookmark.favicon = json["favicon"].toString();
        bookmark.description = json["description"].toString();
        bookmark.createdAt = QDateTime::fromString(json["createdAt"].toString(), Qt::ISODate);
        bookmark.updatedAt = QDateTime::fromString(json["updatedAt"].toString(), Qt::ISODate);
        bookmark.visitCount = json["visitCount"].toInt(0);
        return bookmark;
    }

    /**
     * @brief 유효성 검증
     * @return 유효하면 true
     */
    bool isValid() const {
        return !id.isEmpty() && !title.isEmpty() && !url.isEmpty();
    }
};

/**
 * @brief 북마크 폴더 데이터 구조체
 */
struct BookmarkFolder {
    QString id;              // 고유 식별자 (UUID)
    QString name;            // 폴더 이름
    QString parentId;        // 부모 폴더 ID (빈 문자열이면 루트)
    QDateTime createdAt;     // 생성 시각

    /**
     * @brief 기본 생성자
     */
    BookmarkFolder()
        : createdAt(QDateTime::currentDateTime()) {}

    /**
     * @brief JSON 객체로 변환
     * @return JSON 객체
     */
    QJsonObject toJson() const {
        QJsonObject json;
        json["id"] = id;
        json["name"] = name;
        json["parentId"] = parentId;
        json["createdAt"] = createdAt.toString(Qt::ISODate);
        return json;
    }

    /**
     * @brief JSON 객체에서 생성
     * @param json JSON 객체
     * @return 폴더 객체
     */
    static BookmarkFolder fromJson(const QJsonObject& json) {
        BookmarkFolder folder;
        folder.id = json["id"].toString();
        folder.name = json["name"].toString();
        folder.parentId = json["parentId"].toString();
        folder.createdAt = QDateTime::fromString(json["createdAt"].toString(), Qt::ISODate);
        return folder;
    }

    /**
     * @brief 유효성 검증
     * @return 유효하면 true
     */
    bool isValid() const {
        return !id.isEmpty() && !name.isEmpty();
    }
};

} // namespace webosbrowser
