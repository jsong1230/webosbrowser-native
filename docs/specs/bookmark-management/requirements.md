# 북마크 관리 — 요구사항 분석서

## 1. 개요

### 기능명
북마크 관리 (F-07)

### 목적
사용자가 자주 방문하는 웹사이트를 북마크로 저장하고 관리할 수 있는 기능을 제공하여, 리모컨만으로도 빠르고 편리하게 저장된 사이트에 접근할 수 있도록 합니다. C++/Qt 기반 Native App으로 구현하여 webOS 프로젝터 환경에서 최적의 성능과 UX를 제공합니다.

### 대상 사용자
- 자주 방문하는 웹사이트(YouTube, Netflix, 뉴스 사이트 등)를 빠르게 재방문하고자 하는 프로젝터 사용자
- 리모컨으로 URL을 반복 입력하는 불편함을 줄이고자 하는 일반 사용자
- 북마크를 폴더별로 체계적으로 관리하고자 하는 조직적인 사용자

### 요청 배경
- 리모컨 가상 키보드로 URL을 입력하는 것은 시간이 오래 걸리고 불편함
- 자주 방문하는 사이트를 북마크로 저장하면 2~3번의 버튼 클릭만으로 접근 가능
- 북마크는 웹 브라우저의 핵심 기능으로, 사용자 경험 향상에 필수적
- 대화면 환경에서 북마크 목록을 탐색하고 관리하기 위한 리모컨 최적화 Qt UI 필요
- webOS LS2 API를 통한 영속적 데이터 저장 및 시스템 통합

---

## 2. 기능 요구사항 (Functional Requirements)

### FR-1: 북마크 추가
- **설명**: 현재 페이지를 북마크로 저장합니다.
- **유저 스토리**: As a 사용자, I want 현재 보고 있는 페이지를 북마크로 저장할 수 있기를 원합니다, so that 나중에 빠르게 재방문할 수 있습니다.
- **우선순위**: Must
- **상세 요구사항**:
  - 북마크 추가 버튼 (NavigationBar에 Qt QPushButton으로 배치)
  - 버튼 클릭 시 북마크 추가 QDialog 표시
  - 다이얼로그 입력 항목 (Qt Widgets):
    - **제목** (QLineEdit): 현재 페이지 타이틀 자동 입력, 사용자 편집 가능
    - **URL** (QLineEdit): 현재 페이지 URL (읽기 전용, Qt::ReadOnly)
    - **폴더 선택** (QComboBox): 저장할 폴더 선택 (기본: "루트 폴더")
    - **설명** (QTextEdit, 선택): 북마크 설명 메모 (선택적 기능)
  - 확인 버튼 (QPushButton)으로 저장, 취소 버튼으로 닫기
  - 저장 성공 시 상태바 메시지 또는 Qt Notification 표시 ("북마크가 저장되었습니다")
  - 이미 북마크된 페이지일 경우 QMessageBox 경고 ("이미 북마크에 추가된 페이지입니다")
  - BookmarkService::addBookmark() 호출 → LS2 API로 데이터 저장

### FR-2: 북마크 목록 조회
- **설명**: 저장된 북마크를 리스트 형태로 조회합니다.
- **유저 스토리**: As a 사용자, I want 저장된 북마크 목록을 탐색할 수 있기를 원합니다, so that 원하는 사이트를 찾아 접근할 수 있습니다.
- **우선순위**: Must
- **상세 요구사항**:
  - BookmarkPanel 컴포넌트 (QWidget 기반 패널)
  - 북마크 메뉴 버튼으로 패널 열기 (NavigationBar 또는 리모컨 단축키)
  - 북마크 리스트 표시 (QListWidget 또는 QTreeWidget):
    - 각 항목 (QListWidgetItem): 파비콘 (QIcon, 선택), 제목 (QString), URL (서브 텍스트)
    - 폴더 구조 계층형 트리 뷰 (QTreeWidget)
    - 리모컨 방향키로 목록 스크롤 (Qt 포커스 시스템)
  - 포커스된 북마크 하이라이트 표시 (Qt Stylesheet 또는 QStyle)
  - 빈 북마크 목록일 경우 QLabel로 "저장된 북마크가 없습니다" 표시
  - 백 버튼 또는 ESC 키로 패널 닫기 (QWidget::close())
  - BookmarkService::getAllBookmarks() 호출 → LS2 API에서 데이터 로드

### FR-3: 북마크 실행 (페이지 열기)
- **설명**: 북마크 목록에서 북마크를 선택하면 해당 페이지를 WebView에서 로드합니다.
- **유저 스토리**: As a 사용자, I want 북마크를 선택하면 해당 페이지가 즉시 열리기를 원합니다, so that URL을 다시 입력하지 않아도 됩니다.
- **우선순위**: Must
- **상세 요구사항**:
  - 북마크 항목 더블클릭 또는 선택 버튼 (Enter 키) 시 해당 URL로 페이지 로드
  - 새 탭에서 열기 옵션 제공 (컨텍스트 메뉴 또는 Ctrl+클릭)
  - 로드 시작 시 LoadingIndicator 표시 (F-05와 연계)
  - 로드 성공 시 BookmarkPanel 자동 닫기 (설정 가능)
  - 로드 실패 시 QMessageBox 에러 메시지 표시 (F-10과 연계)
  - WebView::load(QUrl) 호출 또는 emit bookmarkSelected(url) 시그널

### FR-4: 북마크 편집
- **설명**: 기존 북마크의 제목, URL, 폴더를 수정합니다.
- **유저 스토리**: As a 사용자, I want 북마크의 제목이나 위치를 변경할 수 있기를 원합니다, so that 북마크를 더 체계적으로 관리할 수 있습니다.
- **우선순위**: Must
- **상세 요구사항**:
  - 북마크 항목 우클릭 또는 컨텍스트 메뉴 버튼 (Options 키) → "편집" 메뉴 항목
  - 북마크 편집 QDialog 표시:
    - 제목 (QLineEdit), URL (QLineEdit), 폴더 선택 (QComboBox)
    - 기존 값 자동 입력됨 (QString으로 전달)
  - 확인 버튼으로 저장 (BookmarkService::updateBookmark()), 취소 버튼으로 닫기
  - 저장 성공 시 상태바 메시지 ("북마크가 수정되었습니다")
  - URL 변경 시 유효성 검증 (URLValidator::isValid())
  - LS2 API로 업데이트된 데이터 저장

### FR-5: 북마크 삭제
- **설명**: 북마크 목록에서 선택한 북마크를 삭제합니다.
- **유저 스토리**: As a 사용자, I want 더 이상 필요하지 않은 북마크를 삭제할 수 있기를 원합니다, so that 북마크 목록을 깔끔하게 유지할 수 있습니다.
- **우선순위**: Must
- **상세 요구사항**:
  - 북마크 항목 컨텍스트 메뉴 → "삭제" 메뉴 항목 또는 Delete 키
  - 삭제 확인 QMessageBox 표시 ("북마크를 삭제하시겠습니까?")
  - 확인 버튼으로 삭제 (BookmarkService::deleteBookmark()), 취소 버튼으로 닫기
  - 삭제 성공 시 QListWidget에서 항목 즉시 제거, 상태바 메시지 표시 ("북마크가 삭제되었습니다")
  - 삭제 후 포커스는 이전 항목 또는 다음 항목으로 이동 (Qt::Key_Up/Down 이벤트)
  - LS2 API로 데이터 삭제 (비동기 호출)

### FR-6: 폴더 관리 (생성, 이름 변경, 삭제)
- **설명**: 북마크를 폴더별로 분류하여 관리합니다.
- **유저 스토리**: As a 사용자, I want 북마크를 폴더로 구분하여 저장할 수 있기를 원합니다, so that 카테고리별로 북마크를 정리할 수 있습니다.
- **우선순위**: Should
- **상세 요구사항**:
  - **폴더 생성**:
    - "새 폴더" QPushButton (BookmarkPanel 상단)
    - 폴더 생성 QInputDialog: 폴더 이름 입력
    - 폴더는 루트 또는 다른 폴더 내에 생성 가능 (1단계 서브폴더만 지원)
    - BookmarkService::createFolder(QString name, QString parentId)
  - **폴더 이름 변경**:
    - 폴더 항목 컨텍스트 메뉴 → "이름 변경"
    - QInputDialog로 새 이름 입력
    - BookmarkService::renameFolder(QString folderId, QString newName)
  - **폴더 삭제**:
    - 폴더 항목 컨텍스트 메뉴 → "삭제"
    - 폴더 내 북마크가 있을 경우 QMessageBox 경고 ("폴더 내 북마크도 삭제됩니다")
    - 확인 시 폴더 및 하위 북마크 모두 삭제 (BookmarkService::deleteFolder())
  - **폴더 구조**:
    - 루트 폴더 (기본 위치, parentId == nullptr)
    - 서브폴더 (최대 1단계 깊이)
    - QTreeWidget으로 계층 구조 표시 (QTreeWidgetItem::addChild())
  - **폴더 탐색**:
    - 폴더 선택 시 해당 폴더 내 북마크 표시 (QTreeWidget::clicked 시그널)
    - 백 버튼으로 상위 폴더로 이동 (QTreeWidget::currentItem()->parent())

### FR-7: 북마크 정렬 및 검색
- **설명**: 북마크 목록을 정렬하거나 검색하여 원하는 북마크를 빠르게 찾습니다.
- **유저 스토리**: As a 사용자, I want 북마크 목록에서 빠르게 원하는 북마크를 찾을 수 있기를 원합니다, so that 많은 북마크 중에서도 쉽게 탐색할 수 있습니다.
- **우선순위**: Could (선택적 기능)
- **상세 요구사항**:
  - **정렬 옵션**:
    - QComboBox 드롭다운: "이름순", "추가된 날짜순", "방문 빈도순"
    - QListWidget::sortItems() 또는 BookmarkService에서 정렬된 데이터 반환
  - **검색 기능**:
    - BookmarkPanel 상단에 QLineEdit 검색 입력 필드
    - 리모컨 가상 키보드로 검색어 입력
    - QLineEdit::textChanged 시그널로 실시간 필터링
    - 제목 또는 URL에서 일치하는 북마크 필터링 (QString::contains())
    - 검색 결과 QListWidget 업데이트

### FR-8: 북마크 데이터 영속성 (LS2 API 저장)
- **설명**: 북마크 데이터를 webOS LS2 API를 통해 저장하여 앱 재시작 후에도 유지됩니다.
- **유저 스토리**: As a 사용자, I want 저장한 북마크가 앱을 종료해도 사라지지 않기를 원합니다, so that 다음에 앱을 실행할 때도 북마크를 사용할 수 있습니다.
- **우선순위**: Must
- **상세 요구사항**:
  - BookmarkService 클래스: StorageService를 통해 LS2 API 호출
  - 북마크 데이터 구조 (C++ struct):
    ```cpp
    struct Bookmark {
        QString id;           // UUID (QUuid::createUuid())
        QString title;        // 북마크 제목
        QString url;          // 북마크 URL
        QString folderId;     // 폴더 ID (루트는 빈 문자열)
        QString favicon;      // 파비콘 URL (선택)
        qint64 createdAt;     // Unix timestamp (QDateTime::currentMSecsSinceEpoch())
        qint64 updatedAt;     // Unix timestamp
        int visitCount;       // 방문 횟수 (선택)
    };
    ```
  - 폴더 데이터 구조 (C++ struct):
    ```cpp
    struct Folder {
        QString id;           // UUID
        QString name;         // 폴더 이름
        QString parentId;     // 부모 폴더 ID (루트는 빈 문자열)
        qint64 createdAt;     // Unix timestamp
    };
    ```
  - LS2 API 저장 형식 (JSON):
    - StorageService::call("luna://com.webos.service.db/put", QJsonObject)
    - 데이터베이스 이름: "com.jsong.webosbrowser.native.bookmarks"
    - Kind: "com.jsong.webosbrowser.native:1" (북마크), "com.jsong.webosbrowser.native.folder:1" (폴더)
  - 앱 초기화 시 북마크 및 폴더 데이터 로드:
    - BookmarkService::init() → StorageService::call("luna://com.webos.service.db/find")
    - QJsonArray를 Bookmark/Folder 구조체 배열로 변환
  - 비동기 LS2 API 호출 처리 (Qt Signal/Slot):
    - StorageService::dataLoaded(QJsonObject) 시그널
    - BookmarkService::onDataLoaded(QJsonObject) 슬롯

### FR-9: 리모컨 키 매핑
- **설명**: webOS 리모컨 물리 버튼을 북마크 기능에 매핑합니다.
- **유저 스토리**: As a 리모컨 사용자, I want 리모컨 버튼만으로 북마크 관리의 모든 기능을 사용할 수 있기를 원합니다, so that 마우스 없이 편리하게 북마크를 관리할 수 있습니다.
- **우선순위**: Must
- **상세 요구사항**:
  - Qt 키 이벤트 처리 (QWidget::keyPressEvent()):
    - Qt::Key_Up/Down: 북마크 목록 스크롤 (QListWidget::setCurrentRow())
    - Qt::Key_Left: 상위 폴더로 이동 (QTreeWidget::currentItem()->parent())
    - Qt::Key_Right: 폴더 열기 (QTreeWidget::expandItem())
    - Qt::Key_Return/Enter: 북마크 실행 (페이지 열기)
    - Qt::Key_Escape/Back: BookmarkPanel 닫기
    - Qt::Key_Delete: 북마크 삭제
  - 리모컨 컬러 버튼 매핑 (webOS 리모컨 키코드):
    - 빨강 (0x193): 북마크 추가 (현재 페이지)
    - 파랑 (0x195): 북마크 편집
    - 노랑 (0x194): 북마크 삭제
    - 초록 (0x192): 새 폴더 생성
  - 옵션 버튼 (Qt::Key_Menu): 컨텍스트 메뉴 열기 (QMenu::exec())
  - 포커스 가시성 향상:
    - Qt Stylesheet로 포커스된 항목 배경색/테두리 강조
    - QListWidget::item:focus { border: 3px solid #0078d7; }

---

## 3. 비기능 요구사항 (Non-Functional Requirements)

### 성능
- **북마크 로딩 시간**: 앱 시작 시 LS2 API에서 북마크 데이터 로드 1초 이내
- **북마크 추가/편집/삭제 반응 속도**: 사용자 동작 후 0.5초 이내에 UI 업데이트
- **목록 스크롤 성능**: 북마크 100개 이상일 때도 스크롤 버벅임 없이 60fps 유지 (QListWidget::setUniformItemSizes(true) 최적화)
- **메모리 사용량**: 북마크 데이터 및 BookmarkPanel UI는 최대 50MB 이하

### 가독성 (대화면 최적화)
- **폰트 크기**:
  - 북마크 제목: 최소 20px (QFont::setPointSize(20))
  - URL 서브 텍스트: 최소 16px
  - 폴더 이름: 최소 22px
- **대비**: 배경과 전경 색상 대비 비율 4.5:1 이상 (WCAG AA 기준)
- **포커스 표시**: 포커스된 북마크 항목은 3px 두께 테두리 또는 명확한 배경색 변화
- **아이콘 크기**: 파비콘, 폴더 아이콘 최소 32x32px (QIcon::fromTheme() 또는 QPixmap)

### UX (리모컨 최적화)
- **포커스 경로**: 리모컨 방향키로 모든 북마크, 폴더, 버튼에 접근 가능 (Qt Tab Order)
- **직관적인 폴더 탐색**: QTreeWidget으로 폴더 구조 시각적 표시 (들여쓰기, 확장/축소 아이콘)
- **피드백 제공**: 북마크 추가/편집/삭제 시 상태바 메시지 또는 Qt Notification
- **에러 복구**: 삭제 후 "실행 취소" 옵션 제공 (선택적 기능, QUndoStack)

### 데이터 안정성
- **LS2 API 에러 처리**:
  - 네트워크 오류, 타임아웃, 잘못된 응답 처리
  - StorageService::error(QString) 시그널 → BookmarkService::onError(QString) 슬롯
  - QMessageBox로 에러 메시지 표시
- **데이터 무결성**:
  - 북마크 추가 시 중복 URL 체크 (BookmarkService::exists(QString url))
  - 폴더 삭제 시 하위 북마크 cascade 삭제 또는 루트로 이동
  - LS2 API 트랜잭션 실패 시 롤백 (가능한 경우)

### 확장성
- **북마크 동기화**: 향후 LG 계정 연동으로 북마크 클라우드 동기화 가능하도록 서비스 계층 모듈화
- **북마크 가져오기/내보내기**: HTML 북마크 파일 가져오기/내보내기 기능 추가 가능 (QFile, QDomDocument)
- **폴더 깊이 확장**: 현재 1단계 서브폴더만 지원하나, 향후 다단계 폴더 구조 지원 가능 (재귀적 QTreeWidgetItem)
- **최대 북마크 개수**: 1000개 이상 북마크 지원 (LS2 API 페이지네이션 또는 QListWidget 가상화)

### 보안
- **URL 검증**: 북마크 추가/편집 시 URLValidator::isValid(QString url) 호출
- **XSS 방지**: 북마크 제목, URL에 HTML 태그 이스케이프 처리 (QString::toHtmlEscaped())
- **LS2 API 권한**: appinfo.json에 com.webos.service.db 권한 명시
  ```json
  "requiredPermissions": ["db8.find", "db8.put", "db8.del"]
  ```

---

## 4. 제약사항

### webOS 플랫폼 제약사항
- **리모컨 전용 입력**: 마우스, 터치, 물리 키보드 사용 불가 (Qt::Key_* 이벤트로만 입력 처리)
- **Qt 포커스 시스템**: QWidget::setFocusPolicy(Qt::StrongFocus), Tab Order 설정 필수
- **5-way 방향키**: 상/하/좌/우/선택 버튼만으로 모든 입력 처리
- **화면 해상도**: 4K UHD (3840x2160) 기본, Full HD (1920x1080) 호환 (QApplication::desktop()->screenGeometry())

### 기술 제약사항
- **프레임워크**: Qt 5.15+ (C++17 표준)
- **UI 컴포넌트**: Qt Widgets (QListWidget, QTreeWidget, QDialog, QPushButton, QLineEdit 등)
- **스타일링**: Qt Stylesheet (.qss 파일) 또는 QStyle
- **상태 관리**: Qt Signal/Slot 시스템
- **데이터 저장**: webOS LS2 API (com.webos.service.db)
  - JSON 형식 데이터 저장 (QJsonObject, QJsonArray)
  - 비동기 호출 (Qt 이벤트 루프)
- **북마크 용량**: LS2 API DB8 용량 제한 (앱당 최대 100MB)

### 메모리 제약
- **프로젝터 RAM**: LG HU715QW는 제한적인 RAM (1~2GB 추정)
- **BookmarkPanel 메모리**: 북마크 1000개일 때 최대 50MB 이하
- **QListWidget 최적화**:
  - QListWidget::setUniformItemSizes(true) 사용
  - 가상화 스크롤링 고려 (QAbstractItemView::ScrollPerPixel)

### 의존성
- **F-01 (프로젝트 초기 설정)**: Qt/C++ 프로젝트 구조, CMakeLists.txt, 네임스페이스 설정 완료 필요
- **F-02 (웹뷰 통합)**: 북마크 실행 시 WebView::load(QUrl) 메서드 호출 필요
- **F-03 (URL 입력 UI)**: 북마크 URL 편집 시 URLBar의 가상 키보드 재사용 가능 (선택적)
- **StorageService**: LS2 API 래퍼 클래스 구현 완료 필요 (비동기 호출, JSON 파싱)

### 범위 외 (Out of Scope)
- **북마크 동기화**: 외부 브라우저 또는 클라우드와 북마크 동기화 미지원 (향후 확장 가능)
- **북마크 가져오기/내보내기**: HTML 북마크 파일 가져오기/내보내기 미지원
- **파비콘 자동 다운로드**: 북마크 추가 시 파비콘 자동 다운로드 미지원 (선택적 기능)
- **북마크 태그**: 북마크에 태그 추가 기능 미지원
- **북마크 공유**: 다른 사용자와 북마크 공유 기능 미지원
- **다단계 폴더**: 현재 1단계 서브폴더만 지원 (2단계 이상 미지원)

---

## 5. 용어 정의

| 용어 | 정의 |
|------|------|
| **북마크** | 사용자가 자주 방문하는 웹 페이지의 URL과 제목을 저장한 항목 (C++ Bookmark 구조체) |
| **BookmarkPanel** | 북마크 목록을 표시하고 관리하는 Qt QWidget 기반 패널 컴포넌트 |
| **BookmarkService** | 북마크 데이터의 CRUD 작업을 담당하는 QObject 기반 서비스 계층 클래스 |
| **폴더** | 북마크를 카테고리별로 분류하기 위한 디렉토리 구조 (C++ Folder 구조체) |
| **루트 폴더** | 최상위 폴더로, folderId가 빈 문자열인 북마크가 저장되는 기본 위치 |
| **서브폴더** | 루트 폴더 또는 다른 폴더 내에 생성된 하위 폴더 (1단계 깊이만 지원) |
| **LS2 API** | webOS의 Luna Service 2 메시지 버스 API (com.webos.service.db) |
| **StorageService** | LS2 API를 래핑하여 비동기 데이터 저장/조회를 제공하는 QObject 클래스 |
| **DB8** | webOS의 JSON 기반 NoSQL 데이터베이스 (LS2 API로 접근) |
| **파비콘** | 웹사이트의 작은 아이콘 이미지 (QIcon, 16x16px 또는 32x32px) |
| **컨텍스트 메뉴** | 북마크 항목에서 추가 작업(편집, 삭제 등)을 선택할 수 있는 Qt QMenu 팝업 |
| **시그널/슬롯** | Qt의 이벤트 처리 메커니즘 (emit 시그널 → connect된 슬롯 함수 호출) |

---

## 6. UI/UX 플로우

### 플로우 1: 북마크 추가
1. 사용자가 웹 페이지를 탐색 중
2. NavigationBar의 "북마크 추가" QPushButton 클릭 (또는 리모컨 빨강 버튼)
3. 북마크 추가 QDialog 표시:
   - 제목 QLineEdit에 현재 페이지 타이틀 자동 입력
   - URL QLineEdit에 현재 URL 표시 (읽기 전용)
   - 폴더 선택 QComboBox (기본: "루트 폴더")
4. 사용자가 제목 편집 또는 폴더 선택
5. "확인" QPushButton 클릭
6. BookmarkService::addBookmark() 호출 → LS2 API로 데이터 저장
7. 저장 성공 시 상태바 메시지 표시 ("북마크가 저장되었습니다")
8. QDialog 닫기

### 플로우 2: 북마크 목록 조회 및 실행
1. 사용자가 NavigationBar의 "북마크" QPushButton 클릭 (또는 리모컨 단축키)
2. BookmarkPanel (QWidget) 열기 (슬라이드 인 애니메이션)
3. BookmarkService::getAllBookmarks() 호출 → LS2 API에서 데이터 로드
4. 북마크 목록 QListWidget 또는 QTreeWidget에 표시:
   - 각 항목: 파비콘 (QIcon), 제목 (QString), URL (서브 텍스트)
   - 폴더 구조 트리 뷰
5. 사용자가 리모컨 방향키로 북마크 탐색 (포커스 이동)
6. 북마크 선택 (Enter 키 또는 더블클릭)
7. emit BookmarkPanel::bookmarkSelected(QString url) 시그널
8. BrowserWindow::onBookmarkSelected(QString url) 슬롯 → WebView::load(QUrl)
9. BookmarkPanel 자동 닫기 (슬라이드 아웃 애니메이션)

### 플로우 3: 북마크 편집
1. BookmarkPanel에서 북마크 항목 우클릭 (또는 리모컨 옵션 버튼)
2. 컨텍스트 QMenu 표시: "편집", "삭제", "새 탭에서 열기"
3. "편집" 메뉴 항목 선택
4. 북마크 편집 QDialog 표시 (기존 제목, URL, 폴더 자동 입력)
5. 사용자가 제목 또는 폴더 변경
6. "확인" QPushButton 클릭
7. BookmarkService::updateBookmark() 호출 → LS2 API로 업데이트
8. 저장 성공 시 QListWidget 항목 즉시 업데이트, 상태바 메시지 표시
9. QDialog 닫기

### 플로우 4: 북마크 삭제
1. BookmarkPanel에서 북마크 항목 선택 후 Delete 키 (또는 컨텍스트 메뉴 → "삭제")
2. 삭제 확인 QMessageBox 표시 ("북마크를 삭제하시겠습니까?")
3. "확인" 버튼 클릭
4. BookmarkService::deleteBookmark() 호출 → LS2 API로 삭제
5. 삭제 성공 시 QListWidget에서 항목 제거, 상태바 메시지 표시
6. 포커스를 다음 항목으로 이동
7. QMessageBox 닫기

### 플로우 5: 폴더 생성 및 탐색
1. BookmarkPanel에서 "새 폴더" QPushButton 클릭 (또는 리모컨 초록 버튼)
2. QInputDialog 표시: "폴더 이름을 입력하세요"
3. 사용자가 폴더 이름 입력 후 "확인"
4. BookmarkService::createFolder() 호출 → LS2 API로 저장
5. QTreeWidget에 새 폴더 항목 추가 (QTreeWidgetItem)
6. 사용자가 폴더 항목 선택 (Enter 키 또는 오른쪽 방향키)
7. QTreeWidget::expandItem() → 하위 북마크 표시
8. 사용자가 왼쪽 방향키 또는 백 버튼으로 상위 폴더로 이동

---

## 7. 데이터 모델

### Bookmark 구조체 (C++)
```cpp
namespace webosbrowser {

struct Bookmark {
    QString id;           // UUID (QUuid::createUuid().toString())
    QString title;        // 북마크 제목 (예: "YouTube")
    QString url;          // 북마크 URL (예: "https://www.youtube.com")
    QString folderId;     // 폴더 ID (루트는 빈 문자열 "")
    QString favicon;      // 파비콘 URL (선택, 기본 빈 문자열)
    qint64 createdAt;     // 생성 시각 (Unix timestamp, QDateTime::currentMSecsSinceEpoch())
    qint64 updatedAt;     // 수정 시각 (Unix timestamp)
    int visitCount;       // 방문 횟수 (선택, 기본 0)

    // JSON 직렬화
    QJsonObject toJson() const;
    static Bookmark fromJson(const QJsonObject &json);
};

} // namespace webosbrowser
```

### Folder 구조체 (C++)
```cpp
namespace webosbrowser {

struct Folder {
    QString id;           // UUID (QUuid::createUuid().toString())
    QString name;         // 폴더 이름 (예: "엔터테인먼트")
    QString parentId;     // 부모 폴더 ID (루트는 빈 문자열 "")
    qint64 createdAt;     // 생성 시각 (Unix timestamp)

    // JSON 직렬화
    QJsonObject toJson() const;
    static Folder fromJson(const QJsonObject &json);
};

} // namespace webosbrowser
```

### LS2 API 저장 형식 (JSON)
```json
// 북마크 데이터 (com.webos.service.db/put)
{
  "_kind": "com.jsong.webosbrowser.native:1",
  "id": "550e8400-e29b-41d4-a716-446655440000",
  "title": "YouTube",
  "url": "https://www.youtube.com",
  "folderId": "",
  "favicon": "https://www.youtube.com/favicon.ico",
  "createdAt": 1620000000000,
  "updatedAt": 1620000000000,
  "visitCount": 5
}

// 폴더 데이터 (com.webos.service.db/put)
{
  "_kind": "com.jsong.webosbrowser.native.folder:1",
  "id": "660e8400-e29b-41d4-a716-446655440001",
  "name": "엔터테인먼트",
  "parentId": "",
  "createdAt": 1620000000000
}
```

### LS2 API 호출 예시
```cpp
// 북마크 추가 (StorageService 래퍼)
QJsonObject bookmark;
bookmark["_kind"] = "com.jsong.webosbrowser.native:1";
bookmark["id"] = QUuid::createUuid().toString();
bookmark["title"] = "YouTube";
bookmark["url"] = "https://www.youtube.com";
bookmark["folderId"] = "";
bookmark["createdAt"] = QDateTime::currentMSecsSinceEpoch();

storageService->put(bookmark);

// 북마크 조회
QJsonObject query;
query["_kind"] = "com.jsong.webosbrowser.native:1";
query["limit"] = 1000;

storageService->find(query);
```

---

## 8. 에러 케이스

### LS2 API 에러 처리
| 에러 상황 | 원인 | 대응 방안 |
|-----------|------|-----------|
| 네트워크 타임아웃 | LS2 서비스 응답 없음 | QMessageBox 에러 표시 ("북마크 저장 실패: 네트워크 오류"), 재시도 버튼 제공 |
| DB8 용량 초과 | 앱당 100MB 제한 초과 | QMessageBox 경고 ("저장 공간 부족: 오래된 북마크를 삭제해주세요") |
| 잘못된 JSON 형식 | LS2 API 응답 파싱 실패 | Logger::error() 호출, 기본 빈 배열 반환 |
| 권한 거부 | appinfo.json에 권한 없음 | QMessageBox 에러 ("앱 권한 오류: 앱을 재설치해주세요") |

### 중복 북마크 처리
- **시나리오**: 사용자가 이미 북마크된 URL을 다시 추가 시도
- **대응**:
  1. BookmarkService::exists(QString url) 호출 (LS2 API find query)
  2. 중복 발견 시 QMessageBox 경고 표시 ("이미 북마크에 추가된 페이지입니다")
  3. 다이얼로그 닫기, 북마크 추가하지 않음

### 잘못된 URL 처리
- **시나리오**: 사용자가 북마크 편집 시 잘못된 URL 입력
- **대응**:
  1. QLineEdit::textChanged 시그널 → URLValidator::isValid() 호출
  2. 유효하지 않은 URL이면 QLineEdit 배경색 빨강으로 변경 (Qt Stylesheet)
  3. "확인" 버튼 비활성화 (QPushButton::setEnabled(false))
  4. QLabel로 에러 메시지 표시 ("올바른 URL 형식이 아닙니다")

### 폴더 삭제 시 하위 북마크 처리
- **시나리오**: 사용자가 북마크가 있는 폴더 삭제 시도
- **대응**:
  1. BookmarkService::getBookmarksInFolder(QString folderId) 호출
  2. 하위 북마크가 있을 경우 QMessageBox 경고:
     - "폴더 내 북마크 X개도 함께 삭제됩니다. 계속하시겠습니까?"
     - 옵션: "북마크를 루트 폴더로 이동" 체크박스
  3. "확인" 시 BookmarkService::deleteFolder(QString folderId, bool moveToRoot)
  4. moveToRoot == true이면 하위 북마크의 folderId를 빈 문자열로 업데이트

### 앱 초기화 시 데이터 로드 실패
- **시나리오**: 앱 시작 시 LS2 API에서 북마크 로드 실패
- **대응**:
  1. BookmarkService::init() → LS2 API error 시그널 수신
  2. Logger::error("북마크 로드 실패") 호출
  3. BookmarkPanel 열 때 빈 목록 표시 ("저장된 북마크가 없습니다")
  4. 재시도 버튼 제공 (BookmarkService::init() 재호출)

---

## 9. 완료 기준 (Definition of Done)

### DoD-1: 기능 완성도
- [ ] 북마크 추가/편집/삭제 기능 정상 동작 (LS2 API 저장/조회/삭제)
- [ ] 폴더 생성/이름 변경/삭제 기능 정상 동작 (1단계 서브폴더 지원)
- [ ] 북마크 목록 조회 및 폴더 구조 트리 뷰 표시
- [ ] 북마크 선택 시 WebView에서 페이지 로드
- [ ] 리모컨 방향키 및 단축키로 모든 기능 사용 가능
- [ ] 앱 재시작 후 저장된 북마크 데이터 정상 로드

### DoD-2: 테스트 통과
- [ ] 단위 테스트: BookmarkService 클래스 (Google Test)
  - addBookmark(), updateBookmark(), deleteBookmark() 테스트
  - getAllBookmarks(), getBookmarksInFolder() 테스트
  - 중복 URL 체크 테스트
- [ ] 통합 테스트: BookmarkPanel UI (Qt Test)
  - 북마크 추가 다이얼로그 표시 및 입력 테스트
  - 북마크 목록 QListWidget/QTreeWidget 표시 테스트
  - 북마크 실행 시 bookmarkSelected 시그널 emit 테스트
- [ ] 수동 테스트: 실제 webOS 프로젝터 환경에서 리모컨 테스트
  - 북마크 추가 → 목록 조회 → 실행 → 페이지 로드 전체 플로우
  - 폴더 생성 → 북마크 이동 → 폴더 삭제 플로우
  - 리모컨 방향키/컬러 버튼 매핑 테스트

### DoD-3: 코드 품질
- [ ] C++ 코딩 컨벤션 준수 (CLAUDE.md 기준)
  - 주석 언어: 한국어
  - 클래스명 PascalCase, 변수명 camelCase
  - 스마트 포인터 사용 (std::unique_ptr, std::shared_ptr)
  - Qt Signal/Slot 신규 문법 사용
- [ ] 헤더 파일: `#pragma once` 사용
- [ ] 메모리 누수 없음 (Qt 부모-자식 메모리 관리)
- [ ] 에러 처리: LS2 API 에러, 중복 URL, 잘못된 입력 모두 처리
- [ ] 로깅: Logger::info(), Logger::error() 적절히 사용

### DoD-4: 성능 기준
- [ ] 북마크 로딩 시간 1초 이내 (100개 북마크 기준)
- [ ] 북마크 추가/편집/삭제 반응 속도 0.5초 이내
- [ ] 목록 스크롤 60fps 유지 (100개 이상 북마크)
- [ ] 메모리 사용량 50MB 이하 (BookmarkPanel + BookmarkService)

### DoD-5: 문서화
- [ ] 이 요구사항 분석서 (requirements.md) 완성
- [ ] 기술 설계서 (design.md) 작성 완료 (architect)
- [ ] 구현 계획서 (plan.md) 작성 완료 (product-manager)
- [ ] API 문서: BookmarkService 클래스 공개 메서드 Doxygen 주석
- [ ] README 업데이트: F-07 기능 완료 명시

### DoD-6: 코드 리뷰
- [ ] code-reviewer 에이전트 리뷰 통과
  - 코드 컨벤션 준수 확인
  - 메모리 관리 적절성 확인
  - 에러 처리 완전성 확인
- [ ] 리뷰 피드백 반영 완료

---

## 10. 참고 사항

### 관련 문서
- **PRD**: `docs/project/prd.md`
- **기능 백로그**: `docs/project/features.md`
- **프로젝트 가이드**: `CLAUDE.md`
- **URL 입력 UI 요구사항**: `docs/specs/url-input-ui/requirements.md` (가상 키보드 재사용 가능)
- **웹뷰 통합 요구사항**: `docs/specs/webview-integration/requirements.md` (WebView::load() 연동)
- **탭 관리 요구사항**: `docs/specs/tab-management/requirements.md` (TabManager 연동)

### 참고 기술 문서
- **Qt QListWidget**: https://doc.qt.io/qt-5/qlistwidget.html
- **Qt QTreeWidget**: https://doc.qt.io/qt-5/qtreewidget.html
- **Qt Signal/Slot**: https://doc.qt.io/qt-5/signalsandslots.html
- **Qt Stylesheet**: https://doc.qt.io/qt-5/stylesheet.html
- **webOS LS2 API (DB8)**: https://webostv.developer.lge.com/develop/native-apps/native-service-overview
- **Qt JSON**: https://doc.qt.io/qt-5/json.html

### 우선순위 조정 가능성
- **FR-6 (폴더 관리)**: Should 우선순위로, 기본 북마크 CRUD 완료 후 추가 가능
- **FR-7 (정렬 및 검색)**: Could 우선순위로, M3 이후 확장 기능으로 연기 가능
- **파비콘 자동 다운로드**: 선택적 기능으로, 기본 기능 완료 후 추가 가능
- **실행 취소 (QUndoStack)**: 선택적 기능으로, M3 이후 확장 가능

### 기술적 고려사항
- **LS2 API vs SQLite**: webOS 공식 권장은 LS2 API (DB8)이나, SQLite 직접 사용도 가능. 프로젝트는 LS2 API 사용.
- **비동기 처리**: LS2 API는 비동기 호출이므로 Qt Signal/Slot으로 응답 처리 필수
- **폴더 구조 깊이**: 현재 1단계 서브폴더만 지원, 향후 재귀적 QTreeWidget으로 다단계 확장 가능
- **북마크 용량**: 1000개 이상 북마크 시 QListWidget 가상화 (QAbstractItemView::ScrollPerPixel) 고려
- **북마크 중복 방지**: URL 기준 중복 체크 (대소문자 구분 없음, QString::toLower())

### 병렬 작업 가능성
- **PG-2 병렬 그룹**: F-07 (북마크 관리), F-08 (히스토리 관리), F-09 (검색 엔진 통합)은 독립적으로 개발 가능
- **주의 사항**:
  - F-09 (검색 엔진 통합)는 URLBar 수정 필요 → F-03 완료 후 시작
  - BookmarkService와 BookmarkPanel은 동일 기능 내에서 순차 개발 권장 (BookmarkService 먼저 구현)

### 에이전트 라우팅
1. **요구사항 분석 (현재 단계)**: product-manager → requirements.md 작성 완료
2. **기술 설계**: architect → design.md 작성 (클래스 다이어그램, LS2 API 시퀀스 다이어그램)
3. **구현 계획**: product-manager → plan.md 작성 (Phase별 Task, 의존성, 병렬 실행 판단)
4. **C++ 구현**: cpp-dev (필요 시 생성) → BookmarkService, BookmarkPanel 코드 작성
5. **테스트**: test-runner → 단위 테스트 (Google Test), 통합 테스트 (Qt Test)
6. **코드 리뷰**: code-reviewer → 코드 + 문서 검증
7. **문서화**: doc-writer → CHANGELOG.md, dev-log.md 업데이트

---

## 11. 변경 이력

| 날짜 | 변경 내용 | 작성자 |
|------|-----------|--------|
| 2026-02-14 | 초기 작성 (Web App PoC 기반 요구사항을 Native App으로 전환) | product-manager |
