# 히스토리 관리 — 요구사항 분석서

## 1. 개요

### 기능명
히스토리 관리 (F-08)

### 목적
사용자의 웹 페이지 방문 기록을 자동으로 저장하고 조회/삭제할 수 있는 기능을 제공하여, 이전에 방문했던 사이트를 빠르게 재방문하고 브라우징 히스토리를 관리할 수 있도록 합니다. C++/Qt 기반 Native App으로 구현하여 webOS 프로젝터 환경에서 최적의 성능과 UX를 제공합니다.

### 대상 사용자
- 이전에 방문했던 웹사이트를 다시 찾고자 하는 프로젝터 사용자
- 브라우징 이력을 날짜별로 확인하고 관리하고자 하는 일반 사용자
- 프라이버시 보호를 위해 방문 기록을 삭제하고자 하는 사용자

### 요청 배경
- 웹 브라우징 중 방문했던 페이지의 URL을 기억하지 못하는 경우가 빈번함
- 히스토리 기능을 통해 과거 방문 기록을 시간순으로 조회하여 재방문 가능
- 북마크와 달리 히스토리는 자동으로 기록되므로 별도 저장 작업 불필요
- 대화면 환경에서 날짜별로 그룹화된 히스토리를 탐색하기 위한 리모컨 최적화 Qt UI 필요
- webOS LS2 API를 통한 영속적 데이터 저장 및 대용량 데이터 처리

---

## 2. 기능 요구사항 (Functional Requirements)

### FR-1: 자동 히스토리 기록
- **설명**: WebView에서 페이지 로드 완료 시 자동으로 히스토리에 저장합니다.
- **유저 스토리**: As a 사용자, I want 방문한 모든 페이지가 자동으로 기록되기를 원합니다, so that 나중에 방문 기록을 확인할 수 있습니다.
- **우선순위**: Must
- **상세 요구사항**:
  - WebView의 loadFinished(bool ok) 시그널 수신 시 히스토리 저장
  - 기록 조건:
    - 페이지 로드 성공 (ok == true)
    - HTTP/HTTPS 프로토콜 페이지만 기록 (file://, about: 등 제외)
    - 빈 페이지 (about:blank) 제외
  - 기록 데이터:
    - **URL** (QString): 방문한 페이지 URL
    - **제목** (QString): 페이지 타이틀 (WebView::title() 또는 빈 문자열)
    - **방문 시각** (qint64): Unix timestamp (QDateTime::currentMSecsSinceEpoch())
    - **파비콘** (QString, 선택): 페이지 파비콘 URL
  - 중복 URL 처리:
    - 동일 URL을 짧은 시간(5분) 내 재방문 시 기존 기록 업데이트 (visitCount 증가)
    - 5분 이상 경과 시 새 히스토리 항목 생성
  - HistoryService::addHistory(QString url, QString title) 호출 → LS2 API로 데이터 저장
  - 자동 기록이므로 사용자 UI 피드백 불필요 (조용히 저장)

### FR-2: 히스토리 목록 조회 (전체, 날짜별)
- **설명**: 저장된 히스토리를 시간순으로 조회하며, 날짜별로 그룹화하여 표시합니다.
- **유저 스토리**: As a 사용자, I want 방문 기록을 날짜별로 분류하여 볼 수 있기를 원합니다, so that 특정 날짜의 브라우징 이력을 쉽게 찾을 수 있습니다.
- **우선순위**: Must
- **상세 요구사항**:
  - HistoryPanel 컴포넌트 (QWidget 기반 패널)
  - 히스토리 메뉴 버튼으로 패널 열기 (NavigationBar 또는 리모컨 단축키)
  - 히스토리 리스트 표시 (QListWidget 또는 QTreeWidget):
    - **날짜별 그룹 헤더** (QListWidgetItem): "오늘", "어제", "YYYY-MM-DD"
    - 각 히스토리 항목 (QListWidgetItem):
      - 파비콘 (QIcon, 선택)
      - 제목 (QString, 볼드체)
      - URL (서브 텍스트, 작은 폰트)
      - 방문 시각 (QString, "HH:MM" 형식)
    - 최신순 정렬 (최근 방문 기록이 상단)
  - 리모컨 방향키로 목록 스크롤 (Qt 포커스 시스템)
  - 포커스된 히스토리 항목 하이라이트 표시 (Qt Stylesheet)
  - 빈 히스토리일 경우 QLabel로 "방문 기록이 없습니다" 표시
  - 백 버튼 또는 ESC 키로 패널 닫기 (QWidget::close())
  - HistoryService::getAllHistory(int limit) 호출 → LS2 API에서 데이터 로드 (기본 limit: 500)
  - 날짜 그룹핑 로직 (Qt C++):
    - QDateTime::currentDateTime()과 비교하여 "오늘", "어제" 판단
    - 그 외: QDateTime::toString("yyyy-MM-dd dddd") (예: "2026-02-14 금요일")

### FR-3: 히스토리 검색
- **설명**: 히스토리 목록에서 제목 또는 URL로 검색하여 원하는 기록을 빠르게 찾습니다.
- **유저 스토리**: As a 사용자, I want 히스토리에서 특정 키워드로 검색할 수 있기를 원합니다, so that 많은 기록 중에서 원하는 페이지를 빠르게 찾을 수 있습니다.
- **우선순위**: Must
- **상세 요구사항**:
  - HistoryPanel 상단에 QLineEdit 검색 입력 필드
  - 리모컨 가상 키보드로 검색어 입력
  - QLineEdit::textChanged 시그널로 실시간 필터링
  - 검색 범위:
    - 페이지 제목 (QString::contains(), 대소문자 무시)
    - URL (QString::contains(), 대소문자 무시)
  - 검색 결과 QListWidget 업데이트 (일치하는 히스토리만 표시)
  - 검색어 하이라이트 (선택적 기능, QTextDocument::setHtml())
  - 검색 결과 개수 표시 (QLabel, "X개 결과 찾음")
  - 검색어 지우기 버튼 (QPushButton, 검색 필드 옆)
  - HistoryService::searchHistory(QString query) 호출 또는 클라이언트 측 필터링

### FR-4: 히스토리 실행 (페이지 열기)
- **설명**: 히스토리 목록에서 항목을 선택하면 해당 페이지를 WebView에서 로드합니다.
- **유저 스토리**: As a 사용자, I want 히스토리 항목을 선택하면 해당 페이지가 즉시 열리기를 원합니다, so that 이전에 방문했던 페이지를 빠르게 재방문할 수 있습니다.
- **우선순위**: Must
- **상세 요구사항**:
  - 히스토리 항목 더블클릭 또는 선택 버튼 (Enter 키) 시 해당 URL로 페이지 로드
  - 새 탭에서 열기 옵션 제공 (컨텍스트 메뉴 또는 Ctrl+클릭)
  - 로드 시작 시 LoadingIndicator 표시 (F-05와 연계)
  - 로드 성공 시 HistoryPanel 자동 닫기 (설정 가능)
  - 로드 실패 시 QMessageBox 에러 메시지 표시 (F-10과 연계)
  - WebView::load(QUrl) 호출 또는 emit historySelected(url) 시그널
  - 히스토리 항목 실행 시 visitCount 증가 (HistoryService::incrementVisitCount())

### FR-5: 히스토리 삭제 (단일, 날짜별, 전체)
- **설명**: 히스토리 목록에서 개별 항목, 특정 날짜 기록, 또는 전체 기록을 삭제합니다.
- **유저 스토리**: As a 사용자, I want 불필요한 방문 기록을 삭제할 수 있기를 원합니다, so that 프라이버시를 보호하고 히스토리를 깔끔하게 유지할 수 있습니다.
- **우선순위**: Must
- **상세 요구사항**:
  - **단일 항목 삭제**:
    - 히스토리 항목 컨텍스트 메뉴 → "삭제" 메뉴 항목 또는 Delete 키
    - 삭제 확인 QMessageBox 표시 ("이 기록을 삭제하시겠습니까?")
    - 확인 버튼으로 삭제 (HistoryService::deleteHistory(QString id)), 취소 버튼으로 닫기
    - 삭제 성공 시 QListWidget에서 항목 즉시 제거, 상태바 메시지 ("기록이 삭제되었습니다")
  - **날짜별 삭제**:
    - 날짜 그룹 헤더 컨텍스트 메뉴 → "이 날짜의 기록 모두 삭제"
    - 삭제 확인 QMessageBox 표시 ("YYYY-MM-DD의 기록 X개를 삭제하시겠습니까?")
    - 확인 시 HistoryService::deleteHistoryByDate(QDate date)
    - LS2 API로 해당 날짜 범위의 모든 히스토리 삭제 (비동기 배치 삭제)
  - **전체 삭제**:
    - HistoryPanel 하단에 "전체 삭제" QPushButton
    - 삭제 확인 QMessageBox 표시 ("모든 방문 기록을 삭제하시겠습니까? 이 작업은 되돌릴 수 없습니다")
    - 확인 시 HistoryService::clearAllHistory()
    - LS2 API로 모든 히스토리 데이터 삭제 (비동기 배치 삭제)
    - 삭제 완료 시 QListWidget 초기화, "방문 기록이 없습니다" 표시
  - 삭제 후 포커스는 이전 항목 또는 다음 항목으로 이동 (Qt::Key_Up/Down 이벤트)

### FR-6: 방문 횟수 및 시간 표시
- **설명**: 히스토리 항목에 방문 횟수와 최근 방문 시각을 표시합니다.
- **유저 스토리**: As a 사용자, I want 각 페이지를 몇 번 방문했는지 확인할 수 있기를 원합니다, so that 자주 방문하는 사이트를 파악할 수 있습니다.
- **우선순위**: Should
- **상세 요구사항**:
  - 히스토리 항목에 방문 횟수 표시:
    - QListWidgetItem 서브 텍스트에 "방문 X회" 표시
    - visitCount 값 표시 (HistoryEntry::visitCount)
  - 최근 방문 시각 표시:
    - "오늘 14:30", "어제 09:15", "2026-02-10 16:45" 형식
    - QDateTime::toString() 사용
  - 방문 횟수 순 정렬 옵션 제공 (QComboBox, "최신순", "방문 횟수순")
  - 동일 URL 재방문 시 visitCount 자동 증가 (FR-1 참조)

### FR-7: 히스토리 필터 (날짜 범위, 도메인)
- **설명**: 히스토리 목록을 특정 날짜 범위 또는 도메인으로 필터링합니다.
- **유저 스토리**: As a 사용자, I want 특정 기간이나 특정 사이트의 기록만 볼 수 있기를 원합니다, so that 원하는 정보를 빠르게 찾을 수 있습니다.
- **우선순위**: Could (선택적 기능)
- **상세 요구사항**:
  - **날짜 범위 필터**:
    - HistoryPanel 상단에 날짜 선택 위젯 (QDateEdit, 시작일-종료일)
    - "최근 1주일", "최근 1개월", "전체" 프리셋 버튼 (QPushButton)
    - HistoryService::getHistoryByDateRange(QDate start, QDate end) 호출
  - **도메인 필터**:
    - QComboBox 드롭다운: 방문한 도메인 목록 (예: "youtube.com", "naver.com")
    - 도메인 선택 시 해당 도메인의 히스토리만 표시
    - HistoryService::getHistoryByDomain(QString domain) 호출
  - 필터 초기화 버튼 (QPushButton, "전체 보기")

### FR-8: 히스토리 데이터 영속성 (LS2 API 저장)
- **설명**: 히스토리 데이터를 webOS LS2 API를 통해 저장하여 앱 재시작 후에도 유지됩니다.
- **유저 스토리**: As a 사용자, I want 방문 기록이 앱을 종료해도 사라지지 않기를 원합니다, so that 다음에 앱을 실행할 때도 히스토리를 확인할 수 있습니다.
- **우선순위**: Must
- **상세 요구사항**:
  - HistoryService 클래스: StorageService를 통해 LS2 API 호출
  - 히스토리 데이터 구조 (C++ struct):
    ```cpp
    struct HistoryEntry {
        QString id;           // UUID (QUuid::createUuid())
        QString url;          // 방문 URL
        QString title;        // 페이지 제목
        QString domain;       // 도메인 (QUrl::host(), 검색 최적화용)
        QString favicon;      // 파비콘 URL (선택)
        qint64 visitedAt;     // 방문 시각 (Unix timestamp)
        qint64 lastVisitedAt; // 최근 방문 시각 (재방문 시 업데이트)
        int visitCount;       // 방문 횟수 (초기값 1, 재방문 시 증가)
    };
    ```
  - LS2 API 저장 형식 (JSON):
    - StorageService::call("luna://com.webos.service.db/put", QJsonObject)
    - 데이터베이스 이름: "com.jsong.webosbrowser.native.history"
    - Kind: "com.jsong.webosbrowser.native.history:1"
  - 앱 초기화 시 히스토리 데이터 로드:
    - HistoryService::init() → StorageService::call("luna://com.webos.service.db/find")
    - 최근 500개 또는 최근 30일 데이터만 로드 (성능 최적화)
  - 비동기 LS2 API 호출 처리 (Qt Signal/Slot):
    - StorageService::dataLoaded(QJsonObject) 시그널
    - HistoryService::onDataLoaded(QJsonObject) 슬롯
  - 히스토리 인덱싱:
    - URL 인덱스 (중복 체크용)
    - visitedAt 인덱스 (날짜별 조회용)
    - domain 인덱스 (도메인별 필터용)

### FR-9: 리모컨 키 매핑
- **설명**: webOS 리모컨 물리 버튼을 히스토리 기능에 매핑합니다.
- **유저 스토리**: As a 리모컨 사용자, I want 리모컨 버튼만으로 히스토리 관리의 모든 기능을 사용할 수 있기를 원합니다, so that 마우스 없이 편리하게 히스토리를 조회하고 삭제할 수 있습니다.
- **우선순위**: Must
- **상세 요구사항**:
  - Qt 키 이벤트 처리 (QWidget::keyPressEvent()):
    - Qt::Key_Up/Down: 히스토리 목록 스크롤 (QListWidget::setCurrentRow())
    - Qt::Key_Return/Enter: 히스토리 실행 (페이지 열기)
    - Qt::Key_Escape/Back: HistoryPanel 닫기
    - Qt::Key_Delete: 히스토리 삭제 (단일 항목)
    - Qt::Key_PageUp/PageDown: 빠른 스크롤 (10개 항목씩)
  - 리모컨 컬러 버튼 매핑 (webOS 리모컨 키코드):
    - 빨강 (0x193): 단일 항목 삭제
    - 파랑 (0x195): 날짜별 삭제
    - 노랑 (0x194): 전체 삭제
    - 초록 (0x192): 검색 모드 전환
  - 옵션 버튼 (Qt::Key_Menu): 컨텍스트 메뉴 열기 (QMenu::exec())
  - 포커스 가시성 향상:
    - Qt Stylesheet로 포커스된 항목 배경색/테두리 강조
    - QListWidget::item:focus { border: 3px solid #0078d7; }

---

## 3. 비기능 요구사항 (Non-Functional Requirements)

### 성능
- **히스토리 로딩 시간**: 앱 시작 시 LS2 API에서 최근 500개 히스토리 로드 2초 이내
- **히스토리 추가 반응 속도**: 페이지 로드 완료 후 0.3초 이내에 히스토리 저장 (비동기, UI 블로킹 없음)
- **목록 스크롤 성능**: 히스토리 500개 이상일 때도 스크롤 버벅임 없이 60fps 유지 (QListWidget 최적화)
- **검색 응답 속도**: 검색어 입력 후 0.5초 이내에 검색 결과 표시 (클라이언트 측 필터링)
- **대량 삭제 성능**: 전체 히스토리 삭제 시 5초 이내 완료 (LS2 API 배치 삭제)
- **메모리 사용량**: 히스토리 데이터 및 HistoryPanel UI는 최대 100MB 이하

### 가독성 (대화면 최적화)
- **폰트 크기**:
  - 히스토리 제목: 최소 18px (QFont::setPointSize(18))
  - URL 서브 텍스트: 최소 14px
  - 날짜 그룹 헤더: 최소 22px (볼드체)
  - 방문 시각: 최소 12px (작은 텍스트)
- **대비**: 배경과 전경 색상 대비 비율 4.5:1 이상 (WCAG AA 기준)
- **포커스 표시**: 포커스된 히스토리 항목은 3px 두께 테두리 또는 명확한 배경색 변화
- **아이콘 크기**: 파비콘 최소 24x24px (QIcon, QPixmap)
- **날짜 그룹 구분**: 날짜 헤더 배경색 차별화 (밝은 회색) 및 구분선 (QFrame::HLine)

### UX (리모컨 최적화)
- **포커스 경로**: 리모컨 방향키로 모든 히스토리 항목, 버튼에 접근 가능 (Qt Tab Order)
- **날짜별 그룹핑**: 날짜 헤더로 시각적 구분 명확
- **빠른 스크롤**: PageUp/PageDown 키로 10개 항목씩 이동 (대량 히스토리 탐색)
- **피드백 제공**: 히스토리 삭제 시 상태바 메시지 또는 Qt Notification
- **에러 복구**: 삭제 후 "실행 취소" 옵션 제공 (선택적 기능, QUndoStack)
- **로딩 상태 표시**: 대량 히스토리 로드 시 QProgressBar 또는 "로딩 중..." 메시지

### 데이터 안정성
- **LS2 API 에러 처리**:
  - 네트워크 오류, 타임아웃, 잘못된 응답 처리
  - StorageService::error(QString) 시그널 → HistoryService::onError(QString) 슬롯
  - QMessageBox로 에러 메시지 표시
- **데이터 무결성**:
  - 중복 URL 체크 (5분 이내 재방문 시 업데이트, 아니면 새 항목 생성)
  - 잘못된 URL 필터링 (빈 URL, about:blank 제외)
  - LS2 API 트랜잭션 실패 시 롤백 (가능한 경우)
- **대용량 데이터 처리**:
  - 히스토리 1000개 이상 축적 시 자동 정리 (30일 이전 데이터 삭제, 설정 가능)
  - LS2 API 페이지네이션 (limit, offset) 사용
  - QListWidget 가상화 스크롤링 (QAbstractItemView::ScrollPerPixel)

### 프라이버시
- **히스토리 자동 삭제**: 설정에서 히스토리 보관 기간 설정 (1주일, 1개월, 3개월, 무제한)
- **시크릿 모드 지원 준비**: 향후 시크릿 모드 추가 시 히스토리 기록 안 함 (플래그 추가)
- **민감한 URL 표시**: 히스토리 목록에서 URL 길이 제한 (최대 100자, "..." 생략 표시)

### 확장성
- **히스토리 동기화**: 향후 LG 계정 연동으로 히스토리 클라우드 동기화 가능하도록 서비스 계층 모듈화
- **히스토리 가져오기/내보내기**: HTML 히스토리 파일 가져오기/내보내기 기능 추가 가능 (QFile, QJsonDocument)
- **최대 히스토리 개수**: 10,000개 이상 히스토리 지원 (LS2 API 인덱싱 및 페이지네이션)
- **AI 기반 추천**: 향후 히스토리 분석으로 자주 방문하는 시간대/사이트 패턴 인사이트 제공

### 보안
- **URL 검증**: 히스토리 기록 시 URLValidator::isValid(QString url) 호출 (XSS 방지)
- **XSS 방지**: 히스토리 제목, URL에 HTML 태그 이스케이프 처리 (QString::toHtmlEscaped())
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
- **화면 해상도**: 4K UHD (3840x2160) 기본, Full HD (1920x1080) 호환

### 기술 제약사항
- **프레임워크**: Qt 5.15+ (C++17 표준)
- **UI 컴포넌트**: Qt Widgets (QListWidget, QTreeWidget, QDialog, QPushButton, QLineEdit, QDateEdit 등)
- **스타일링**: Qt Stylesheet (.qss 파일) 또는 QStyle
- **상태 관리**: Qt Signal/Slot 시스템
- **데이터 저장**: webOS LS2 API (com.webos.service.db)
  - JSON 형식 데이터 저장 (QJsonObject, QJsonArray)
  - 비동기 호출 (Qt 이벤트 루프)
- **히스토리 용량**: LS2 API DB8 용량 제한 (앱당 최대 100MB)
  - 히스토리 1개당 약 1KB 가정 → 최대 약 100,000개 저장 가능

### 메모리 제약
- **프로젝터 RAM**: LG HU715QW는 제한적인 RAM (1~2GB 추정)
- **HistoryPanel 메모리**: 히스토리 1000개일 때 최대 100MB 이하
- **QListWidget 최적화**:
  - QListWidget::setUniformItemSizes(true) 사용
  - 가상화 스크롤링 고려 (QAbstractItemView::ScrollPerPixel)
  - 초기 로딩 제한 (최근 500개만 로드, 추가 로드는 Lazy Loading)

### 대용량 데이터 처리
- **히스토리 개수**: 수천~수만 개 레코드 가능
- **로딩 전략**:
  - 초기 로딩: 최근 500개 또는 최근 30일 데이터
  - 추가 로딩: 사용자가 스크롤 하단 도달 시 Lazy Loading (다음 500개)
- **삭제 전략**:
  - 자동 정리: 설정된 보관 기간 경과 데이터 자동 삭제 (백그라운드 작업)
  - 배치 삭제: 전체 삭제 또는 날짜별 삭제 시 LS2 API 배치 쿼리 사용
- **인덱싱**:
  - LS2 API DB8 인덱스 생성 (visitedAt, domain) → 조회 성능 향상

### 의존성
- **F-02 (웹뷰 통합)**:
  - 히스토리 자동 기록 시 WebView::loadFinished(bool ok) 시그널 수신 필요
  - WebView::url(), WebView::title() 메서드 호출 필요
- **F-01 (프로젝트 초기 설정)**: Qt/C++ 프로젝트 구조, CMakeLists.txt, 네임스페이스 설정 완료 필요
- **StorageService**: LS2 API 래퍼 클래스 구현 완료 필요 (비동기 호출, JSON 파싱)

### 범위 외 (Out of Scope)
- **히스토리 동기화**: 외부 브라우저 또는 클라우드와 히스토리 동기화 미지원 (향후 확장 가능)
- **히스토리 가져오기/내보내기**: HTML 히스토리 파일 가져오기/내보내기 미지원
- **파비콘 자동 다운로드**: 히스토리 기록 시 파비콘 자동 다운로드 미지원 (선택적 기능)
- **히스토리 태그**: 히스토리에 태그 추가 기능 미지원
- **히스토리 통계**: 방문 패턴 분석, 그래프 표시 미지원 (향후 확장 가능)
- **시크릿 모드**: 현재 프로젝트 범위 외 (M3 이후 확장 가능)

---

## 5. 용어 정의

| 용어 | 정의 |
|------|------|
| **히스토리** | 사용자가 방문한 웹 페이지의 URL, 제목, 방문 시각을 기록한 항목 (C++ HistoryEntry 구조체) |
| **HistoryPanel** | 히스토리 목록을 표시하고 관리하는 Qt QWidget 기반 패널 컴포넌트 |
| **HistoryService** | 히스토리 데이터의 CRUD 작업을 담당하는 QObject 기반 서비스 계층 클래스 |
| **날짜 그룹** | 히스토리 목록에서 동일 날짜의 항목을 그룹화한 섹션 (예: "오늘", "어제", "2026-02-10") |
| **방문 횟수** | 동일 URL을 여러 번 방문한 횟수 (HistoryEntry::visitCount, 초기값 1) |
| **최근 방문 시각** | 동일 URL의 가장 최근 방문 시각 (HistoryEntry::lastVisitedAt, 재방문 시 업데이트) |
| **LS2 API** | webOS의 Luna Service 2 메시지 버스 API (com.webos.service.db) |
| **StorageService** | LS2 API를 래핑하여 비동기 데이터 저장/조회를 제공하는 QObject 클래스 |
| **DB8** | webOS의 JSON 기반 NoSQL 데이터베이스 (LS2 API로 접근) |
| **파비콘** | 웹사이트의 작은 아이콘 이미지 (QIcon, 16x16px 또는 24x24px) |
| **컨텍스트 메뉴** | 히스토리 항목에서 추가 작업(삭제, 새 탭에서 열기 등)을 선택할 수 있는 Qt QMenu 팝업 |
| **시그널/슬롯** | Qt의 이벤트 처리 메커니즘 (emit 시그널 → connect된 슬롯 함수 호출) |
| **Lazy Loading** | 초기 로딩 시 일부 데이터만 로드하고, 필요 시 추가 데이터를 로드하는 방식 (성능 최적화) |

---

## 6. UI/UX 플로우

### 플로우 1: 자동 히스토리 기록
1. 사용자가 웹 페이지 탐색 (URLBar에 URL 입력 또는 링크 클릭)
2. WebView::load(QUrl) 호출 → 페이지 로딩 시작
3. WebView::loadFinished(bool ok) 시그널 emit
4. BrowserWindow::onLoadFinished(bool ok) 슬롯 수신:
   - ok == true이고 HTTP/HTTPS 프로토콜인 경우
   - WebView::url(), WebView::title() 호출하여 현재 페이지 정보 가져옴
5. HistoryService::addHistory(QString url, QString title) 호출
6. HistoryService 내부:
   - 중복 URL 체크 (최근 5분 이내 방문 기록 존재 여부)
   - 중복 시: 기존 기록의 visitCount 증가, lastVisitedAt 업데이트
   - 중복 아님: 새 HistoryEntry 생성 (UUID, visitCount=1)
7. StorageService::put(QJsonObject) 호출 → LS2 API로 비동기 저장
8. 저장 성공 시 emit HistoryService::historyAdded(HistoryEntry) 시그널 (UI 업데이트용)
9. 사용자에게 별도 피드백 없음 (자동 기록이므로 조용히 처리)

### 플로우 2: 히스토리 목록 조회
1. 사용자가 NavigationBar의 "히스토리" QPushButton 클릭 (또는 리모컨 단축키)
2. HistoryPanel (QWidget) 열기 (슬라이드 인 애니메이션)
3. HistoryService::getAllHistory(int limit=500) 호출 → LS2 API에서 데이터 로드
4. LS2 API 응답 수신 → emit HistoryService::dataLoaded(QVector<HistoryEntry>) 시그널
5. HistoryPanel::onDataLoaded(QVector<HistoryEntry> entries) 슬롯:
   - 날짜별 그룹핑 로직 실행 (오늘, 어제, YYYY-MM-DD)
   - QListWidget에 날짜 그룹 헤더 추가 (QListWidgetItem, 배경색 회색)
   - 각 그룹 아래 히스토리 항목 추가 (파비콘, 제목, URL, 방문 시각)
6. 사용자가 리모컨 방향키로 히스토리 탐색 (포커스 이동)
7. 히스토리 선택 (Enter 키 또는 더블클릭)
8. emit HistoryPanel::historySelected(QString url) 시그널
9. BrowserWindow::onHistorySelected(QString url) 슬롯 → WebView::load(QUrl)
10. HistoryPanel 자동 닫기 (슬라이드 아웃 애니메이션)
11. HistoryService::incrementVisitCount(QString id) 호출 (백그라운드)

### 플로우 3: 히스토리 검색
1. HistoryPanel에서 검색 QLineEdit에 포커스 (초록 버튼 또는 클릭)
2. 리모컨 가상 키보드로 검색어 입력
3. QLineEdit::textChanged(QString text) 시그널 emit
4. HistoryPanel::onSearchTextChanged(QString text) 슬롯:
   - text가 빈 문자열이면 전체 히스토리 표시
   - text가 있으면 클라이언트 측 필터링:
     - entries 벡터를 순회하며 title 또는 url에 text 포함 여부 체크
     - 일치하는 항목만 QListWidget에 표시
5. 검색 결과 개수 표시 (QLabel, "X개 결과 찾음")
6. 검색 결과에서 히스토리 선택 → 플로우 2와 동일하게 페이지 로드

### 플로우 4: 히스토리 삭제 (단일)
1. HistoryPanel에서 히스토리 항목 선택 후 Delete 키 (또는 컨텍스트 메뉴 → "삭제")
2. 삭제 확인 QMessageBox 표시 ("이 기록을 삭제하시겠습니까?")
3. "확인" 버튼 클릭
4. HistoryService::deleteHistory(QString id) 호출 → LS2 API로 삭제
5. 삭제 성공 시 emit HistoryService::historyDeleted(QString id) 시그널
6. HistoryPanel::onHistoryDeleted(QString id) 슬롯:
   - QListWidget에서 해당 항목 제거 (QListWidget::takeItem())
   - 상태바 메시지 표시 ("기록이 삭제되었습니다")
7. 포커스를 다음 항목으로 이동 (Qt::Key_Down)
8. QMessageBox 닫기

### 플로우 5: 히스토리 전체 삭제
1. HistoryPanel 하단의 "전체 삭제" QPushButton 클릭 (또는 리모컨 노랑 버튼)
2. 삭제 확인 QMessageBox 표시:
   - "모든 방문 기록을 삭제하시겠습니까? 이 작업은 되돌릴 수 없습니다"
   - "확인" / "취소" 버튼
3. "확인" 버튼 클릭
4. HistoryService::clearAllHistory() 호출 → LS2 API 배치 삭제
5. 삭제 진행 중 QProgressDialog 표시 ("히스토리 삭제 중...")
6. 삭제 완료 시 emit HistoryService::allHistoryCleared() 시그널
7. HistoryPanel::onAllHistoryCleared() 슬롯:
   - QListWidget 초기화 (clear())
   - QLabel로 "방문 기록이 없습니다" 표시
   - 상태바 메시지 ("모든 기록이 삭제되었습니다")
8. QProgressDialog 닫기

### 플로우 6: 날짜별 히스토리 삭제
1. HistoryPanel에서 날짜 그룹 헤더 선택 후 옵션 버튼 (컨텍스트 메뉴)
2. 컨텍스트 QMenu 표시: "이 날짜의 기록 모두 삭제"
3. "이 날짜의 기록 모두 삭제" 메뉴 항목 선택
4. 삭제 확인 QMessageBox 표시:
   - "2026-02-14의 기록 25개를 삭제하시겠습니까?"
   - "확인" / "취소" 버튼
5. "확인" 버튼 클릭
6. HistoryService::deleteHistoryByDate(QDate date) 호출:
   - date의 00:00:00 ~ 23:59:59 범위의 히스토리 모두 삭제
   - LS2 API 배치 쿼리 (where: visitedAt >= startTimestamp AND visitedAt <= endTimestamp)
7. 삭제 성공 시 emit HistoryService::historyDeletedByDate(QDate date) 시그널
8. HistoryPanel::onHistoryDeletedByDate(QDate date) 슬롯:
   - QListWidget에서 해당 날짜 그룹 및 하위 항목 모두 제거
   - 상태바 메시지 ("2026-02-14의 기록이 삭제되었습니다")
9. QMessageBox 닫기

---

## 7. 데이터 모델

### HistoryEntry 구조체 (C++)
```cpp
namespace webosbrowser {

struct HistoryEntry {
    QString id;             // UUID (QUuid::createUuid().toString())
    QString url;            // 방문 URL (예: "https://www.youtube.com/watch?v=...")
    QString title;          // 페이지 제목 (예: "YouTube - Trending Videos")
    QString domain;         // 도메인 (예: "www.youtube.com", QUrl::host()로 추출)
    QString favicon;        // 파비콘 URL (선택, 기본 빈 문자열)
    qint64 visitedAt;       // 첫 방문 시각 (Unix timestamp, QDateTime::currentMSecsSinceEpoch())
    qint64 lastVisitedAt;   // 최근 방문 시각 (재방문 시 업데이트)
    int visitCount;         // 방문 횟수 (초기값 1, 재방문 시 증가)

    // JSON 직렬화
    QJsonObject toJson() const;
    static HistoryEntry fromJson(const QJsonObject &json);

    // 날짜 그룹핑 헬퍼
    QString getDateGroup() const {
        QDateTime dateTime = QDateTime::fromMSecsSinceEpoch(lastVisitedAt);
        QDate date = dateTime.date();
        QDate today = QDate::currentDate();

        if (date == today) {
            return "오늘";
        } else if (date == today.addDays(-1)) {
            return "어제";
        } else {
            return date.toString("yyyy-MM-dd dddd");
        }
    }

    // 방문 시각 표시용 문자열
    QString getVisitTimeString() const {
        QDateTime dateTime = QDateTime::fromMSecsSinceEpoch(lastVisitedAt);
        return dateTime.toString("HH:mm");
    }
};

} // namespace webosbrowser
```

### LS2 API 저장 형식 (JSON)
```json
// 히스토리 데이터 (com.webos.service.db/put)
{
  "_kind": "com.jsong.webosbrowser.native.history:1",
  "id": "660e8400-e29b-41d4-a716-446655440002",
  "url": "https://www.youtube.com/watch?v=dQw4w9WgXcQ",
  "title": "Rick Astley - Never Gonna Give You Up (Official Video)",
  "domain": "www.youtube.com",
  "favicon": "https://www.youtube.com/favicon.ico",
  "visitedAt": 1620000000000,
  "lastVisitedAt": 1620003600000,
  "visitCount": 3
}
```

### LS2 API 호출 예시
```cpp
// 히스토리 추가 (StorageService 래퍼)
QJsonObject history;
history["_kind"] = "com.jsong.webosbrowser.native.history:1";
history["id"] = QUuid::createUuid().toString();
history["url"] = "https://www.youtube.com";
history["title"] = "YouTube";
history["domain"] = QUrl("https://www.youtube.com").host();
history["visitedAt"] = QDateTime::currentMSecsSinceEpoch();
history["lastVisitedAt"] = QDateTime::currentMSecsSinceEpoch();
history["visitCount"] = 1;

storageService->put(history);

// 히스토리 조회 (최근 500개, 최신순)
QJsonObject query;
query["_kind"] = "com.jsong.webosbrowser.native.history:1";
query["limit"] = 500;
query["orderBy"] = "lastVisitedAt";
query["desc"] = true;

storageService->find(query);

// 날짜별 삭제 (특정 날짜 범위)
QJsonObject deleteQuery;
deleteQuery["_kind"] = "com.jsong.webosbrowser.native.history:1";
deleteQuery["where"] = QJsonArray({
    QJsonObject({{"prop", "visitedAt"}, {"op", ">="}, {"val", startTimestamp}}),
    QJsonObject({{"prop", "visitedAt"}, {"op", "<="}, {"val", endTimestamp}})
});

storageService->del(deleteQuery);
```

### 중복 URL 처리 로직
```cpp
// HistoryService::addHistory() 내부
bool HistoryService::addHistory(const QString &url, const QString &title) {
    // 1. 최근 5분 이내 동일 URL 방문 기록 조회
    qint64 fiveMinutesAgo = QDateTime::currentMSecsSinceEpoch() - (5 * 60 * 1000);

    QJsonObject query;
    query["_kind"] = "com.jsong.webosbrowser.native.history:1";
    query["where"] = QJsonArray({
        QJsonObject({{"prop", "url"}, {"op", "="}, {"val", url}}),
        QJsonObject({{"prop", "lastVisitedAt"}, {"op", ">="}, {"val", fiveMinutesAgo}})
    });
    query["limit"] = 1;

    // 2. 비동기 조회 결과 처리
    connect(storageService, &StorageService::findCompleted, this, [=](const QJsonArray &results) {
        if (results.size() > 0) {
            // 중복 발견: 기존 기록 업데이트
            QJsonObject existing = results[0].toObject();
            QString id = existing["id"].toString();
            int visitCount = existing["visitCount"].toInt() + 1;
            qint64 now = QDateTime::currentMSecsSinceEpoch();

            QJsonObject update;
            update["_id"] = existing["_id"].toString();
            update["visitCount"] = visitCount;
            update["lastVisitedAt"] = now;
            update["title"] = title; // 제목 업데이트 (페이지 제목 변경 가능)

            storageService->merge(update);
        } else {
            // 중복 없음: 새 히스토리 생성
            HistoryEntry entry;
            entry.id = QUuid::createUuid().toString();
            entry.url = url;
            entry.title = title;
            entry.domain = QUrl(url).host();
            entry.visitedAt = QDateTime::currentMSecsSinceEpoch();
            entry.lastVisitedAt = entry.visitedAt;
            entry.visitCount = 1;

            storageService->put(entry.toJson());
        }
    });

    storageService->find(query);
    return true;
}
```

---

## 8. 에러 케이스

### LS2 API 에러 처리
| 에러 상황 | 원인 | 대응 방안 |
|-----------|------|-----------|
| 네트워크 타임아웃 | LS2 서비스 응답 없음 | QMessageBox 에러 표시 ("히스토리 로드 실패: 네트워크 오류"), 재시도 버튼 제공 |
| DB8 용량 초과 | 앱당 100MB 제한 초과 | QMessageBox 경고 ("저장 공간 부족: 오래된 기록을 삭제해주세요"), 자동 정리 실행 |
| 잘못된 JSON 형식 | LS2 API 응답 파싱 실패 | Logger::error() 호출, 기본 빈 배열 반환 |
| 권한 거부 | appinfo.json에 권한 없음 | QMessageBox 에러 ("앱 권한 오류: 앱을 재설치해주세요") |

### 대량 히스토리 로딩 실패
- **시나리오**: 앱 시작 시 LS2 API에서 히스토리 10,000개 로드 시도 → 타임아웃 또는 메모리 부족
- **대응**:
  1. 초기 로딩 제한 (최근 500개 또는 최근 30일만)
  2. Lazy Loading 구현: 사용자가 스크롤 하단 도달 시 추가 500개 로드
  3. QListWidget::verticalScrollBar()->valueChanged() 시그널로 스크롤 위치 감지
  4. 스크롤 하단 (value >= maximum - 100) 시 HistoryService::loadMoreHistory() 호출

### 중복 URL 처리 실패
- **시나리오**: 5분 이내 동일 URL 재방문 체크 쿼리 실패
- **대응**:
  1. 쿼리 실패 시 새 히스토리 항목 생성 (중복되더라도 기록 누락 방지)
  2. Logger::warning("중복 체크 실패, 새 항목 생성") 호출
  3. 향후 백그라운드 작업으로 중복 항목 병합 (선택적 기능)

### 잘못된 URL 처리
- **시나리오**: WebView가 빈 URL 또는 잘못된 프로토콜 (about:blank, file://) 로드
- **대응**:
  1. HistoryService::addHistory() 내부에서 URL 검증:
     - URLValidator::isValid(url) 호출
     - 프로토콜 체크 (http://, https://만 허용)
     - 빈 URL 또는 about:blank 제외
  2. 유효하지 않은 URL이면 히스토리 기록 스킵
  3. Logger::info("히스토리 기록 스킵: 유효하지 않은 URL") 호출

### 날짜별 삭제 시 대량 데이터 처리
- **시나리오**: 특정 날짜에 1000개 이상 히스토리 존재 → 삭제 시 타임아웃
- **대응**:
  1. LS2 API 배치 삭제 쿼리 사용 (del with where clause)
  2. 삭제 진행 중 QProgressDialog 표시
  3. 타임아웃 발생 시 재시도 로직 구현 (최대 3회)
  4. 재시도 실패 시 QMessageBox 에러 ("삭제 실패: 일부 기록이 남아있을 수 있습니다")

### 앱 초기화 시 데이터 로드 실패
- **시나리오**: 앱 시작 시 LS2 API에서 히스토리 로드 실패
- **대응**:
  1. HistoryService::init() → LS2 API error 시그널 수신
  2. Logger::error("히스토리 로드 실패") 호출
  3. HistoryPanel 열 때 빈 목록 표시 ("방문 기록이 없습니다")
  4. 재시도 버튼 제공 (HistoryService::init() 재호출)

---

## 9. 완료 기준 (Definition of Done)

### DoD-1: 기능 완성도
- [ ] 페이지 로드 완료 시 히스토리 자동 기록 (LS2 API 저장)
- [ ] 히스토리 목록 조회 (전체, 날짜별 그룹핑)
- [ ] 히스토리 검색 (제목, URL)
- [ ] 히스토리 삭제 (단일, 날짜별, 전체)
- [ ] 히스토리 선택 시 WebView에서 페이지 로드
- [ ] 리모컨 방향키 및 단축키로 모든 기능 사용 가능
- [ ] 앱 재시작 후 저장된 히스토리 데이터 정상 로드
- [ ] 방문 횟수 및 시간 표시

### DoD-2: 테스트 통과
- [ ] 단위 테스트: HistoryService 클래스 (Google Test)
  - addHistory() 테스트 (중복 URL 처리 포함)
  - getAllHistory(), getHistoryByDateRange() 테스트
  - deleteHistory(), clearAllHistory() 테스트
  - 날짜 그룹핑 로직 테스트
- [ ] 통합 테스트: HistoryPanel UI (Qt Test)
  - 히스토리 목록 QListWidget 표시 테스트
  - 날짜 그룹 헤더 생성 테스트
  - 검색 필터링 테스트
  - 히스토리 실행 시 historySelected 시그널 emit 테스트
- [ ] 성능 테스트:
  - 1000개 히스토리 로딩 시간 (2초 이내)
  - 전체 삭제 성능 (5초 이내)
  - 검색 응답 속도 (0.5초 이내)
- [ ] 수동 테스트: 실제 webOS 프로젝터 환경에서 리모컨 테스트
  - 페이지 방문 → 히스토리 자동 기록 → 목록 조회 → 실행 전체 플로우
  - 검색 → 결과 확인 → 항목 선택 플로우
  - 삭제 (단일, 날짜별, 전체) 플로우
  - 리모컨 방향키/컬러 버튼 매핑 테스트

### DoD-3: 코드 품질
- [ ] C++ 코딩 컨벤션 준수 (CLAUDE.md 기준)
  - 주석 언어: 한국어
  - 클래스명 PascalCase, 변수명 camelCase
  - 스마트 포인터 사용 (std::unique_ptr, std::shared_ptr)
  - Qt Signal/Slot 신규 문법 사용
- [ ] 헤더 파일: `#pragma once` 사용
- [ ] 메모리 누수 없음 (Qt 부모-자식 메모리 관리)
- [ ] 에러 처리: LS2 API 에러, 중복 URL, 대량 데이터 모두 처리
- [ ] 로깅: Logger::info(), Logger::error() 적절히 사용

### DoD-4: 성능 기준
- [ ] 히스토리 로딩 시간 2초 이내 (500개 히스토리 기준)
- [ ] 히스토리 자동 기록 반응 속도 0.3초 이내 (비동기, UI 블로킹 없음)
- [ ] 목록 스크롤 60fps 유지 (1000개 이상 히스토리)
- [ ] 검색 응답 속도 0.5초 이내
- [ ] 전체 삭제 성능 5초 이내
- [ ] 메모리 사용량 100MB 이하 (HistoryPanel + HistoryService)

### DoD-5: 문서화
- [ ] 이 요구사항 분석서 (requirements.md) 완성
- [ ] 기술 설계서 (design.md) 작성 완료 (architect)
- [ ] 구현 계획서 (plan.md) 작성 완료 (product-manager)
- [ ] API 문서: HistoryService 클래스 공개 메서드 Doxygen 주석
- [ ] README 업데이트: F-08 기능 완료 명시

### DoD-6: 코드 리뷰
- [ ] code-reviewer 에이전트 리뷰 통과
  - 코드 컨벤션 준수 확인
  - 메모리 관리 적절성 확인
  - 에러 처리 완전성 확인
  - 대용량 데이터 처리 최적화 확인
- [ ] 리뷰 피드백 반영 완료

---

## 10. 참고 사항

### 관련 문서
- **PRD**: `docs/project/prd.md`
- **기능 백로그**: `docs/project/features.md`
- **프로젝트 가이드**: `CLAUDE.md`
- **북마크 관리 요구사항**: `docs/specs/bookmark-management/requirements.md` (유사 패턴 참조)
- **웹뷰 통합 요구사항**: `docs/specs/webview-integration/requirements.md` (WebView 시그널 연동)
- **URL 입력 UI 요구사항**: `docs/specs/url-input-ui/requirements.md` (가상 키보드 재사용 가능)

### 참고 기술 문서
- **Qt QListWidget**: https://doc.qt.io/qt-5/qlistwidget.html
- **Qt QTreeWidget**: https://doc.qt.io/qt-5/qtreewidget.html
- **Qt Signal/Slot**: https://doc.qt.io/qt-5/signalsandslots.html
- **Qt Stylesheet**: https://doc.qt.io/qt-5/stylesheet.html
- **Qt QDateTime**: https://doc.qt.io/qt-5/qdatetime.html
- **webOS LS2 API (DB8)**: https://webostv.developer.lge.com/develop/native-apps/native-service-overview
- **Qt JSON**: https://doc.qt.io/qt-5/json.html

### 우선순위 조정 가능성
- **FR-6 (방문 횟수 및 시간 표시)**: Should 우선순위로, 기본 히스토리 CRUD 완료 후 추가 가능
- **FR-7 (히스토리 필터)**: Could 우선순위로, M3 이후 확장 기능으로 연기 가능
- **파비콘 자동 다운로드**: 선택적 기능으로, 기본 기능 완료 후 추가 가능
- **실행 취소 (QUndoStack)**: 선택적 기능으로, M3 이후 확장 가능

### 기술적 고려사항
- **LS2 API vs SQLite**: webOS 공식 권장은 LS2 API (DB8)이나, SQLite 직접 사용도 가능. 프로젝트는 LS2 API 사용.
- **비동기 처리**: LS2 API는 비동기 호출이므로 Qt Signal/Slot으로 응답 처리 필수
- **대용량 데이터**: 히스토리는 북마크보다 데이터량이 많으므로 Lazy Loading 및 페이지네이션 필수
- **중복 URL 처리**: 5분 이내 재방문 시 visitCount 증가, 아니면 새 항목 생성
- **날짜 그룹핑**: QDateTime 연산으로 "오늘", "어제" 판단 (currentDate() 비교)
- **히스토리 용량 관리**: 자동 정리 기능 (30일 이전 데이터 삭제) 권장

### 병렬 작업 가능성
- **PG-2 병렬 그룹**: F-07 (북마크 관리), F-08 (히스토리 관리), F-09 (검색 엔진 통합)은 독립적으로 개발 가능
- **주의 사항**:
  - F-08은 F-02 (웹뷰 통합)의 loadFinished 시그널 의존 → F-02 완료 후 시작 권장
  - HistoryService와 HistoryPanel은 동일 기능 내에서 순차 개발 권장 (HistoryService 먼저 구현)
  - BookmarkService와 HistoryService는 별도 파일이므로 병렬 개발 가능

### 에이전트 라우팅
1. **요구사항 분석 (현재 단계)**: product-manager → requirements.md 작성 완료
2. **기술 설계**: architect → design.md 작성 (클래스 다이어그램, LS2 API 시퀀스 다이어그램, 날짜 그룹핑 알고리즘)
3. **구현 계획**: product-manager → plan.md 작성 (Phase별 Task, 의존성, 병렬 실행 판단)
4. **C++ 구현**: cpp-dev (필요 시 생성) → HistoryService, HistoryPanel 코드 작성
5. **테스트**: test-runner → 단위 테스트 (Google Test), 통합 테스트 (Qt Test), 성능 테스트
6. **코드 리뷰**: code-reviewer → 코드 + 문서 검증 (대용량 데이터 처리 최적화 중점 확인)
7. **문서화**: doc-writer → CHANGELOG.md, dev-log.md 업데이트

---

## 11. 변경 이력

| 날짜 | 변경 내용 | 작성자 |
|------|-----------|--------|
| 2026-02-14 | Web App PoC 요구사항을 Native App(C++/Qt)으로 전환 | product-manager |
| 2026-02-14 | IndexedDB → LS2 API 변경, Enact → Qt Widgets 변경 | product-manager |
