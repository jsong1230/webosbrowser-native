# 북마크 관리 — 기술 설계서

## 1. 참조
- **요구사항 분석서**: `docs/specs/bookmark-management/requirements.md`
- **프로젝트 가이드**: `CLAUDE.md`
- **관련 기능**: F-02 (웹뷰 통합), F-03 (URL 입력 UI)

---

## 2. 아키텍처 개요

### 2.1 레이어 구조
```
┌──────────────────────────────────────────────────┐
│                   UI Layer                       │
│  ┌────────────────┐        ┌──────────────────┐ │
│  │ NavigationBar  │        │  BookmarkPanel   │ │
│  │  (북마크 버튼)  │◄──────►│  (목록/편집 UI)  │ │
│  └────────────────┘        └──────────────────┘ │
└────────────────┬─────────────────────┬───────────┘
                 │                     │
                 ▼                     ▼
┌──────────────────────────────────────────────────┐
│                Service Layer                     │
│            ┌──────────────────┐                  │
│            │ BookmarkService  │                  │
│            │  (비즈니스 로직)  │                  │
│            └──────────────────┘                  │
└────────────────┬─────────────────────────────────┘
                 │ Signal/Slot (비동기)
                 ▼
┌──────────────────────────────────────────────────┐
│                Storage Layer                     │
│            ┌──────────────────┐                  │
│            │ StorageService   │                  │
│            │  (LS2 API 래퍼)   │                  │
│            └──────────────────┘                  │
└────────────────┬─────────────────────────────────┘
                 │ luna-service2 API
                 ▼
         ┌────────────────┐
         │  webOS DB8     │
         │ (JSON NoSQL)   │
         └────────────────┘
```

### 2.2 컴포넌트 다이어그램
```
┌─────────────────┐           ┌──────────────────┐
│ BrowserWindow   │           │   WebView        │
│                 │◄──────────│                  │
│  - urlBar_      │           │  + title()       │
│  - navBar_      │           │  + url()         │
│  - webView_     │           └──────────────────┘
│  - bookmarkPanel│                    △
└─────────────────┘                    │
         △                             │ getCurrentInfo()
         │                             │
    contains                           │
         │                             │
         ▼                             │
┌─────────────────┐           ┌──────────────────┐
│ NavigationBar   │           │  BookmarkPanel   │
│                 │           │                  │
│  - backBtn_     │  show()   │  + show()        │
│  - bookmarkBtn_ │──────────►│  + hide()        │
│  + addBookmark()│           │  + loadBookmarks │
└─────────────────┘           └──────────────────┘
                                       │
                                       │ uses
                                       ▼
                              ┌──────────────────┐
                              │ BookmarkService  │
                              │                  │
                              │  + add()         │
                              │  + getAll()      │
                              │  + update()      │
                              │  + delete()      │
                              │  + createFolder()│
                              └──────────────────┘
                                       │
                                       │ uses
                                       ▼
                              ┌──────────────────┐
                              │ StorageService   │
                              │                  │
                              │  + put()         │
                              │  + find()        │
                              │  + del()         │
                              └──────────────────┘
```

### 2.3 데이터 흐름
```
사용자 입력 → UI 이벤트 → BookmarkService → StorageService → LS2 API → DB8
     ↑                                                                      │
     └──────────── 시그널/슬롯 (비동기 응답) ◄─────────────────────────────┘
```

---

## 3. 아키텍처 결정

### 결정 1: 데이터 저장 방식

**선택지**:
- A) webOS LS2 API (DB8) 사용
- B) SQLite 직접 사용
- C) JSON 파일 저장

**결정**: **A) webOS LS2 API (DB8) 사용**

**근거**:
- webOS 공식 권장 방식으로 시스템 통합이 용이
- 비동기 메시지 버스로 메인 스레드 블로킹 방지
- JSON 기반으로 유연한 스키마 변경 가능
- 앱 샌드박스 환경에서 권한 관리 자동화
- 향후 webOS 서비스(백업, 동기화 등)와 통합 가능

**트레이드오프**:
- 포기: SQLite의 SQL 쿼리 기능, 로컬 파일 직접 제어
- 얻음: webOS 플랫폼 최적화, 비동기 처리, 시스템 통합

### 결정 2: 폴더 구조 깊이

**선택지**:
- A) 단일 레벨 (폴더 없음, 북마크만 존재)
- B) 1단계 서브폴더 (루트 폴더 + 1단계 서브폴더)
- C) 다단계 폴더 (무제한 깊이)

**결정**: **B) 1단계 서브폴더**

**근거**:
- 리모컨 네비게이션 복잡도 최소화 (2~3번 버튼 클릭으로 접근)
- QTreeWidget 구현 복잡도 낮음 (parent-child 관계만 처리)
- 대부분 사용자는 1단계 폴더로 충분 (예: 엔터테인먼트, 뉴스, 쇼핑)
- 향후 요구사항 변경 시 재귀적 구조로 확장 가능

**트레이드오프**:
- 포기: 복잡한 다단계 폴더 구조
- 얻음: 단순한 UX, 빠른 구현, 리모컨 최적화

### 결정 3: UI 패턴 (Dialog vs Panel)

**선택지**:
- A) QDialog (모달 팝업)
- B) QWidget Panel (슬라이드 인/아웃)
- C) QDockWidget (도킹 가능)

**결정**: **B) QWidget Panel (슬라이드 인/아웃)**

**근거**:
- 대화면 환경에서 북마크 목록을 넓게 표시 가능
- 비모달 방식으로 백그라운드 웹 페이지를 볼 수 있음
- 슬라이드 애니메이션으로 자연스러운 전환 (QPropertyAnimation)
- 리모컨 Back 버튼으로 패널 닫기 직관적
- BrowserWindow의 QStackedWidget에 추가하여 레이아웃 관리 용이

**트레이드오프**:
- 포기: QDialog의 간단한 모달 처리
- 얻음: 유연한 레이아웃, 애니메이션, 비모달 UX

### 결정 4: 북마크 추가 다이얼로그 방식

**선택지**:
- A) NavigationBar에 버튼 추가 → QDialog 표시
- B) 리모컨 단축키(빨강 버튼) → QDialog 표시
- C) 컨텍스트 메뉴 → QDialog 표시

**결정**: **A + B 병행** (NavigationBar 버튼 + 리모컨 단축키)

**근거**:
- NavigationBar 버튼: 시각적 피드백, 초보 사용자 친화적
- 리모컨 단축키: 파워 유저 효율성 (1번 버튼으로 즉시 추가)
- 두 방식 모두 같은 QDialog 표시 (코드 재사용)

**트레이드오프**:
- 포기: 단일 진입점
- 얻음: 다양한 사용자층 대응, 접근성 향상

### 결정 5: 북마크 중복 체크 기준

**선택지**:
- A) URL만 비교 (대소문자 무시)
- B) URL + 제목 조합
- C) URL + 폴더 조합 (같은 URL이라도 다른 폴더면 허용)

**결정**: **A) URL만 비교 (대소문자 무시)**

**근거**:
- 일반적으로 같은 URL은 같은 페이지 (제목/폴더 무관)
- 사용자 혼란 방지 (같은 사이트 중복 저장 방지)
- LS2 API find 쿼리로 간단히 구현 가능
- QString::toLower()로 대소문자 정규화

**트레이드오프**:
- 포기: 같은 URL을 다른 폴더에 중복 저장
- 얻음: 명확한 중복 방지, 데이터 정합성

---

## 4. 클래스 설계

### 4.1 데이터 모델 (src/models/Bookmark.h)

#### Bookmark 구조체
```cpp
namespace webosbrowser {

/**
 * @struct Bookmark
 * @brief 북마크 데이터 모델
 */
struct Bookmark {
    QString id;           ///< UUID (QUuid::createUuid().toString())
    QString title;        ///< 북마크 제목
    QString url;          ///< 북마크 URL (정규화된 형태)
    QString folderId;     ///< 폴더 ID (빈 문자열 = 루트)
    QString favicon;      ///< 파비콘 URL (선택, 기본 빈 문자열)
    qint64 createdAt;     ///< 생성 시각 (Unix timestamp)
    qint64 updatedAt;     ///< 수정 시각 (Unix timestamp)
    int visitCount;       ///< 방문 횟수 (선택, 기본 0)

    /**
     * @brief JSON 직렬화
     * @return QJsonObject
     */
    QJsonObject toJson() const {
        QJsonObject json;
        json["_kind"] = "com.jsong.webosbrowser.native:1";
        json["id"] = id;
        json["title"] = title;
        json["url"] = url;
        json["folderId"] = folderId;
        json["favicon"] = favicon;
        json["createdAt"] = createdAt;
        json["updatedAt"] = updatedAt;
        json["visitCount"] = visitCount;
        return json;
    }

    /**
     * @brief JSON 역직렬화
     * @param json QJsonObject
     * @return Bookmark 인스턴스
     */
    static Bookmark fromJson(const QJsonObject &json) {
        Bookmark bookmark;
        bookmark.id = json["id"].toString();
        bookmark.title = json["title"].toString();
        bookmark.url = json["url"].toString();
        bookmark.folderId = json["folderId"].toString();
        bookmark.favicon = json["favicon"].toString();
        bookmark.createdAt = json["createdAt"].toVariant().toLongLong();
        bookmark.updatedAt = json["updatedAt"].toVariant().toLongLong();
        bookmark.visitCount = json["visitCount"].toInt(0);
        return bookmark;
    }

    /**
     * @brief 기본 생성자
     */
    Bookmark() : createdAt(0), updatedAt(0), visitCount(0) {}

    /**
     * @brief 편의 생성자
     */
    Bookmark(const QString &t, const QString &u, const QString &fid = "")
        : id(QUuid::createUuid().toString(QUuid::WithoutBraces)),
          title(t),
          url(u),
          folderId(fid),
          createdAt(QDateTime::currentMSecsSinceEpoch()),
          updatedAt(QDateTime::currentMSecsSinceEpoch()),
          visitCount(0) {}
};

/**
 * @struct Folder
 * @brief 북마크 폴더 데이터 모델
 */
struct Folder {
    QString id;           ///< UUID
    QString name;         ///< 폴더 이름
    QString parentId;     ///< 부모 폴더 ID (빈 문자열 = 루트)
    qint64 createdAt;     ///< 생성 시각 (Unix timestamp)

    /**
     * @brief JSON 직렬화
     */
    QJsonObject toJson() const {
        QJsonObject json;
        json["_kind"] = "com.jsong.webosbrowser.native.folder:1";
        json["id"] = id;
        json["name"] = name;
        json["parentId"] = parentId;
        json["createdAt"] = createdAt;
        return json;
    }

    /**
     * @brief JSON 역직렬화
     */
    static Folder fromJson(const QJsonObject &json) {
        Folder folder;
        folder.id = json["id"].toString();
        folder.name = json["name"].toString();
        folder.parentId = json["parentId"].toString();
        folder.createdAt = json["createdAt"].toVariant().toLongLong();
        return folder;
    }

    /**
     * @brief 기본 생성자
     */
    Folder() : createdAt(0) {}

    /**
     * @brief 편의 생성자
     */
    Folder(const QString &n, const QString &pid = "")
        : id(QUuid::createUuid().toString(QUuid::WithoutBraces)),
          name(n),
          parentId(pid),
          createdAt(QDateTime::currentMSecsSinceEpoch()) {}
};

} // namespace webosbrowser
```

### 4.2 StorageService 확장 (src/services/StorageService.h)

```cpp
#pragma once

#include <QObject>
#include <QJsonObject>
#include <QJsonArray>
#include <QString>
#include <functional>

namespace webosbrowser {

/**
 * @class StorageService
 * @brief webOS LS2 API (DB8) 래퍼 클래스
 *
 * 비동기 JSON 기반 데이터 저장/조회/삭제 기능 제공.
 * luna-service2 메시지 버스를 통해 DB8과 통신.
 */
class StorageService : public QObject {
    Q_OBJECT

public:
    explicit StorageService(QObject *parent = nullptr);
    ~StorageService() override;

    StorageService(const StorageService&) = delete;
    StorageService& operator=(const StorageService&) = delete;

    /**
     * @brief 데이터 저장 (com.webos.service.db/put)
     * @param data JSON 객체 (반드시 _kind 필드 포함)
     */
    void put(const QJsonObject &data);

    /**
     * @brief 데이터 조회 (com.webos.service.db/find)
     * @param query JSON 쿼리 객체 (예: {"_kind": "...", "limit": 100})
     */
    void find(const QJsonObject &query);

    /**
     * @brief 데이터 삭제 (com.webos.service.db/del)
     * @param ids 삭제할 데이터 ID 배열
     */
    void del(const QJsonArray &ids);

    /**
     * @brief 단일 데이터 삭제 (편의 메서드)
     * @param id 삭제할 데이터 ID
     */
    void del(const QString &id);

signals:
    /**
     * @brief put() 성공 시그널
     * @param id 저장된 데이터 ID
     */
    void putSuccess(const QString &id);

    /**
     * @brief find() 성공 시그널
     * @param results 조회 결과 JSON 배열
     */
    void findSuccess(const QJsonArray &results);

    /**
     * @brief del() 성공 시그널
     * @param count 삭제된 데이터 개수
     */
    void delSuccess(int count);

    /**
     * @brief 에러 시그널
     * @param errorMessage 에러 메시지
     */
    void error(const QString &errorMessage);

private:
    class StorageServicePrivate;
    QScopedPointer<StorageServicePrivate> d_ptr;
    Q_DECLARE_PRIVATE(StorageService)
};

} // namespace webosbrowser
```

### 4.3 BookmarkService (src/services/BookmarkService.h)

```cpp
#pragma once

#include <QObject>
#include <QString>
#include <QVector>
#include <QMap>
#include <memory>
#include "../models/Bookmark.h"

namespace webosbrowser {

// Forward declaration
class StorageService;

/**
 * @class BookmarkService
 * @brief 북마크 비즈니스 로직 서비스
 *
 * 북마크 CRUD, 폴더 관리, 중복 체크, 정렬 등의 기능 제공.
 * StorageService를 통해 LS2 API와 통신.
 */
class BookmarkService : public QObject {
    Q_OBJECT

public:
    /**
     * @brief 생성자
     * @param storageService StorageService 인스턴스 (의존성 주입)
     * @param parent 부모 QObject
     */
    explicit BookmarkService(StorageService *storageService, QObject *parent = nullptr);
    ~BookmarkService() override;

    BookmarkService(const BookmarkService&) = delete;
    BookmarkService& operator=(const BookmarkService&) = delete;

    /**
     * @brief 서비스 초기화 (앱 시작 시 북마크/폴더 로드)
     */
    void initialize();

    /**
     * @brief 북마크 추가
     * @param title 제목
     * @param url URL (정규화된 형태)
     * @param folderId 폴더 ID (빈 문자열 = 루트)
     * @return 성공 여부 (중복 URL 시 false)
     */
    bool addBookmark(const QString &title, const QString &url, const QString &folderId = "");

    /**
     * @brief 모든 북마크 조회
     * @return 북마크 배열
     */
    QVector<Bookmark> getAllBookmarks() const;

    /**
     * @brief 폴더별 북마크 조회
     * @param folderId 폴더 ID (빈 문자열 = 루트)
     * @return 북마크 배열
     */
    QVector<Bookmark> getBookmarksByFolder(const QString &folderId) const;

    /**
     * @brief 북마크 조회 (ID로)
     * @param bookmarkId 북마크 ID
     * @return Bookmark 인스턴스 (존재하지 않으면 std::nullopt)
     */
    std::optional<Bookmark> getBookmark(const QString &bookmarkId) const;

    /**
     * @brief 북마크 업데이트
     * @param bookmarkId 북마크 ID
     * @param title 새 제목
     * @param url 새 URL
     * @param folderId 새 폴더 ID
     * @return 성공 여부
     */
    bool updateBookmark(const QString &bookmarkId, const QString &title,
                        const QString &url, const QString &folderId);

    /**
     * @brief 북마크 삭제
     * @param bookmarkId 북마크 ID
     * @return 성공 여부
     */
    bool deleteBookmark(const QString &bookmarkId);

    /**
     * @brief URL 존재 여부 확인 (중복 체크)
     * @param url URL (대소문자 무시)
     * @return 존재하면 true
     */
    bool exists(const QString &url) const;

    /**
     * @brief 폴더 생성
     * @param name 폴더 이름
     * @param parentId 부모 폴더 ID (빈 문자열 = 루트)
     * @return 성공 여부
     */
    bool createFolder(const QString &name, const QString &parentId = "");

    /**
     * @brief 폴더 이름 변경
     * @param folderId 폴더 ID
     * @param newName 새 이름
     * @return 성공 여부
     */
    bool renameFolder(const QString &folderId, const QString &newName);

    /**
     * @brief 폴더 삭제
     * @param folderId 폴더 ID
     * @param moveToRoot true면 하위 북마크를 루트로 이동, false면 함께 삭제
     * @return 성공 여부
     */
    bool deleteFolder(const QString &folderId, bool moveToRoot = false);

    /**
     * @brief 모든 폴더 조회
     * @return 폴더 배열
     */
    QVector<Folder> getAllFolders() const;

    /**
     * @brief 폴더 조회 (ID로)
     * @param folderId 폴더 ID
     * @return Folder 인스턴스 (존재하지 않으면 std::nullopt)
     */
    std::optional<Folder> getFolder(const QString &folderId) const;

signals:
    /**
     * @brief 북마크 추가 완료 시그널
     * @param bookmark 추가된 북마크
     */
    void bookmarkAdded(const Bookmark &bookmark);

    /**
     * @brief 북마크 업데이트 완료 시그널
     * @param bookmark 업데이트된 북마크
     */
    void bookmarkUpdated(const Bookmark &bookmark);

    /**
     * @brief 북마크 삭제 완료 시그널
     * @param bookmarkId 삭제된 북마크 ID
     */
    void bookmarkDeleted(const QString &bookmarkId);

    /**
     * @brief 폴더 추가 완료 시그널
     * @param folder 추가된 폴더
     */
    void folderAdded(const Folder &folder);

    /**
     * @brief 폴더 업데이트 완료 시그널
     * @param folder 업데이트된 폴더
     */
    void folderUpdated(const Folder &folder);

    /**
     * @brief 폴더 삭제 완료 시그널
     * @param folderId 삭제된 폴더 ID
     */
    void folderDeleted(const QString &folderId);

    /**
     * @brief 초기화 완료 시그널
     */
    void initialized();

    /**
     * @brief 에러 시그널
     * @param errorMessage 에러 메시지
     */
    void error(const QString &errorMessage);

private slots:
    /**
     * @brief StorageService::findSuccess 슬롯
     */
    void onFindSuccess(const QJsonArray &results);

    /**
     * @brief StorageService::putSuccess 슬롯
     */
    void onPutSuccess(const QString &id);

    /**
     * @brief StorageService::delSuccess 슬롯
     */
    void onDelSuccess(int count);

    /**
     * @brief StorageService::error 슬롯
     */
    void onStorageError(const QString &errorMessage);

private:
    /**
     * @brief 북마크 데이터를 메모리 캐시에 로드
     */
    void loadBookmarksFromStorage();

    /**
     * @brief 폴더 데이터를 메모리 캐시에 로드
     */
    void loadFoldersFromStorage();

private:
    StorageService *storageService_;       ///< StorageService 인스턴스 (약한 참조)
    QMap<QString, Bookmark> bookmarks_;    ///< 북마크 캐시 (ID -> Bookmark)
    QMap<QString, Folder> folders_;        ///< 폴더 캐시 (ID -> Folder)
    bool initialized_;                     ///< 초기화 완료 플래그
};

} // namespace webosbrowser
```

### 4.4 BookmarkPanel (src/ui/BookmarkPanel.h)

```cpp
#pragma once

#include <QWidget>
#include <QTreeWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <memory>

namespace webosbrowser {

// Forward declarations
class BookmarkService;
struct Bookmark;
struct Folder;

/**
 * @class BookmarkPanel
 * @brief 북마크 목록 표시 및 관리 패널
 *
 * QTreeWidget 기반 폴더 구조 표시 (1단계 서브폴더).
 * 리모컨 방향키 네비게이션, 컨텍스트 메뉴, 검색 기능 제공.
 */
class BookmarkPanel : public QWidget {
    Q_OBJECT

public:
    /**
     * @brief 생성자
     * @param bookmarkService BookmarkService 인스턴스
     * @param parent 부모 위젯
     */
    explicit BookmarkPanel(BookmarkService *bookmarkService, QWidget *parent = nullptr);
    ~BookmarkPanel() override;

    BookmarkPanel(const BookmarkPanel&) = delete;
    BookmarkPanel& operator=(const BookmarkPanel&) = delete;

    /**
     * @brief 패널 표시 (슬라이드 인 애니메이션)
     */
    void show();

    /**
     * @brief 패널 숨김 (슬라이드 아웃 애니메이션)
     */
    void hide();

    /**
     * @brief 북마크 목록 새로고침
     */
    void refresh();

signals:
    /**
     * @brief 북마크 선택 시그널 (페이지 열기)
     * @param url 북마크 URL
     */
    void bookmarkSelected(const QString &url);

    /**
     * @brief 패널 닫기 시그널
     */
    void closed();

protected:
    /**
     * @brief 키 이벤트 처리 (리모컨 입력)
     */
    void keyPressEvent(QKeyEvent *event) override;

    /**
     * @brief 포커스 인 이벤트
     */
    void focusInEvent(QFocusEvent *event) override;

private slots:
    /**
     * @brief 북마크 아이템 더블클릭 핸들러
     */
    void onItemDoubleClicked(QTreeWidgetItem *item, int column);

    /**
     * @brief 북마크 아이템 활성화 핸들러 (Enter 키)
     */
    void onItemActivated(QTreeWidgetItem *item, int column);

    /**
     * @brief 컨텍스트 메뉴 요청 핸들러
     */
    void onContextMenuRequested(const QPoint &pos);

    /**
     * @brief 새 폴더 버튼 클릭 핸들러
     */
    void onNewFolderClicked();

    /**
     * @brief 검색 텍스트 변경 핸들러
     */
    void onSearchTextChanged(const QString &text);

    /**
     * @brief 북마크 편집 액션
     */
    void onEditBookmark();

    /**
     * @brief 북마크 삭제 액션
     */
    void onDeleteBookmark();

    /**
     * @brief 폴더 이름 변경 액션
     */
    void onRenameFolder();

    /**
     * @brief 폴더 삭제 액션
     */
    void onDeleteFolder();

    /**
     * @brief BookmarkService::bookmarkAdded 슬롯
     */
    void onBookmarkAdded(const Bookmark &bookmark);

    /**
     * @brief BookmarkService::bookmarkUpdated 슬롯
     */
    void onBookmarkUpdated(const Bookmark &bookmark);

    /**
     * @brief BookmarkService::bookmarkDeleted 슬롯
     */
    void onBookmarkDeleted(const QString &bookmarkId);

    /**
     * @brief BookmarkService::folderAdded 슬롯
     */
    void onFolderAdded(const Folder &folder);

    /**
     * @brief BookmarkService::folderUpdated 슬롯
     */
    void onFolderUpdated(const Folder &folder);

    /**
     * @brief BookmarkService::folderDeleted 슬롯
     */
    void onFolderDeleted(const QString &folderId);

private:
    /**
     * @brief UI 초기화
     */
    void setupUI();

    /**
     * @brief 시그널/슬롯 연결
     */
    void setupConnections();

    /**
     * @brief 스타일시트 적용
     */
    void applyStyles();

    /**
     * @brief 포커스 순서 설정
     */
    void setupFocusOrder();

    /**
     * @brief 북마크 목록 로드 (QTreeWidget)
     */
    void loadBookmarks();

    /**
     * @brief 폴더 목록 로드 (QTreeWidget 루트 아이템)
     */
    void loadFolders();

    /**
     * @brief 북마크 아이템 생성 (QTreeWidgetItem)
     */
    QTreeWidgetItem* createBookmarkItem(const Bookmark &bookmark);

    /**
     * @brief 폴더 아이템 생성 (QTreeWidgetItem)
     */
    QTreeWidgetItem* createFolderItem(const Folder &folder);

    /**
     * @brief 북마크 검색 필터링
     */
    void filterBookmarks(const QString &searchText);

    /**
     * @brief 컨텍스트 메뉴 생성
     */
    void createContextMenu(QTreeWidgetItem *item, const QPoint &pos);

    /**
     * @brief 북마크 편집 다이얼로그 표시
     */
    void showEditDialog(const QString &bookmarkId);

    /**
     * @brief 폴더 이름 변경 다이얼로그 표시
     */
    void showRenameFolderDialog(const QString &folderId);

private:
    // 서비스
    BookmarkService *bookmarkService_;  ///< BookmarkService 인스턴스 (약한 참조)

    // UI 컴포넌트
    QVBoxLayout *mainLayout_;           ///< 메인 레이아웃
    QHBoxLayout *topLayout_;            ///< 상단 레이아웃 (검색, 버튼)
    QLineEdit *searchEdit_;             ///< 검색 입력 필드
    QPushButton *newFolderButton_;      ///< 새 폴더 버튼
    QTreeWidget *treeWidget_;           ///< 북마크/폴더 트리 위젯
    QLabel *emptyLabel_;                ///< 빈 목록 안내 라벨
    QPushButton *closeButton_;          ///< 닫기 버튼

    // 상태
    QMap<QString, QTreeWidgetItem*> bookmarkItems_;  ///< 북마크 ID -> QTreeWidgetItem
    QMap<QString, QTreeWidgetItem*> folderItems_;    ///< 폴더 ID -> QTreeWidgetItem
    bool isAnimating_;                               ///< 애니메이션 중 플래그
};

} // namespace webosbrowser
```

### 4.5 BookmarkDialog (새 파일, src/ui/BookmarkDialog.h)

```cpp
#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QTextEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QFormLayout>

namespace webosbrowser {

// Forward declaration
class BookmarkService;

/**
 * @class BookmarkDialog
 * @brief 북마크 추가/편집 다이얼로그
 *
 * 제목, URL, 폴더 선택, 설명 입력 필드 제공.
 * URLValidator로 URL 유효성 검증.
 */
class BookmarkDialog : public QDialog {
    Q_OBJECT

public:
    /**
     * @enum Mode
     * @brief 다이얼로그 모드
     */
    enum class Mode {
        Add,    ///< 북마크 추가 모드
        Edit    ///< 북마크 편집 모드
    };

    /**
     * @brief 생성자
     * @param mode 다이얼로그 모드
     * @param bookmarkService BookmarkService 인스턴스
     * @param parent 부모 위젯
     */
    explicit BookmarkDialog(Mode mode, BookmarkService *bookmarkService, QWidget *parent = nullptr);
    ~BookmarkDialog() override;

    BookmarkDialog(const BookmarkDialog&) = delete;
    BookmarkDialog& operator=(const BookmarkDialog&) = delete;

    /**
     * @brief 제목 설정 (편집 모드)
     */
    void setTitle(const QString &title);

    /**
     * @brief URL 설정 (편집 모드)
     */
    void setUrl(const QString &url);

    /**
     * @brief 폴더 ID 설정 (편집 모드)
     */
    void setFolderId(const QString &folderId);

    /**
     * @brief 입력된 제목 반환
     */
    QString title() const;

    /**
     * @brief 입력된 URL 반환
     */
    QString url() const;

    /**
     * @brief 선택된 폴더 ID 반환
     */
    QString folderId() const;

protected:
    /**
     * @brief 키 이벤트 처리 (리모컨 입력)
     */
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    /**
     * @brief URL 텍스트 변경 핸들러 (유효성 검증)
     */
    void onUrlTextChanged(const QString &text);

    /**
     * @brief 확인 버튼 클릭 핸들러
     */
    void onAcceptClicked();

    /**
     * @brief 취소 버튼 클릭 핸들러
     */
    void onRejectClicked();

private:
    /**
     * @brief UI 초기화
     */
    void setupUI();

    /**
     * @brief 시그널/슬롯 연결
     */
    void setupConnections();

    /**
     * @brief 스타일시트 적용
     */
    void applyStyles();

    /**
     * @brief 폴더 ComboBox 로드
     */
    void loadFolders();

    /**
     * @brief 입력 유효성 검증
     */
    bool validate();

private:
    Mode mode_;                         ///< 다이얼로그 모드
    BookmarkService *bookmarkService_;  ///< BookmarkService 인스턴스

    // UI 컴포넌트
    QVBoxLayout *mainLayout_;           ///< 메인 레이아웃
    QFormLayout *formLayout_;           ///< 폼 레이아웃
    QLineEdit *titleEdit_;              ///< 제목 입력
    QLineEdit *urlEdit_;                ///< URL 입력
    QComboBox *folderComboBox_;         ///< 폴더 선택
    QTextEdit *descriptionEdit_;        ///< 설명 입력 (선택)
    QHBoxLayout *buttonLayout_;         ///< 버튼 레이아웃
    QPushButton *acceptButton_;         ///< 확인 버튼
    QPushButton *rejectButton_;         ///< 취소 버튼
    QLabel *errorLabel_;                ///< 에러 메시지 라벨
};

} // namespace webosbrowser
```

---

## 5. 데이터베이스 설계 (LS2 API DB8)

### 5.1 Kind 정의

#### 북마크 Kind
```json
{
  "id": "com.jsong.webosbrowser.native:1",
  "owner": "com.jsong.webosbrowser.native",
  "schema": {
    "type": "object",
    "properties": {
      "id": { "type": "string" },
      "title": { "type": "string" },
      "url": { "type": "string" },
      "folderId": { "type": "string" },
      "favicon": { "type": "string" },
      "createdAt": { "type": "integer" },
      "updatedAt": { "type": "integer" },
      "visitCount": { "type": "integer" }
    },
    "required": ["id", "title", "url", "createdAt"]
  },
  "indexes": [
    {
      "name": "url_index",
      "props": [{ "name": "url" }]
    },
    {
      "name": "folder_index",
      "props": [{ "name": "folderId" }]
    }
  ]
}
```

#### 폴더 Kind
```json
{
  "id": "com.jsong.webosbrowser.native.folder:1",
  "owner": "com.jsong.webosbrowser.native",
  "schema": {
    "type": "object",
    "properties": {
      "id": { "type": "string" },
      "name": { "type": "string" },
      "parentId": { "type": "string" },
      "createdAt": { "type": "integer" }
    },
    "required": ["id", "name", "createdAt"]
  },
  "indexes": [
    {
      "name": "parent_index",
      "props": [{ "name": "parentId" }]
    }
  ]
}
```

### 5.2 LS2 API 쿼리 예시

#### 모든 북마크 조회
```json
{
  "query": {
    "from": "com.jsong.webosbrowser.native:1",
    "limit": 1000
  }
}
```

#### 특정 폴더의 북마크 조회
```json
{
  "query": {
    "from": "com.jsong.webosbrowser.native:1",
    "where": [
      { "prop": "folderId", "op": "=", "val": "folder-uuid-123" }
    ],
    "limit": 1000
  }
}
```

#### URL 중복 체크
```json
{
  "query": {
    "from": "com.jsong.webosbrowser.native:1",
    "where": [
      { "prop": "url", "op": "=", "val": "https://www.youtube.com" }
    ],
    "limit": 1
  }
}
```

### 5.3 인덱스 전략

- **url_index**: URL 중복 체크 쿼리 성능 최적화
- **folder_index**: 폴더별 북마크 조회 성능 최적화
- **parent_index**: 서브폴더 조회 성능 최적화

---

## 6. 시퀀스 다이어그램

### 6.1 북마크 추가 플로우

```
사용자                NavigationBar    BookmarkDialog   BookmarkService   StorageService   LS2 API
  │                         │                │                │                │             │
  │  북마크 버튼 클릭       │                │                │                │             │
  │────────────────────────>│                │                │                │             │
  │                         │ new Dialog()   │                │                │             │
  │                         │───────────────>│                │                │             │
  │                         │                │  setTitle()    │                │             │
  │                         │                │  setUrl()      │                │             │
  │                         │                │◄───────────────│                │             │
  │                         │                │  (WebView에서  │                │             │
  │                         │                │   현재 페이지   │                │             │
  │                         │                │   정보 조회)    │                │             │
  │                         │  show()        │                │                │             │
  │                         │───────────────>│                │                │             │
  │                         │                │                │                │             │
  │  제목 편집, 폴더 선택   │                │                │                │             │
  │────────────────────────────────────────>│                │                │             │
  │                         │                │                │                │             │
  │  확인 버튼 클릭         │                │                │                │             │
  │────────────────────────────────────────>│                │                │             │
  │                         │                │  exists(url)?  │                │             │
  │                         │                │───────────────>│                │             │
  │                         │                │                │  find(url)     │             │
  │                         │                │                │───────────────>│             │
  │                         │                │                │                │ luna-call   │
  │                         │                │                │                │────────────>│
  │                         │                │                │                │◄────────────│
  │                         │                │                │  findSuccess()  │             │
  │                         │                │                │◄───────────────│             │
  │                         │                │  false (중복X) │                │             │
  │                         │                │◄───────────────│                │             │
  │                         │                │                │                │             │
  │                         │                │  addBookmark() │                │             │
  │                         │                │───────────────>│                │             │
  │                         │                │                │  put(json)     │             │
  │                         │                │                │───────────────>│             │
  │                         │                │                │                │ luna-call   │
  │                         │                │                │                │────────────>│
  │                         │                │                │                │◄────────────│
  │                         │                │                │  putSuccess()   │             │
  │                         │                │                │◄───────────────│             │
  │                         │                │                │                │             │
  │                         │                │  emit bookmarkAdded()          │             │
  │                         │                │─────────────> BookmarkPanel    │             │
  │                         │                │                (시그널)         │             │
  │                         │                │                │                │             │
  │  "북마크가 저장되었습니다" (QMessageBox)                 │                │             │
  │◄────────────────────────────────────────│                │                │             │
  │                         │  close()       │                │                │             │
  │                         │───────────────>│                │                │             │
```

### 6.2 북마크 목록 조회 플로우

```
사용자          BookmarkPanel    BookmarkService   StorageService   LS2 API
  │                   │                │                │             │
  │  북마크 패널 열기 │                │                │             │
  │──────────────────>│                │                │             │
  │                   │  show()        │                │             │
  │                   │  loadBookmarks()                │             │
  │                   │───────────────>│                │             │
  │                   │                │  getAllBookmarks()           │
  │                   │                │  (메모리 캐시)  │             │
  │                   │                │◄───────────────│             │
  │                   │  QVector<Bookmark>              │             │
  │                   │◄───────────────│                │             │
  │                   │                │                │             │
  │                   │  loadFolders() │                │             │
  │                   │───────────────>│                │             │
  │                   │                │  getAllFolders()              │
  │                   │                │  (메모리 캐시)  │             │
  │                   │                │◄───────────────│             │
  │                   │  QVector<Folder>                │             │
  │                   │◄───────────────│                │             │
  │                   │                │                │             │
  │  QTreeWidget 렌더링 (폴더 + 북마크) │                │             │
  │◄──────────────────│                │                │             │
  │                   │                │                │             │
  │  방향키로 북마크 탐색 (포커스 이동)│                │             │
  │──────────────────>│                │                │             │
  │                   │                │                │             │
  │  Enter 키 (북마크 선택)             │                │             │
  │──────────────────>│                │                │             │
  │                   │  emit bookmarkSelected(url)     │             │
  │                   │──────────────> BrowserWindow    │             │
  │                   │                (시그널)         │             │
  │                   │                │                │             │
  │  WebView::load(url) → 페이지 로드  │                │             │
  │◄──────────────────────────────────────────────────────────────────│
  │                   │  hide()        │                │             │
  │                   │  (슬라이드 아웃)│                │             │
```

### 6.3 북마크 편집 플로우

```
사용자          BookmarkPanel    BookmarkDialog   BookmarkService   StorageService   LS2 API
  │                   │                │                │                │             │
  │  북마크 우클릭    │                │                │                │             │
  │  (컨텍스트 메뉴)  │                │                │                │             │
  │──────────────────>│                │                │                │             │
  │                   │  QMenu::exec() │                │                │             │
  │                   │  "편집" 선택   │                │                │             │
  │──────────────────>│                │                │                │             │
  │                   │  showEditDialog(id)            │                │             │
  │                   │───────────────>│                │                │             │
  │                   │                │  getBookmark(id)               │             │
  │                   │                │───────────────>│                │             │
  │                   │                │  Bookmark      │                │             │
  │                   │                │◄───────────────│                │             │
  │                   │                │  setTitle()    │                │             │
  │                   │                │  setUrl()      │                │             │
  │                   │                │  setFolderId() │                │             │
  │                   │  show()        │                │                │             │
  │                   │───────────────>│                │                │             │
  │                   │                │                │                │             │
  │  제목 편집, 폴더 변경              │                │                │             │
  │────────────────────────────────────>│                │                │             │
  │                   │                │                │                │             │
  │  확인 버튼 클릭   │                │                │                │             │
  │────────────────────────────────────>│                │                │             │
  │                   │                │  updateBookmark()              │             │
  │                   │                │───────────────>│                │             │
  │                   │                │                │  put(json)     │             │
  │                   │                │                │───────────────>│             │
  │                   │                │                │                │ luna-call   │
  │                   │                │                │                │────────────>│
  │                   │                │                │                │◄────────────│
  │                   │                │                │  putSuccess()   │             │
  │                   │                │                │◄───────────────│             │
  │                   │                │  emit bookmarkUpdated()        │             │
  │                   │                │────────────────> BookmarkPanel │             │
  │                   │                │                (시그널)         │             │
  │                   │  refresh()     │                │                │             │
  │                   │  (QTreeWidget 업데이트)         │                │             │
  │◄──────────────────│                │                │                │             │
  │                   │  "북마크가 수정되었습니다"      │                │             │
  │◄──────────────────────────────────────────────────────────────────────────────────│
```

### 6.4 LS2 API 비동기 호출 패턴

```
BookmarkService        StorageService         LS2 API (luna-service2)
       │                      │                           │
       │  put(json)           │                           │
       │─────────────────────>│                           │
       │                      │  luna-send -n 1 -a ...   │
       │                      │  com.webos.service.db/put │
       │                      │──────────────────────────>│
       │  (Qt 이벤트 루프 반환, 메인 스레드 블로킹 X)    │
       │                      │                           │
       │                      │  (비동기 응답 대기...)    │
       │                      │                           │
       │                      │  {"returnValue": true, "id": "..."}
       │                      │◄──────────────────────────│
       │  emit putSuccess(id) │                           │
       │◄─────────────────────│                           │
       │                      │                           │
       │  onPutSuccess(id)    │                           │
       │  (슬롯 함수 호출)    │                           │
       │  emit bookmarkAdded()│                           │
       │──────────────────────> BookmarkPanel (시그널)    │
```

---

## 7. UI 설계

### 7.1 BookmarkPanel 레이아웃 (Qt Widgets)

```
┌───────────────────────────────────────────────────────┐
│  북마크 관리                                    [ X ] │  ← QLabel (제목) + QPushButton (닫기)
├───────────────────────────────────────────────────────┤
│  [ 검색: _____________________ ]  [ 새 폴더 ]        │  ← QLineEdit + QPushButton
├───────────────────────────────────────────────────────┤
│  QTreeWidget (폴더 + 북마크)                          │
│  ├─ 📁 엔터테인먼트                                   │
│  │   ├─ 🌐 YouTube (https://www.youtube.com)        │
│  │   └─ 🌐 Netflix (https://www.netflix.com)        │
│  ├─ 📁 뉴스                                           │
│  │   └─ 🌐 BBC (https://www.bbc.com)                │
│  └─ 🌐 Google (https://www.google.com) ← 루트 북마크 │
│                                                       │
│                                                       │
│  (리모컨 방향키: ↑↓ 북마크 탐색, ←→ 폴더 접기/펼치기) │
│  (Enter: 페이지 열기, Options: 컨텍스트 메뉴)         │
└───────────────────────────────────────────────────────┘
                        600px × 800px (대화면 최적화)
```

### 7.2 BookmarkDialog 레이아웃 (추가/편집)

```
┌───────────────────────────────────────┐
│  북마크 추가 / 편집                   │
├───────────────────────────────────────┤
│  제목:   [ YouTube_____________ ]     │  ← QLineEdit
│  URL:    [ https://youtube.com_ ]     │  ← QLineEdit (편집 모드는 수정 가능)
│  폴더:   [ 엔터테인먼트  ▼ ]          │  ← QComboBox (드롭다운)
│  설명:   [ ___________________ ]      │  ← QTextEdit (선택)
│          [ ___________________ ]      │
│                                       │
│  [ 에러 메시지 라벨 (빨강 텍스트) ]  │  ← QLabel (유효성 검증 에러)
│                                       │
│           [ 확인 ]  [ 취소 ]         │  ← QPushButton
└───────────────────────────────────────┘
             400px × 400px
```

### 7.3 컨텍스트 메뉴 (QMenu)

```
┌────────────────────┐
│ ► 페이지 열기      │  ← 북마크 실행
│ ► 새 탭에서 열기   │  ← 새 탭으로 열기 (F-06 연동)
│ ───────────────    │
│ ✎ 편집             │  ← BookmarkDialog 표시
│ 🗑 삭제             │  ← 삭제 확인 후 삭제
│ ───────────────    │
│ 📁 폴더 이름 변경  │  ← 폴더 항목일 경우만 표시
│ 🗑 폴더 삭제        │  ← 폴더 항목일 경우만 표시
└────────────────────┘
```

### 7.4 리모컨 포커스 흐름

```
NavigationBar (북마크 버튼)
        │
        ▼ (Enter 키)
BookmarkPanel (표시)
        │
        ▼ (방향키 ↓)
searchEdit_ (검색 입력 필드)
        │
        ▼ (방향키 ↓)
treeWidget_ (북마크 목록)
        │
        ├─ (방향키 ↑↓) 북마크 항목 탐색
        ├─ (방향키 ←) 폴더 접기
        ├─ (방향키 →) 폴더 펼치기
        ├─ (Enter 키) 북마크 실행 (페이지 열기)
        ├─ (Options 키) 컨텍스트 메뉴
        └─ (Delete 키) 북마크 삭제
        │
        ▼ (Escape/Back 버튼)
BookmarkPanel 닫기 → BrowserWindow
```

### 7.5 Qt Stylesheet (스타일링)

```cpp
// BookmarkPanel.cpp의 applyStyles() 메서드
void BookmarkPanel::applyStyles() {
    // 전체 패널 배경
    setStyleSheet(R"(
        BookmarkPanel {
            background-color: #2b2b2b;
            border: 1px solid #444;
        }

        /* 검색 입력 필드 */
        QLineEdit {
            background-color: #3c3c3c;
            color: #ffffff;
            border: 2px solid #555;
            border-radius: 5px;
            padding: 10px;
            font-size: 18px;
        }

        QLineEdit:focus {
            border: 3px solid #0078d7;
        }

        /* 버튼 */
        QPushButton {
            background-color: #0078d7;
            color: #ffffff;
            border: none;
            border-radius: 5px;
            padding: 12px 24px;
            font-size: 18px;
            font-weight: bold;
        }

        QPushButton:hover {
            background-color: #005ba1;
        }

        QPushButton:focus {
            border: 3px solid #ffffff;
        }

        /* 트리 위젯 */
        QTreeWidget {
            background-color: #1e1e1e;
            color: #ffffff;
            border: 1px solid #444;
            font-size: 20px;
        }

        QTreeWidget::item {
            padding: 12px;
            border-bottom: 1px solid #333;
        }

        QTreeWidget::item:selected {
            background-color: #0078d7;
            color: #ffffff;
        }

        QTreeWidget::item:focus {
            border: 3px solid #ffffff;
            background-color: #005ba1;
        }

        QTreeWidget::item:hover {
            background-color: #3c3c3c;
        }

        /* 빈 목록 라벨 */
        QLabel#emptyLabel {
            color: #888;
            font-size: 24px;
            padding: 50px;
        }
    )");
}
```

---

## 8. 파일 구조 및 의존성

### 8.1 새로 생성할 파일

```
src/
├── models/
│   └── Bookmark.h                    # 북마크/폴더 구조체 정의 (헤더 온리)
├── services/
│   ├── BookmarkService.h             # 북마크 서비스 헤더 (확장)
│   ├── BookmarkService.cpp           # 북마크 서비스 구현
│   ├── StorageService.h              # 스토리지 서비스 헤더 (확장)
│   └── StorageService.cpp            # 스토리지 서비스 구현 (LS2 API 래퍼)
├── ui/
│   ├── BookmarkPanel.h               # 북마크 패널 헤더 (확장)
│   ├── BookmarkPanel.cpp             # 북마크 패널 구현
│   ├── BookmarkDialog.h              # 북마크 다이얼로그 헤더 (새 파일)
│   └── BookmarkDialog.cpp            # 북마크 다이얼로그 구현 (새 파일)
```

### 8.2 수정할 기존 파일

```
src/
├── browser/
│   ├── BrowserWindow.h               # BookmarkPanel 포인터 추가
│   └── BrowserWindow.cpp             # BookmarkPanel 생성 및 시그널 연결
├── ui/
│   ├── NavigationBar.h               # 북마크 버튼 추가
│   └── NavigationBar.cpp             # 북마크 버튼 클릭 이벤트 핸들러
webos-meta/
└── appinfo.json                      # LS2 API 권한 추가 (db8.find, db8.put, db8.del)
CMakeLists.txt                        # 새 소스 파일 추가
```

### 8.3 의존성 그래프

```
BrowserWindow
    │
    ├── WebView (현재 페이지 정보 조회)
    │
    ├── NavigationBar (북마크 버튼)
    │
    ├── BookmarkPanel (북마크 목록 UI)
    │       │
    │       ├── BookmarkDialog (추가/편집 UI)
    │       │
    │       └── BookmarkService (비즈니스 로직)
    │               │
    │               ├── StorageService (LS2 API 래퍼)
    │               │
    │               └── URLValidator (URL 검증)
    │
    └── TabManager (새 탭에서 열기, F-06 연동)
```

---

## 9. 기술 스택 확정

### 9.1 Qt 컴포넌트

| 컴포넌트 | 용도 | 비고 |
|---------|------|------|
| **QTreeWidget** | 북마크/폴더 목록 표시 | 계층 구조 시각화 (1단계 서브폴더) |
| **QListWidget** | (대안) 단순 북마크 목록 | 폴더 기능 미사용 시 |
| **QTreeWidgetItem** | 북마크/폴더 항목 | 아이콘, 제목, URL 표시 |
| **QDialog** | 북마크 추가/편집 다이얼로그 | 모달 팝업 |
| **QInputDialog** | 폴더 이름 입력 | 간단한 텍스트 입력 |
| **QMessageBox** | 에러, 확인 메시지 | 삭제 확인, 중복 경고 |
| **QLineEdit** | 검색, 제목, URL 입력 | Qt::ReadOnly 옵션 (URL) |
| **QComboBox** | 폴더 선택 드롭다운 | 폴더 목록 표시 |
| **QPushButton** | 버튼 (추가, 편집, 삭제 등) | 리모컨 포커스 지원 |
| **QVBoxLayout** | 세로 레이아웃 | 패널 메인 레이아웃 |
| **QHBoxLayout** | 가로 레이아웃 | 상단 버튼 배치 |
| **QFormLayout** | 폼 레이아웃 | 다이얼로그 입력 필드 |
| **QPropertyAnimation** | 슬라이드 인/아웃 애니메이션 | 패널 표시/숨김 |
| **QMenu** | 컨텍스트 메뉴 | 우클릭 메뉴 |

### 9.2 Qt5 모듈

```cmake
# CMakeLists.txt에 추가
find_package(Qt5 REQUIRED COMPONENTS
    Core          # QObject, QString, QVector, QMap
    Gui           # QIcon, QKeyEvent
    Widgets       # QWidget, QTreeWidget, QDialog
)
```

### 9.3 C++17 표준 라이브러리

- **`std::optional`**: 북마크/폴더 조회 결과 (존재하지 않을 수 있음)
- **`std::unique_ptr`**: 메모리 관리 (PIMPL 패턴)
- **`std::shared_ptr`**: (필요시) 여러 객체 간 공유
- **`QScopedPointer`**: Qt 스마트 포인터 (PIMPL 패턴)

### 9.4 webOS LS2 API

| API | 용도 | 호출 방식 |
|-----|------|----------|
| **com.webos.service.db/put** | 북마크/폴더 저장 | luna-send (비동기) |
| **com.webos.service.db/find** | 북마크/폴더 조회 | luna-send (비동기) |
| **com.webos.service.db/del** | 북마크/폴더 삭제 | luna-send (비동기) |
| **com.webos.service.db/putKind** | Kind 등록 (최초 1회) | luna-send (동기, 설치 시) |

---

## 10. 위험 요소 및 완화 방안

### 10.1 LS2 API 에러 처리

**위험**: LS2 API 호출 실패 시 앱 동작 중단 또는 데이터 손실

**완화 방안**:
1. StorageService::error 시그널 → BookmarkService::onStorageError 슬롯
2. 에러 메시지 로깅 (Logger::error())
3. 사용자에게 QMessageBox로 에러 표시 및 재시도 옵션 제공
4. 메모리 캐시 유지 (LS2 API 실패해도 세션 중 데이터는 유지)
5. 타임아웃 설정 (LS2 API 응답 5초 이내, 실패 시 에러 시그널)

### 10.2 메모리 누수

**위험**: Qt 객체 메모리 관리 실수로 누수 발생

**완화 방안**:
1. Qt 부모-자식 메모리 관리 활용 (QObject 생성자에 parent 전달)
2. QScopedPointer, std::unique_ptr 사용 (수동 delete 금지)
3. QTreeWidgetItem은 QTreeWidget에 추가 시 자동 소유권 이전 (수동 delete 불필요)
4. 시그널/슬롯 연결 해제 (객체 파괴 전 disconnect 호출, 필요시)
5. Valgrind 또는 Qt Creator 메모리 프로파일러로 검증

### 10.3 UI 블로킹 (비동기 처리)

**위험**: LS2 API 동기 호출 시 메인 스레드 블로킹 → UI 프리즈

**완화 방안**:
1. LS2 API는 항상 비동기 호출 (luna-send -n 1)
2. Qt 이벤트 루프로 응답 대기 (블로킹 없음)
3. 로딩 시 LoadingIndicator 표시 (사용자 피드백)
4. 타임아웃 설정 (5초 이내 응답 없으면 에러 처리)

### 10.4 북마크 중복 데이터

**위험**: 동시 추가 요청 시 URL 중복 발생 (race condition)

**완화 방안**:
1. BookmarkService::addBookmark()에서 중복 체크 선행 (exists() 호출)
2. LS2 API 응답 수신 후 메모리 캐시 업데이트 (낙관적 업데이트 금지)
3. DB8 인덱스 (url_index)로 중복 쿼리 성능 최적화
4. 사용자에게 "이미 북마크에 추가됨" QMessageBox 표시

### 10.5 폴더 삭제 시 하위 북마크 처리

**위험**: 폴더 삭제 시 하위 북마크 고아 데이터 발생

**완화 방안**:
1. BookmarkService::deleteFolder(folderId, moveToRoot) 메서드
2. moveToRoot == true이면 하위 북마크의 folderId를 빈 문자열로 업데이트
3. moveToRoot == false이면 하위 북마크도 함께 삭제 (cascade delete)
4. 삭제 전 QMessageBox 경고 ("폴더 내 북마크 X개도 삭제됩니다")
5. 체크박스 옵션 제공 ("북마크를 루트 폴더로 이동")

---

## 11. 성능 고려사항

### 11.1 북마크 로딩 시간

**목표**: 100개 북마크 기준 1초 이내

**최적화 전략**:
1. 메모리 캐시 사용 (LS2 API는 앱 초기화 시 1회만 호출)
2. QMap<QString, Bookmark> 자료구조 (O(log n) 검색)
3. LS2 API 쿼리에 인덱스 활용 (url_index, folder_index)
4. 북마크 1000개 이상 시 페이지네이션 (limit: 100, offset: 0) 고려

### 11.2 캐싱 전략

**레이어별 캐싱**:
1. **BookmarkService**: QMap 메모리 캐시 (bookmarks_, folders_)
   - 앱 시작 시 LS2 API에서 로드 → 메모리 캐시
   - 추가/편집/삭제 시 LS2 API + 메모리 캐시 동시 업데이트
   - 메모리 캐시 읽기 (O(log n)), LS2 API는 쓰기 전용
2. **BookmarkPanel**: QTreeWidget 캐시
   - BookmarkService에서 데이터 조회 → QTreeWidgetItem 생성
   - 시그널 수신 시 증분 업데이트 (전체 새로고침 금지)

**캐시 무효화**:
- BookmarkService::bookmarkAdded/Updated/Deleted 시그널 → BookmarkPanel 증분 업데이트
- 앱 재시작 시 메모리 캐시 초기화 → LS2 API 재로드

### 11.3 대량 북마크 처리

**시나리오**: 북마크 1000개 이상

**최적화 전략**:
1. **QTreeWidget 가상화**:
   - QTreeWidget::setUniformItemSizes(true) 활성화
   - QAbstractItemView::ScrollPerPixel 사용
   - 가시 영역만 렌더링 (Qt 자동 최적화)
2. **LS2 API 페이지네이션**:
   - 쿼리 limit: 100, offset: 0 (첫 페이지)
   - 스크롤 시 다음 페이지 로드 (무한 스크롤)
3. **검색 필터링**:
   - QLineEdit::textChanged → 메모리 캐시에서 필터링
   - QString::contains() 사용 (대소문자 무시)
   - 필터링된 결과만 QTreeWidget 표시

### 11.4 메모리 사용량

**목표**: BookmarkPanel + BookmarkService 최대 50MB

**측정 방법**:
- Qt Creator 메모리 프로파일러
- /proc/[pid]/status (webOS 디바이스)

**최적화**:
- 북마크 1000개 × 1KB (평균) = 1MB (캐시)
- QTreeWidgetItem: 평균 2KB × 1000개 = 2MB
- Qt Widgets 오버헤드: ~10MB
- 여유 메모리: ~37MB (충분)

---

## 12. 영향 범위 분석

### 12.1 수정 필요한 기존 파일

| 파일 | 변경 내용 | 이유 |
|------|-----------|------|
| `src/browser/BrowserWindow.h` | BookmarkPanel 포인터 추가, BookmarkService 포인터 추가 | 패널 생성 및 시그널 연결 |
| `src/browser/BrowserWindow.cpp` | BookmarkPanel/BookmarkService 생성, setupConnections()에서 시그널 연결 | 북마크 선택 시 WebView::load() 호출 |
| `src/ui/NavigationBar.h` | QPushButton *bookmarkButton_ 추가 | 북마크 버튼 추가 |
| `src/ui/NavigationBar.cpp` | 북마크 버튼 생성, 클릭 시 BookmarkDialog 표시 또는 BookmarkPanel 표시 시그널 emit | 북마크 추가 진입점 |
| `webos-meta/appinfo.json` | requiredPermissions에 "db8.find", "db8.put", "db8.del" 추가 | LS2 API 권한 |
| `CMakeLists.txt` | BookmarkService.cpp, BookmarkPanel.cpp, BookmarkDialog.cpp, Bookmark.h 추가 | 빌드 설정 |

### 12.2 새로 생성할 파일

| 파일 | 역할 | 의존성 |
|------|------|--------|
| `src/models/Bookmark.h` | 북마크/폴더 구조체 정의 (헤더 온리) | Qt Core (QString, QJsonObject) |
| `src/services/StorageService.h/.cpp` | LS2 API 래퍼 (put, find, del) | Qt Core, luna-service2 |
| `src/services/BookmarkService.h/.cpp` | 북마크 비즈니스 로직 (CRUD, 폴더 관리) | StorageService, Bookmark.h |
| `src/ui/BookmarkPanel.h/.cpp` | 북마크 목록 UI (QTreeWidget) | BookmarkService, Qt Widgets |
| `src/ui/BookmarkDialog.h/.cpp` | 북마크 추가/편집 다이얼로그 | BookmarkService, URLValidator |

### 12.3 영향 받는 기존 기능

| 기능 | 영향 내용 | 대응 방안 |
|------|-----------|-----------|
| **F-02 (웹뷰 통합)** | BookmarkPanel에서 WebView::load() 호출 | BrowserWindow에서 시그널 연결 (bookmarkSelected → WebView::load) |
| **F-03 (URL 입력 UI)** | URLBar의 가상 키보드 재사용 가능 | BookmarkDialog에서 URLBar::showVirtualKeyboard() 호출 (선택) |
| **F-05 (로딩 인디케이터)** | 북마크 실행 시 LoadingIndicator 표시 | WebView::loadStarted 시그널 → LoadingIndicator::show() (기존 연결 재사용) |
| **F-06 (탭 관리)** | "새 탭에서 열기" 컨텍스트 메뉴 | TabManager::createTab() 호출 |
| **F-08 (히스토리 관리)** | 북마크 실행 시 히스토리 기록 | WebView::loadFinished 시그널 → HistoryService::addHistory() (F-08 구현 시) |

---

## 13. 기술적 주의사항

### 13.1 보안 주의사항

1. **URL 검증**:
   - 북마크 추가/편집 시 URLValidator::isValid() 필수 호출
   - 잘못된 URL (javascript:, data: 등) 차단
   - XSS 방지: 북마크 제목/URL에 HTML 태그 이스케이프 (QString::toHtmlEscaped())
2. **LS2 API 권한**:
   - appinfo.json에 db8 권한 명시 (db8.find, db8.put, db8.del)
   - 앱 샌드박스 내에서만 동작 (다른 앱 데이터 접근 불가)
3. **SQL 인젝션 방지**:
   - LS2 API는 JSON 기반이므로 SQL 인젝션 없음
   - 단, URL/제목에 JSON 특수문자 ("', {}, []) 포함 시 이스케이프 필수 (QJsonObject 자동 처리)

### 13.2 리모컨 키 매핑 충돌

**문제**: webOS 리모컨 컬러 버튼이 다른 앱/기능과 충돌 가능성

**해결**:
1. 리모컨 키 이벤트는 Qt::Key_* 코드로 처리 (webOS 키코드 매핑)
2. NavigationBar에서 처리되지 않은 키 이벤트는 BookmarkPanel로 전달 (QWidget::event() 오버라이드)
3. 컬러 버튼 매핑은 사용자 설정에서 변경 가능 (향후 확장)

### 13.3 동시성 문제 (Race Condition)

**시나리오**: 사용자가 북마크 추가 중 앱 종료 시

**해결**:
1. LS2 API 응답 수신 전 앱 종료 시 데이터 손실 가능
2. BrowserWindow::closeEvent() 오버라이드 → 미완료 LS2 API 호출 대기 (5초 타임아웃)
3. BookmarkService::hasPendingOperations() 메서드 추가 (플래그 관리)
4. 앱 종료 전 QMessageBox 경고 ("북마크 저장 중입니다. 잠시만 기다려주세요")

### 13.4 폴더 구조 깊이 제한

**현재 제한**: 1단계 서브폴더만 지원

**향후 확장 시 고려사항**:
1. BookmarkService::createFolder(name, parentId) → 재귀적 폴더 ID 검증
2. QTreeWidget → 재귀적 QTreeWidgetItem::addChild() 호출
3. 최대 깊이 제한 (예: 5단계) → 무한 재귀 방지
4. LS2 API 쿼리 복잡도 증가 (재귀 쿼리 필요)

---

## 14. 변경 이력

| 날짜 | 변경 내용 | 이유 |
|------|-----------|------|
| 2026-02-14 | 최초 작성 (Web App PoC → Native App 전환) | 요구사항 분석서 기반 C++/Qt Native App 설계 완료 |
