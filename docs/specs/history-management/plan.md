# 히스토리 관리 — 구현 계획서

## 1. 참조 문서
- **요구사항 분석서**: docs/specs/history-management/requirements.md
- **기술 설계서**: docs/specs/history-management/design.md
- **PRD**: docs/project/prd.md
- **CLAUDE.md**: /Users/jsong/dev/jsong1230-github/webosbrowser-native/CLAUDE.md

---

## 2. 개요

### 기능 요약
사용자의 웹 페이지 방문 기록을 자동으로 저장하고 날짜별로 그룹화하여 조회, 검색, 삭제할 수 있는 히스토리 관리 기능을 C++/Qt Native App으로 구현합니다. webOS LS2 API를 통해 영속 데이터를 저장하며, 리모컨 최적화 Qt UI(QListWidget)로 대화면 환경에서 최적의 UX를 제공합니다.

### 구현 목표
1. WebView의 페이지 로드 완료 시 히스토리 자동 기록 (중복 방지 로직 포함)
2. 날짜별 그룹화된 히스토리 목록 조회 (오늘, 어제, YYYY-MM-DD)
3. 제목/URL 기반 실시간 검색
4. 단일/날짜별/전체 삭제 기능
5. 리모컨 방향키 및 단축키로 모든 기능 접근 가능
6. Lazy Loading으로 대용량 히스토리(5,000개+) 성능 최적화
7. LS2 API 비동기 호출로 UI 블로킹 없음

### 주요 차별점 (북마크 관리 F-07과 비교)
| 측면 | 히스토리 관리 (F-08) | 북마크 관리 (F-07) |
|------|---------------------|-------------------|
| **기록 방식** | 자동 (페이지 로드 완료 시) | 수동 (사용자 액션) |
| **데이터 구조** | 단순 리스트 (폴더 없음) | 폴더 계층 구조 |
| **정렬 기준** | 날짜별 그룹화 (최신순) | 폴더별 그룹화 |
| **중복 처리** | 5분 내 재방문 시 visitCount 증가 | 중복 URL 추가 불가 |
| **용량 제한** | 최대 5,000개 (자동 정리) | 제한 없음 |
| **Lazy Loading** | 필수 (초기 500개만 로드) | 필요 없음 (전체 로드) |

---

## 3. Phase 구성 및 태스크 분해

### Phase 1: 데이터 모델 및 서비스 계층 (Backend 기반)

**목표**: LS2 API 연동 및 히스토리 CRUD 서비스 구현

**태스크**:
- [ ] **Task 1.1**: HistoryEntry 구조체 정의
  - **파일**: `src/services/HistoryService.h`
  - **내용**:
    - `HistoryEntry` 구조체 선언 (id, url, title, domain, favicon, visitedAt, lastVisitedAt, visitCount)
    - JSON 직렬화/역직렬화 메서드 (`toJson()`, `fromJson()`)
    - 날짜 그룹핑 헬퍼 (`getDateGroup()`, `getVisitTimeString()`)
  - **담당**: cpp-dev
  - **완료 기준**: HistoryEntry 구조체 컴파일 성공, toJson/fromJson 단위 테스트 통과

- [ ] **Task 1.2**: HistoryService 클래스 구현 (CRUD)
  - **파일**: `src/services/HistoryService.h`, `src/services/HistoryService.cpp`
  - **내용**:
    - `addHistory(QString url, QString title, QString favicon)`: 히스토리 추가 (중복 체크 포함)
    - `getAllHistory(int limit)`: 전체 히스토리 조회 (lastVisitedAt 역순, 기본 limit=500)
    - `getHistoryById(QString id)`: 단일 히스토리 조회
    - `deleteHistory(QString id)`: 개별 히스토리 삭제
    - `clearAllHistory()`: 전체 히스토리 삭제
    - Qt Signal/Slot 정의:
      - `historyAdded(HistoryEntry)`: 히스토리 추가 완료 시그널
      - `dataLoaded(QVector<HistoryEntry>)`: 히스토리 로드 완료
      - `historyDeleted(QString id)`: 히스토리 삭제 완료
      - `error(QString message)`: 에러 발생
  - **담당**: cpp-dev
  - **의존성**: StorageService (LS2 API 래퍼) 완료 필요
  - **완료 기준**:
    - HistoryService 컴파일 성공
    - 단위 테스트: addHistory, getAllHistory, deleteHistory 통과
    - LS2 API 호출 시그널 emit 확인

- [ ] **Task 1.3**: LS2 API Kind 등록 (com.jsong.webosbrowser.native.history:1)
  - **파일**: `webos-meta/appinfo.json` (권한 추가), LS2 Kind 스키마 (별도 JSON)
  - **내용**:
    - appinfo.json에 `requiredPermissions` 추가: `["db8.find", "db8.put", "db8.del"]`
    - LS2 Kind 스키마 정의 (indexes: url, visitedAt, domain, urlVisitedAt 복합 인덱스)
    - StorageService에서 Kind 초기화 (`com.webos.service.db/putKind` 호출)
  - **담당**: cpp-dev
  - **완료 기준**: webOS 프로젝터에서 LS2 Kind 등록 확인 (`luna-send -n 1 luna://com.webos.service.db/getKind ...`)

**산출물**:
- `src/services/HistoryService.h`
- `src/services/HistoryService.cpp`
- `webos-meta/appinfo.json` (권한 추가)
- LS2 Kind 스키마 JSON

**완료 기준**:
- HistoryService 단위 테스트 100% 통과 (Google Test)
- LS2 API 비동기 호출 성공 (StorageService::dataLoaded 시그널 수신 확인)
- appinfo.json에 권한 명시

---

### Phase 2: 자동 히스토리 기록 (WebView 연동)

**목표**: WebView의 페이지 로드 완료 시 히스토리 자동 기록 구현

**태스크**:
- [ ] **Task 2.1**: BrowserWindow에서 WebView::loadFinished 시그널 연결
  - **파일**: `src/browser/BrowserWindow.cpp`, `src/browser/BrowserWindow.h`
  - **내용**:
    - BrowserWindow 생성자에서 WebView::loadFinished(bool ok) 시그널 → BrowserWindow::onLoadFinished(bool ok) 슬롯 연결
    - onLoadFinished() 구현:
      - ok == true이고 HTTP/HTTPS 프로토콜인지 체크
      - URLValidator::isValid(url) 호출
      - about:blank 등 예외 URL 필터링
      - WebView::url(), WebView::title() 호출하여 현재 페이지 정보 추출
      - HistoryService::addHistory(url, title) 호출
  - **담당**: cpp-dev
  - **의존성**: F-02 (WebView 통합) 완료, HistoryService Phase 1 완료
  - **완료 기준**: 페이지 로드 완료 시 HistoryService::historyAdded 시그널 emit 확인

- [ ] **Task 2.2**: 중복 방지 로직 구현 (5분 이내 재방문 체크)
  - **파일**: `src/services/HistoryService.cpp`
  - **내용**:
    - `getRecentVisitByUrl(QString url, qint64 timeWindow)` 프라이빗 메서드 추가
    - LS2 API 쿼리: url 일치 && lastVisitedAt >= (현재 시각 - 5분) 조건
    - 중복 발견 시: 기존 HistoryEntry의 lastVisitedAt 업데이트, visitCount 증가 (LS2 `merge` 호출)
    - 중복 없음: 새 HistoryEntry 생성 (visitCount=1, QUuid::createUuid())
  - **담당**: cpp-dev
  - **완료 기준**:
    - 5분 내 동일 URL 재방문 시 중복 기록 안 함 (visitCount 증가 확인)
    - 5분 초과 재방문 시 새 HistoryEntry 생성 확인

- [ ] **Task 2.3**: 용량 제한 (5,000개) 및 자동 정리
  - **파일**: `src/services/HistoryService.cpp`
  - **내용**:
    - `pruneOldHistory()` 프라이빗 메서드 추가
    - getAllHistory() 호출하여 전체 히스토리 개수 확인
    - 5,000개 초과 시 visitedAt 오름차순 정렬 후 가장 오래된 항목부터 삭제
    - addHistory() 호출 시 pruneOldHistory() 자동 실행
  - **담당**: cpp-dev
  - **완료 기준**: 히스토리 5,000개 초과 시 자동 삭제 로직 동작 확인 (로그 출력)

**산출물**:
- `src/browser/BrowserWindow.cpp` (onLoadFinished 추가)
- `src/services/HistoryService.cpp` (중복 체크, 용량 제한 로직)

**완료 기준**:
- 페이지 방문 시 히스토리 자동 기록 (LS2 API에 저장)
- 5분 내 동일 URL 재방문 시 중복 방지
- 히스토리 5,000개 초과 시 자동 정리

---

### Phase 3: 중복 방문 체크 최적화 (캐시 + DB Hybrid)

**목표**: 중복 체크 성능 최적화 (메모리 캐시 + LS2 API Hybrid)

**태스크**:
- [ ] **Task 3.1**: 메모리 캐시 구현 (최근 100개 히스토리)
  - **파일**: `src/services/HistoryService.h`, `src/services/HistoryService.cpp`
  - **내용**:
    - `QMap<QString, HistoryEntry> recentHistoryCache` 멤버 변수 추가 (URL 키)
    - addHistory() 호출 시 캐시 먼저 확인 → 히트 시 LS2 API 호출 생략
    - 캐시 미스 시 LS2 API 쿼리 → 결과를 캐시에 추가
    - 캐시 크기 제한 (최대 100개, LRU 방식으로 오래된 항목 삭제)
  - **담당**: cpp-dev
  - **완료 기준**:
    - 중복 체크 속도 0.1초 이내 (캐시 히트 시)
    - 캐시 미스 시에도 LS2 API 쿼리 정상 동작

- [ ] **Task 3.2**: LS2 API 복합 인덱스 활용
  - **파일**: LS2 Kind 스키마 (Task 1.3에서 생성)
  - **내용**:
    - `urlVisitedAt` 복합 인덱스 추가 (url + lastVisitedAt)
    - getRecentVisitByUrl() 쿼리에서 복합 인덱스 활용 (IDBKeyRange.bound 방식)
  - **담당**: cpp-dev
  - **완료 기준**: LS2 API 쿼리 속도 0.3초 이내 (복합 인덱스 사용 시)

**산출물**:
- `src/services/HistoryService.cpp` (메모리 캐시 로직)
- LS2 Kind 스키마 (복합 인덱스 추가)

**완료 기준**:
- 중복 체크 성능 개선 (캐시 히트 시 0.1초 이내)
- LS2 API 쿼리 성능 확인 (복합 인덱스 활용)

---

### Phase 4: HistoryPanel UI 구현 (Lazy Loading)

**목표**: 날짜별 그룹화된 히스토리 목록 UI 구현 (QListWidget + Lazy Loading)

**태스크**:
- [ ] **Task 4.1**: HistoryPanel QWidget 기본 구조
  - **파일**: `src/ui/HistoryPanel.h`, `src/ui/HistoryPanel.cpp`, `src/ui/HistoryPanel.ui` (Qt Designer 사용 가능)
  - **내용**:
    - HistoryPanel 클래스 생성 (QWidget 상속)
    - QListWidget 멤버 변수 추가 (`historyListWidget`)
    - QLineEdit 검색 필드 추가 (`searchLineEdit`)
    - QPushButton 추가: "전체 삭제", "닫기"
    - Qt Signal 정의:
      - `historySelected(QString url)`: 히스토리 항목 선택 시
      - `closed()`: 패널 닫기
  - **담당**: cpp-dev
  - **완료 기준**: HistoryPanel 컴파일 성공, 빈 패널 표시

- [ ] **Task 4.2**: 날짜별 그룹화 로직 (UI용)
  - **파일**: `src/services/HistoryService.cpp`
  - **내용**:
    - `groupHistoryByDate(QVector<HistoryEntry> entries)` 메서드 추가
    - 반환값: `QVector<QPair<QString, QVector<HistoryEntry>>>` (날짜 그룹 라벨, 히스토리 리스트)
    - 날짜 그룹: "오늘", "어제", "YYYY-MM-DD dddd" (QDateTime::toString 사용)
    - QDateTime::currentDateTime()과 비교하여 "오늘", "어제" 판단
  - **담당**: cpp-dev
  - **완료 기준**: 날짜별 그룹화 단위 테스트 통과 (오늘, 어제, 과거 날짜 구분)

- [ ] **Task 4.3**: QListWidget 렌더링 (날짜 헤더 + 히스토리 항목)
  - **파일**: `src/ui/HistoryPanel.cpp`
  - **내용**:
    - HistoryPanel::loadHistory() 메서드 구현:
      - HistoryService::getAllHistory(500) 호출
      - groupHistoryByDate() 호출
      - QListWidget에 항목 추가:
        - 날짜 헤더: QListWidgetItem (배경색 회색, setFlags(Qt::NoItemFlags) → 포커스 불가)
        - 히스토리 항목: QListWidgetItem (아이콘, 제목, URL, 방문 시각 표시)
    - QListWidgetItem 스타일:
      - 파비콘 (QIcon, 기본 이모지 🔖)
      - 제목 (볼드체, 18px)
      - URL (서브 텍스트, 14px, 회색)
      - 방문 시각 (HH:mm, 우측 정렬, 12px)
  - **담당**: cpp-dev
  - **완료 기준**:
    - 히스토리 목록이 날짜별로 그룹화되어 표시
    - 리모컨 방향키로 항목 스크롤 가능 (날짜 헤더는 스킵)

- [ ] **Task 4.4**: Lazy Loading 구현 (스크롤 하단 도달 시 추가 로드)
  - **파일**: `src/ui/HistoryPanel.cpp`
  - **내용**:
    - QListWidget::verticalScrollBar()::valueChanged(int value) 시그널 연결
    - 스크롤 하단 도달 감지: `value >= maximum - 100`
    - HistoryService::getHistoryWithOffset(int offset, int limit) 메서드 추가
    - 추가 500개 히스토리 로드 후 QListWidget에 append
    - 로딩 중 상태 표시 (QLabel "로딩 중...", 하단에 추가)
  - **담당**: cpp-dev
  - **완료 기준**:
    - 스크롤 하단 도달 시 추가 500개 로드
    - 히스토리 5,000개일 때 초기 로딩 2초 이내, 추가 로딩 1초 이내

- [ ] **Task 4.5**: 히스토리 선택 핸들러 (페이지 열기)
  - **파일**: `src/ui/HistoryPanel.cpp`
  - **내용**:
    - QListWidget::itemDoubleClicked(QListWidgetItem* item) 시그널 연결
    - 선택한 히스토리 항목의 URL 추출
    - emit historySelected(url) 시그널
    - BrowserWindow에서 historySelected 시그널 수신 → WebView::load(url) 호출
  - **담당**: cpp-dev
  - **완료 기준**: 히스토리 항목 더블클릭 시 WebView에서 페이지 로드

**산출물**:
- `src/ui/HistoryPanel.h`, `src/ui/HistoryPanel.cpp`
- `src/ui/HistoryPanel.ui` (Qt Designer 파일, 선택)
- `src/services/HistoryService.cpp` (groupHistoryByDate, getHistoryWithOffset 추가)

**완료 기준**:
- HistoryPanel UI 표시 (날짜별 그룹화)
- Lazy Loading 동작 (초기 500개, 스크롤 시 추가 로드)
- 히스토리 선택 시 페이지 열기

---

### Phase 5: 검색 및 삭제 기능

**목표**: 히스토리 검색, 단일/날짜별/전체 삭제 구현

**태스크**:
- [ ] **Task 5.1**: 히스토리 검색 (제목, URL)
  - **파일**: `src/ui/HistoryPanel.cpp`, `src/services/HistoryService.cpp`
  - **내용**:
    - `HistoryService::searchHistory(QString query)` 메서드 추가:
      - getAllHistory() 호출 → 클라이언트 측 필터링 (title, url에 query 포함 여부 체크)
      - QString::contains(query, Qt::CaseInsensitive) 사용
    - QLineEdit::textChanged(QString text) 시그널 연결
    - 검색어 입력 시 searchHistory() 호출 → QListWidget 갱신
    - 검색 결과 개수 표시 (QLabel "X개 결과 찾음")
  - **담당**: cpp-dev
  - **완료 기준**:
    - 검색어 입력 시 실시간 필터링 (0.5초 이내)
    - 검색 결과가 날짜별로 그룹화되어 표시

- [ ] **Task 5.2**: 단일 히스토리 삭제
  - **파일**: `src/ui/HistoryPanel.cpp`, `src/services/HistoryService.cpp`
  - **내용**:
    - 히스토리 항목 컨텍스트 메뉴 추가 (QListWidget::customContextMenuRequested 시그널)
    - QMenu 생성: "삭제" 액션 추가
    - "삭제" 액션 클릭 시 QMessageBox 확인 다이얼로그 표시
    - 확인 버튼 클릭 시 HistoryService::deleteHistory(id) 호출
    - HistoryService::historyDeleted(QString id) 시그널 수신 → QListWidget에서 항목 제거
  - **담당**: cpp-dev
  - **완료 기준**:
    - 히스토리 항목 삭제 시 즉시 UI 반영
    - LS2 API에서 데이터 삭제 확인

- [ ] **Task 5.3**: 날짜별 히스토리 삭제
  - **파일**: `src/ui/HistoryPanel.cpp`, `src/services/HistoryService.cpp`
  - **내용**:
    - `HistoryService::deleteHistoryByDate(QDate date)` 메서드 추가:
      - date의 00:00:00 ~ 23:59:59 범위 계산 (QDateTime)
      - LS2 API 쿼리: visitedAt >= startTimestamp && visitedAt <= endTimestamp
      - LS2 API 배치 삭제 (`com.webos.service.db/del` with where clause)
    - 날짜 그룹 헤더 컨텍스트 메뉴: "이 날짜의 기록 모두 삭제"
    - 확인 다이얼로그 표시 ("YYYY-MM-DD의 기록 X개를 삭제하시겠습니까?")
    - 삭제 완료 시 QListWidget 갱신
  - **담당**: cpp-dev
  - **완료 기준**: 날짜별 삭제 시 해당 날짜의 모든 히스토리 제거 (LS2 API 확인)

- [ ] **Task 5.4**: 전체 히스토리 삭제
  - **파일**: `src/ui/HistoryPanel.cpp`, `src/services/HistoryService.cpp`
  - **내용**:
    - "전체 삭제" QPushButton 클릭 시 QMessageBox 경고 다이얼로그
    - "모든 방문 기록을 삭제하시겠습니까? 이 작업은 되돌릴 수 없습니다"
    - 확인 버튼 클릭 시 HistoryService::clearAllHistory() 호출
    - LS2 API 배치 삭제 (모든 히스토리)
    - 삭제 완료 시 QListWidget 초기화, "방문 기록이 없습니다" QLabel 표시
  - **담당**: cpp-dev
  - **완료 기준**:
    - 전체 삭제 시 모든 히스토리 제거 (LS2 API 확인)
    - UI 초기화 (빈 상태 표시)

**산출물**:
- `src/ui/HistoryPanel.cpp` (검색, 삭제 기능)
- `src/services/HistoryService.cpp` (searchHistory, deleteHistoryByDate, clearAllHistory)

**완료 기준**:
- 검색 기능 동작 (실시간 필터링)
- 단일/날짜별/전체 삭제 기능 동작
- 삭제 후 UI 즉시 반영

---

### Phase 6: 리모컨 키 매핑 및 최적화

**목표**: webOS 리모컨 물리 버튼 매핑 및 성능 최적화

**태스크**:
- [ ] **Task 6.1**: 리모컨 키 이벤트 처리
  - **파일**: `src/ui/HistoryPanel.cpp`
  - **내용**:
    - QWidget::keyPressEvent(QKeyEvent* event) 오버라이드
    - 키 매핑:
      - Qt::Key_Up/Down: QListWidget 스크롤 (setCurrentRow)
      - Qt::Key_Return/Enter: 히스토리 실행 (historySelected 시그널 emit)
      - Qt::Key_Escape/Back: HistoryPanel 닫기 (close())
      - Qt::Key_Delete: 단일 히스토리 삭제
      - Qt::Key_PageUp/PageDown: 빠른 스크롤 (10개 항목씩)
    - webOS 리모컨 컬러 버튼 (0x192~0x195):
      - 빨강 (0x193): 단일 항목 삭제
      - 파랑 (0x195): 날짜별 삭제 다이얼로그
      - 노랑 (0x194): 전체 삭제 다이얼로그
      - 초록 (0x192): 검색 모드 전환 (QLineEdit 포커스)
  - **담당**: cpp-dev
  - **완료 기준**:
    - 리모컨 방향키로 히스토리 탐색
    - 선택 버튼으로 페이지 열기
    - 컬러 버튼으로 삭제/검색 실행

- [ ] **Task 6.2**: 포커스 가시성 향상 (Qt Stylesheet)
  - **파일**: `src/ui/HistoryPanel.cpp` (또는 별도 .qss 파일)
  - **내용**:
    - Qt Stylesheet 정의:
      - `QListWidget::item:focus { border: 3px solid #0078d7; background-color: rgba(0, 120, 215, 0.2); }`
      - 날짜 헤더 배경색: `background-color: rgba(255, 255, 255, 0.03);`
      - 히스토리 항목 호버: `QListWidget::item:hover { background-color: rgba(255, 255, 255, 0.1); }`
    - 대화면 가독성 (폰트 크기):
      - 제목: 18px (QFont::setPointSize(18))
      - URL: 14px
      - 날짜 헤더: 22px (볼드체)
      - 방문 시각: 12px
  - **담당**: cpp-dev
  - **완료 기준**:
    - 포커스된 항목이 명확하게 강조됨 (3px 테두리)
    - 대화면에서 가독성 확보 (폰트 크기 적절)

- [ ] **Task 6.3**: NavigationBar에 히스토리 버튼 추가
  - **파일**: `src/ui/NavigationBar.cpp`, `src/ui/NavigationBar.h`
  - **내용**:
    - "히스토리" QPushButton 추가 (icon: QIcon::fromTheme("view-history"))
    - Qt Signal 정의: `historyButtonClicked()`
    - BrowserWindow에서 historyButtonClicked 시그널 수신 → HistoryPanel 열기
  - **담당**: cpp-dev
  - **완료 기준**:
    - NavigationBar에 히스토리 버튼 표시
    - 버튼 클릭 시 HistoryPanel 열림

- [ ] **Task 6.4**: 성능 최적화 (QListWidget 가상화)
  - **파일**: `src/ui/HistoryPanel.cpp`
  - **내용**:
    - QListWidget::setUniformItemSizes(true) 설정 (렌더링 최적화)
    - QAbstractItemView::ScrollPerPixel 설정 (부드러운 스크롤)
    - 날짜 헤더와 히스토리 항목의 높이 고정 (날짜 헤더: 60px, 히스토리 항목: 80px)
  - **담당**: cpp-dev
  - **완료 기준**:
    - 히스토리 1,000개 이상일 때 스크롤 60fps 유지
    - 메모리 사용량 100MB 이하 (HistoryPanel 포함)

**산출물**:
- `src/ui/HistoryPanel.cpp` (키 이벤트, 스타일시트, 성능 최적화)
- `src/ui/NavigationBar.cpp` (히스토리 버튼 추가)

**완료 기준**:
- 리모컨 키 매핑 완료 (방향키, 선택, 컬러 버튼)
- 포커스 가시성 향상 (Qt Stylesheet)
- 성능 최적화 (60fps 스크롤)

---

### Phase 7: 테스트 및 검증

**목표**: 단위 테스트, 통합 테스트, 성능 테스트, 수동 테스트

**태스크**:
- [ ] **Task 7.1**: 단위 테스트 작성 (Google Test)
  - **파일**: `tests/unit/HistoryServiceTest.cpp`
  - **내용**:
    - `addHistory()` 테스트: 신규 히스토리 추가, JSON 직렬화 확인
    - 중복 체크 테스트: 5분 내 재방문 시 visitCount 증가 확인
    - `getAllHistory()` 테스트: 최신순 정렬 확인
    - `deleteHistory()` 테스트: 단일 히스토리 삭제 확인
    - `clearAllHistory()` 테스트: 전체 히스토리 삭제 확인
    - 날짜 그룹화 테스트: "오늘", "어제", "YYYY-MM-DD" 구분 확인
    - 용량 제한 테스트: 5,000개 초과 시 자동 정리 확인
  - **담당**: test-runner
  - **완료 기준**: 단위 테스트 100% 통과

- [ ] **Task 7.2**: 통합 테스트 작성 (Qt Test)
  - **파일**: `tests/integration/HistoryPanelTest.cpp`
  - **내용**:
    - HistoryPanel UI 렌더링 테스트: QListWidget에 히스토리 항목 표시 확인
    - 날짜 그룹 헤더 생성 테스트: 날짜별 그룹화 UI 확인
    - 검색 필터링 테스트: 검색어 입력 시 결과 필터링 확인
    - 히스토리 선택 테스트: historySelected 시그널 emit 확인
    - 삭제 기능 테스트: 단일/날짜별/전체 삭제 후 UI 업데이트 확인
  - **담당**: test-runner
  - **완료 기준**: 통합 테스트 100% 통과

- [ ] **Task 7.3**: 성능 테스트
  - **파일**: `tests/performance/HistoryPerformanceTest.cpp`
  - **내용**:
    - 히스토리 로딩 시간: 500개 히스토리 로드 2초 이내 확인
    - 히스토리 추가 반응 속도: 페이지 로드 완료 후 0.3초 이내에 저장 확인
    - 검색 응답 속도: 검색어 입력 후 0.5초 이내에 결과 표시 확인
    - 전체 삭제 성능: 5,000개 히스토리 삭제 5초 이내 확인
    - 스크롤 성능: 1,000개 히스토리 스크롤 60fps 유지 확인
    - 메모리 사용량: HistoryPanel + HistoryService 100MB 이하 확인
  - **담당**: test-runner
  - **완료 기준**: 모든 성능 기준 충족

- [ ] **Task 7.4**: 수동 테스트 (webOS 프로젝터)
  - **내용**:
    - **시나리오 1**: 페이지 방문 → 히스토리 자동 기록 → HistoryPanel에서 확인
    - **시나리오 2**: 동일 URL 5분 내 재방문 → visitCount 증가 확인
    - **시나리오 3**: 히스토리 검색 → 결과 확인 → 항목 선택 → 페이지 열기
    - **시나리오 4**: 단일 히스토리 삭제 → UI 업데이트 확인
    - **시나리오 5**: 날짜별 삭제 → 해당 날짜 히스토리 모두 제거 확인
    - **시나리오 6**: 전체 삭제 → 모든 히스토리 제거 확인
    - **시나리오 7**: 리모컨 방향키/컬러 버튼으로 모든 기능 접근 확인
    - **시나리오 8**: Lazy Loading → 스크롤 하단 도달 시 추가 로드 확인
  - **담당**: test-runner
  - **완료 기준**: 모든 시나리오 정상 동작

**산출물**:
- `tests/unit/HistoryServiceTest.cpp`
- `tests/integration/HistoryPanelTest.cpp`
- `tests/performance/HistoryPerformanceTest.cpp`
- 수동 테스트 결과 보고서 (PHASE*_TEST_REPORT.md)

**완료 기준**:
- 단위 테스트 100% 통과
- 통합 테스트 100% 통과
- 성능 테스트 모든 기준 충족
- 수동 테스트 모든 시나리오 통과

---

### Phase 8: 코드 리뷰 및 문서화

**목표**: 코드 품질 검증 및 문서 업데이트

**태스크**:
- [ ] **Task 8.1**: 코드 리뷰 (code-reviewer)
  - **내용**:
    - C++ 코딩 컨벤션 준수 확인 (CLAUDE.md 기준)
    - 메모리 관리 적절성 확인 (스마트 포인터, Qt 부모-자식 메모리 관리)
    - LS2 API 에러 처리 완전성 확인
    - 대용량 데이터 처리 최적화 확인 (Lazy Loading, 메모리 캐시)
    - 리모컨 키 매핑 완전성 확인
  - **담당**: code-reviewer
  - **완료 기준**: 리뷰 피드백 없음 또는 모든 피드백 반영 완료

- [ ] **Task 8.2**: API 문서 작성 (Doxygen 주석)
  - **파일**: `src/services/HistoryService.h`
  - **내용**:
    - HistoryService 클래스 공개 메서드에 Doxygen 주석 추가
    - 메서드 설명, 파라미터, 반환값, 예외 사항 명시
  - **담당**: doc-writer
  - **완료 기준**: Doxygen HTML 문서 생성 확인

- [ ] **Task 8.3**: CHANGELOG.md 업데이트
  - **파일**: `CHANGELOG.md`
  - **내용**:
    - F-08 히스토리 관리 기능 완료 항목 추가
    - 주요 변경 사항, 신규 파일, 수정된 파일 목록 명시
  - **담당**: doc-writer
  - **완료 기준**: CHANGELOG.md 업데이트 완료

- [ ] **Task 8.4**: README.md 업데이트
  - **파일**: `README.md`
  - **내용**:
    - F-08 히스토리 관리 기능 완료 명시
    - 주요 기능 목록에 히스토리 관리 추가
  - **담당**: doc-writer
  - **완료 기준**: README.md 업데이트 완료

**산출물**:
- 코드 리뷰 보고서 (code-reviewer)
- Doxygen API 문서 (HTML)
- `CHANGELOG.md` (업데이트)
- `README.md` (업데이트)

**완료 기준**:
- 코드 리뷰 통과
- API 문서 작성 완료
- CHANGELOG.md, README.md 업데이트 완료

---

## 4. 태스크 의존성

```
Phase 1 (데이터 모델 및 서비스 계층)
  ├── Task 1.1 (HistoryEntry 구조체)
  ├── Task 1.2 (HistoryService CRUD) ─── depends on ──▶ StorageService (F-07 완료)
  └── Task 1.3 (LS2 API Kind 등록)
        │
        ▼
Phase 2 (자동 히스토리 기록)
  ├── Task 2.1 (BrowserWindow 연동) ─── depends on ──▶ WebView::loadFinished (F-02 완료)
  ├── Task 2.2 (중복 방지 로직)
  └── Task 2.3 (용량 제한)
        │
        ▼
Phase 3 (중복 방문 체크 최적화)
  ├── Task 3.1 (메모리 캐시)
  └── Task 3.2 (LS2 API 복합 인덱스)
        │
        ▼
Phase 4 (HistoryPanel UI)
  ├── Task 4.1 (HistoryPanel 기본 구조)
  ├── Task 4.2 (날짜별 그룹화 로직)
  ├── Task 4.3 (QListWidget 렌더링)
  ├── Task 4.4 (Lazy Loading)
  └── Task 4.5 (히스토리 선택 핸들러)
        │
        ▼
Phase 5 (검색 및 삭제 기능)
  ├── Task 5.1 (히스토리 검색)
  ├── Task 5.2 (단일 삭제)
  ├── Task 5.3 (날짜별 삭제)
  └── Task 5.4 (전체 삭제)
        │
        ▼
Phase 6 (리모컨 키 매핑 및 최적화)
  ├── Task 6.1 (리모컨 키 이벤트)
  ├── Task 6.2 (포커스 가시성)
  ├── Task 6.3 (NavigationBar 히스토리 버튼)
  └── Task 6.4 (성능 최적화)
        │
        ▼
Phase 7 (테스트)
  ├── Task 7.1 (단위 테스트) ────────┐
  ├── Task 7.2 (통합 테스트)         │
  ├── Task 7.3 (성능 테스트)         │
  └── Task 7.4 (수동 테스트)         │
        │                           │
        ▼                           │
Phase 8 (코드 리뷰 및 문서화) ◀──────┘
  ├── Task 8.1 (코드 리뷰)
  ├── Task 8.2 (API 문서)
  ├── Task 8.3 (CHANGELOG.md)
  └── Task 8.4 (README.md)
```

---

## 5. 파일 작업 목록

### 신규 생성 파일
1. `src/services/HistoryService.h`
2. `src/services/HistoryService.cpp`
3. `src/ui/HistoryPanel.h`
4. `src/ui/HistoryPanel.cpp`
5. `src/ui/HistoryPanel.ui` (선택, Qt Designer 사용 시)
6. `tests/unit/HistoryServiceTest.cpp`
7. `tests/integration/HistoryPanelTest.cpp`
8. `tests/performance/HistoryPerformanceTest.cpp`
9. LS2 Kind 스키마 JSON (예: `webos-meta/history-kind.json`)

### 수정 필요 파일
1. `src/browser/BrowserWindow.h` (onLoadFinished 슬롯 추가)
2. `src/browser/BrowserWindow.cpp` (WebView::loadFinished 연결, HistoryService 호출)
3. `src/ui/NavigationBar.h` (historyButtonClicked 시그널 추가)
4. `src/ui/NavigationBar.cpp` (히스토리 버튼 추가)
5. `webos-meta/appinfo.json` (LS2 API 권한 추가)
6. `CMakeLists.txt` (HistoryService, HistoryPanel 빌드 추가)
7. `CHANGELOG.md` (F-08 완료 항목 추가)
8. `README.md` (F-08 기능 명시)

---

## 6. 병렬 실행 판단

### 순차 실행 권장
- **이유**:
  - Phase 1~3 (서비스 계층)과 Phase 4~6 (UI 계층)은 순차 의존성 있음
  - HistoryService 완료 후 HistoryPanel 구현 가능
  - 히스토리 자동 기록 (Phase 2)은 HistoryService (Phase 1) 완료 필수
  - UI 테스트 (Phase 7)는 전체 구현 완료 후 가능

### 병렬 작업 가능한 부분
- **Phase 1.1~1.2 (HistoryService)와 Phase 4.1 (HistoryPanel 기본 구조)**:
  - HistoryPanel UI 레이아웃은 HistoryService 완료 전에 작업 가능
  - 단, 데이터 연동은 Phase 1 완료 후 진행
- **Phase 5 (검색)과 Phase 5 (삭제)**:
  - 검색 기능과 삭제 기능은 독립적으로 구현 가능
- **Phase 7.1 (단위 테스트)와 Phase 7.2 (통합 테스트)**:
  - 단위 테스트와 통합 테스트는 병렬 작성 가능

### Agent Team 사용 권장 여부
- **권장하지 않음**: 순차 작업이 더 효율적
- **이유**:
  - F-08은 단일 기능으로 Phase 간 의존성이 명확함
  - cpp-dev 에이전트 1명이 순차 작업하는 것이 컨텍스트 유지에 유리
  - 병렬 작업 시 Phase 1~3 (백엔드)와 Phase 4~6 (프론트엔드) 간 인터페이스 동기화 필요
- **예외**: 테스트 작업 (Phase 7)은 test-runner 에이전트가 병렬 작성 가능

---

## 7. 의존성 체크

### 필수 선행 기능
1. **F-02 (웹뷰 통합)**:
   - **상태**: 완료 필요
   - **의존 항목**: WebView::loadFinished(bool ok) 시그널
   - **영향**: Phase 2 (자동 히스토리 기록) 구현 불가
   - **확인 방법**: WebView 클래스 존재 확인, loadFinished 시그널 확인

2. **StorageService (LS2 API 래퍼)**:
   - **상태**: F-07 (북마크 관리)에서 구현 완료 (공유 가능)
   - **의존 항목**: StorageService::put(), find(), del() 메서드
   - **영향**: Phase 1 (HistoryService) 구현 불가
   - **확인 방법**: StorageService 클래스 존재 확인, LS2 API 호출 테스트

3. **URLValidator**:
   - **상태**: F-03 (URL 입력 UI)에서 구현 (재사용 가능)
   - **의존 항목**: URLValidator::isValid(QString url)
   - **영향**: Phase 2 (자동 히스토리 기록 시 URL 검증)
   - **확인 방법**: URLValidator 클래스 존재 확인

### 선택적 의존성
1. **F-07 (북마크 관리)**:
   - **연동**: 히스토리 항목에서 "북마크 추가" 버튼 제공 (향후 확장)
   - **현재 범위**: F-08 기본 구현에는 불필요

2. **F-11 (설정 화면)**:
   - **연동**: 히스토리 자동 삭제 기간 설정 (향후 확장)
   - **현재 범위**: F-08 기본 구현에는 불필요

### F-07 (북마크 관리)와의 LS2 Kind 공유
- **북마크**: `com.jsong.webosbrowser.native.bookmarks:1`
- **히스토리**: `com.jsong.webosbrowser.native.history:1`
- **공유**: 동일한 webOS DB8 데이터베이스 사용, 서로 다른 Kind (독립적)
- **주의**: F-07 완료 후 F-08 시작 시 DB8 초기화 로직 재사용 가능

---

## 8. 위험 요소 및 대응

### 위험 1: LS2 API 비동기 처리 복잡도
- **위험도**: 높음
- **영향**: HistoryService 구현 복잡도 증가, 에러 처리 누락 가능성
- **대응 방안**:
  - StorageService (F-07)에서 이미 구현된 비동기 래퍼 재사용
  - Qt Signal/Slot으로 비동기 응답 처리 표준화
  - 모든 LS2 API 호출에 타임아웃 및 에러 핸들러 필수 추가

### 위험 2: 중복 방지 로직 성능 저하
- **위험도**: 중간
- **영향**: 페이지 로드마다 LS2 API 쿼리 발생 → 0.3초 이상 지연 가능
- **대응 방안**:
  - Phase 3에서 메모리 캐시 구현 (최근 100개 히스토리)
  - LS2 API 복합 인덱스 활용 (url + visitedAt)
  - 캐시 히트율 70% 이상 목표 → 평균 응답 속도 0.1초 이내

### 위험 3: Lazy Loading 구현 복잡도
- **위험도**: 중간
- **영향**: QListWidget 스크롤 이벤트 처리 어려움, UI 버벅임
- **대응 방안**:
  - QListWidget::verticalScrollBar()::valueChanged 시그널 활용
  - 추가 로드 시 UI 스레드 블로킹 방지 (QThread 또는 Qt Concurrent 사용)
  - 로딩 중 상태 표시 (QLabel "로딩 중...") 추가

### 위험 4: 대용량 히스토리 메모리 부족
- **위험도**: 낮음
- **영향**: 히스토리 5,000개 이상일 때 메모리 초과 (100MB 이상)
- **대응 방안**:
  - 용량 제한 (5,000개) 엄격히 적용 (Phase 2.3)
  - QListWidget 가상화 스크롤링 (Phase 6.4)
  - 초기 로딩 500개로 제한, 필요 시 Lazy Loading

### 위험 5: 리모컨 키 이벤트 충돌
- **위험도**: 낮음
- **영향**: NavigationBar와 HistoryPanel의 키 이벤트 중복 처리
- **대응 방안**:
  - HistoryPanel 열림 시 BrowserWindow의 키 이벤트 비활성화
  - QWidget::setFocusPolicy(Qt::StrongFocus) 설정
  - Escape 키로 HistoryPanel 닫기 → 포커스 복원

---

## 9. 테스트 전략

### 단위 테스트 (Google Test)
- **대상**: HistoryService 클래스
- **커버리지**: 80% 이상
- **주요 테스트 케이스**:
  - addHistory() 정상 동작
  - 중복 체크 (5분 내 재방문)
  - getAllHistory() 최신순 정렬
  - deleteHistory() 정상 삭제
  - clearAllHistory() 전체 삭제
  - 날짜 그룹화 ("오늘", "어제" 구분)
  - 용량 제한 (5,000개 초과 시 자동 정리)

### 통합 테스트 (Qt Test)
- **대상**: HistoryPanel UI 컴포넌트
- **주요 테스트 케이스**:
  - HistoryPanel 렌더링 (날짜별 그룹화)
  - 검색 필터링 (제목, URL)
  - 히스토리 선택 시 historySelected 시그널 emit
  - 삭제 후 UI 업데이트 (단일/날짜별/전체)

### 성능 테스트
- **측정 항목**:
  - 히스토리 로딩 시간: 500개 → 2초 이내
  - 히스토리 추가 반응 속도: 0.3초 이내
  - 검색 응답 속도: 0.5초 이내
  - 전체 삭제 성능: 5,000개 → 5초 이내
  - 스크롤 성능: 60fps 유지
  - 메모리 사용량: 100MB 이하

### 수동 테스트 (webOS 프로젝터)
- **테스트 환경**: LG HU715QW 프로젝터, webOS Developer Mode
- **주요 시나리오**:
  1. 페이지 방문 → 히스토리 자동 기록
  2. 동일 URL 5분 내 재방문 → visitCount 증가
  3. 히스토리 검색 → 결과 확인 → 항목 선택
  4. 단일/날짜별/전체 삭제
  5. 리모컨 방향키/컬러 버튼으로 모든 기능 접근
  6. Lazy Loading (스크롤 하단 도달 시 추가 로드)

---

## 10. 완료 기준 (Definition of Done)

### DoD-1: 기능 완성도
- [ ] 페이지 로드 완료 시 히스토리 자동 기록 (LS2 API 저장)
- [ ] 히스토리 목록 조회 (전체, 날짜별 그룹화)
- [ ] 히스토리 검색 (제목, URL)
- [ ] 히스토리 삭제 (단일, 날짜별, 전체)
- [ ] 히스토리 선택 시 WebView에서 페이지 로드
- [ ] 리모컨 방향키 및 단축키로 모든 기능 사용 가능
- [ ] 앱 재시작 후 저장된 히스토리 데이터 정상 로드
- [ ] 5분 내 동일 URL 재방문 시 중복 방지 (visitCount 증가)
- [ ] 히스토리 5,000개 초과 시 자동 정리

### DoD-2: 테스트 통과
- [ ] 단위 테스트: HistoryService 클래스 (Google Test) 100% 통과
- [ ] 통합 테스트: HistoryPanel UI (Qt Test) 100% 통과
- [ ] 성능 테스트: 모든 성능 기준 충족
  - 히스토리 로딩 2초 이내
  - 히스토리 추가 0.3초 이내
  - 검색 0.5초 이내
  - 전체 삭제 5초 이내
  - 스크롤 60fps 유지
  - 메모리 100MB 이하
- [ ] 수동 테스트: 모든 시나리오 통과 (webOS 프로젝터)

### DoD-3: 코드 품질
- [ ] C++ 코딩 컨벤션 준수 (CLAUDE.md 기준)
  - 주석 언어: 한국어
  - 클래스명 PascalCase, 변수명 camelCase
  - 스마트 포인터 사용 (std::unique_ptr, std::shared_ptr)
  - Qt Signal/Slot 신규 문법 사용
- [ ] 헤더 파일: `#pragma once` 사용
- [ ] 메모리 누수 없음 (Qt 부모-자식 메모리 관리)
- [ ] LS2 API 에러 처리 완전 (타임아웃, 네트워크 오류)
- [ ] 로깅: Logger::info(), Logger::error() 적절히 사용

### DoD-4: 문서화
- [ ] 이 구현 계획서 (plan.md) 완성
- [ ] API 문서: HistoryService 클래스 공개 메서드 Doxygen 주석
- [ ] CHANGELOG.md 업데이트: F-08 기능 완료 명시
- [ ] README.md 업데이트: F-08 기능 완료 명시

### DoD-5: 코드 리뷰
- [ ] code-reviewer 에이전트 리뷰 통과
  - 코드 컨벤션 준수 확인
  - 메모리 관리 적절성 확인
  - LS2 API 에러 처리 완전성 확인
  - 대용량 데이터 처리 최적화 확인 (Lazy Loading, 메모리 캐시)
  - 리모컨 키 매핑 완전성 확인
- [ ] 리뷰 피드백 반영 완료

---

## 11. 참고 사항

### F-07 (북마크 관리)와의 유사점 활용
- **LS2 API 전환**: F-07에서 구현한 StorageService 재사용
- **QListWidget 렌더링**: F-07 BookmarkPanel과 유사한 UI 패턴
- **CRUD 서비스 구조**: BookmarkService와 동일한 아키텍처 (Signal/Slot)
- **리모컨 키 매핑**: BookmarkPanel과 동일한 키 이벤트 처리 패턴

### F-07과의 차이점 (주의 사항)
1. **폴더 구조 없음**: 히스토리는 단순 리스트 (폴더 계층 미지원)
2. **자동 기록**: WebView::loadFinished 시그널 자동 연동 (사용자 액션 없음)
3. **중복 처리**: 5분 내 재방문 시 visitCount 증가 (북마크는 중복 불가)
4. **Lazy Loading**: 히스토리는 대용량 데이터 처리 필수 (북마크는 전체 로드)
5. **날짜 그룹화**: QDateTime 연산으로 "오늘", "어제" 구분 (북마크는 폴더별 그룹)

### 기술적 고려사항
- **Qt QDateTime 사용**: 날짜별 그룹화 시 현재 시각과 비교 (currentDateTime())
- **LS2 API 복합 인덱스**: url + visitedAt 인덱스로 중복 체크 성능 최적화
- **Qt QListWidget 가상화**: setUniformItemSizes(true)로 렌더링 최적화
- **비동기 LS2 API 호출**: Qt Signal/Slot으로 UI 스레드 블로킹 방지

### 확장성 고려
- **F-11 (설정 화면)**: 히스토리 자동 삭제 기간 설정 (향후 연동)
- **F-15 (즐겨찾기 홈 화면)**: visitCount 기반 자주 가는 사이트 표시 (향후 연동)
- **히스토리 동기화**: LG 계정 연동으로 클라우드 동기화 (M3 이후 확장)
- **히스토리 가져오기/내보내기**: JSON 파일 import/export (M3 이후 확장)

---

## 12. 변경 이력

| 날짜 | 변경 내용 | 작성자 |
|------|-----------|--------|
| 2026-02-14 | 최초 작성 (requirements.md, design.md 기반) | product-manager |
| 2026-02-14 | Phase 1~8 태스크 분해, 의존성 분석, 위험 대응 방안 추가 | product-manager |
| 2026-02-14 | Web App(React/Enact)에서 Native App(C++/Qt)으로 전환 | product-manager |
