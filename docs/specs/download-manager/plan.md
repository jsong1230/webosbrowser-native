# 다운로드 관리 — 구현 계획서

## 1. 참조 문서
- **요구사항 분석서**: `docs/specs/download-manager/requirements.md`
- **기술 설계서**: `docs/specs/download-manager/design.md`
- **프로젝트 가이드**: `CLAUDE.md`

---

## 2. 구현 Phase

### Phase 1: DownloadManager 서비스 구현 (백엔드 로직)

**담당**: cpp-dev
**예상 소요 시간**: 8~10시간

#### Task 1.1: DownloadItem 구조체 및 DownloadState 정의
- [ ] `src/services/DownloadManager.h` 생성
- [ ] `DownloadState` 열거형 정의 (Pending, InProgress, Paused, Completed, Cancelled, Failed)
- [ ] `DownloadItem` 구조체 정의
  - 필드: id, fileName, filePath, url, mimeType, bytesTotal, bytesReceived, state, startTime, finishTime, errorMessage, speed, estimatedTimeLeft
  - Qt 메타타입 등록 (`Q_DECLARE_METATYPE`)
- [ ] 클래스 주석 작성 (한국어, Doxygen 스타일)

**예상 산출물**: `src/services/DownloadManager.h` (구조체/열거형 정의)

**완료 기준**:
- 컴파일 성공
- Doxygen 주석 완성도 80% 이상
- 모든 필드 타입이 Qt 컨테이너 호환

---

#### Task 1.2: DownloadManager 클래스 골격 구현
- [ ] `DownloadManager` 클래스 정의 (QObject 상속)
- [ ] 생성자/소멸자 구현
- [ ] 멤버 변수 선언
  - `QString m_downloadPath`
  - `QVector<DownloadItem> m_downloads`
  - `QMap<QString, QWebEngineDownloadItem*> m_activeDownloads`
  - `QMap<QString, qint64> m_lastBytesReceived`
  - `QMap<QString, qint64> m_lastUpdateTime`
- [ ] 공개 API 메서드 시그니처 선언
  - `startDownload()`, `pauseDownload()`, `resumeDownload()`, `cancelDownload()`, `removeDownload()`
  - `getDownloads()`, `getDownloadById()`
  - `setDownloadPath()`, `downloadPath()`
- [ ] 시그널 선언
  - `downloadAdded`, `downloadProgressChanged`, `downloadStateChanged`, `downloadCompleted`, `downloadFailed`
- [ ] 슬롯 선언
  - `onDownloadProgress()`, `onDownloadFinished()`, `onDownloadStateChanged()`

**예상 산출물**: `src/services/DownloadManager.h` (클래스 정의 완료)

**완료 기준**:
- 헤더 파일 컴파일 성공
- `Q_OBJECT` 매크로 포함
- 시그널/슬롯 문법 확인

---

#### Task 1.3: 다운로드 시작 로직 구현
- [ ] `src/services/DownloadManager.cpp` 생성
- [ ] `startDownload()` 구현
  - UUID 생성 (`generateDownloadId()`)
  - 다운로드 경로 설정 (QStandardPaths::DownloadLocation)
  - 파일명 중복 처리 (`resolveFilePath()`)
  - QWebEngineDownloadItem 시그널 연결 (downloadProgress, finished, stateChanged)
  - `accept()` 호출
  - DownloadItem 구조체 생성 및 m_downloads에 추가
  - `downloadAdded` 시그널 emit
- [ ] `generateDownloadId()` 구현 (QUuid::createUuid())
- [ ] `resolveFilePath()` 구현 (파일명 중복 시 "(1)", "(2)" 추가)
- [ ] `setDownloadPath()` 구현
  - QStandardPaths::DownloadLocation 기본값
  - 디렉토리 존재 확인 및 자동 생성 (QDir::mkpath)
  - 쓰기 권한 확인 (QFileInfo::isWritable)
  - 실패 시 폴백 경로 (AppDataLocation + "/downloads")

**예상 산출물**: `src/services/DownloadManager.cpp` (시작 로직)

**완료 기준**:
- 다운로드 시작 시 `downloadAdded` 시그널 수신 확인
- 파일명 중복 처리 동작 확인 (동일 파일 2회 다운로드)
- 디렉토리 자동 생성 확인

---

#### Task 1.4: 진행률 추적 및 상태 관리 구현
- [ ] `onDownloadProgress()` 구현
  - 쓰로틀링 로직 (500ms 간격)
  - 다운로드 속도 계산 (`calculateSpeed()`)
  - 남은 시간 추정 (`estimateTimeLeft()`)
  - DownloadItem 업데이트 (bytesReceived, speed, estimatedTimeLeft)
  - `downloadProgressChanged` 시그널 emit
- [ ] `calculateSpeed()` 구현
  - 이전 bytesReceived와 현재 차이 계산
  - 시간 차이로 나누어 bytes/sec 산출
- [ ] `estimateTimeLeft()` 구현
  - (bytesTotal - bytesReceived) / speed
  - 속도가 0이면 -1 반환 (알 수 없음)
- [ ] `onDownloadStateChanged()` 구현
  - Qt 상태를 DownloadState로 변환 (`convertQtState()`)
  - `downloadStateChanged` 시그널 emit
- [ ] `convertQtState()` 구현
  - QWebEngineDownloadItem::DownloadState → DownloadState 매핑

**예상 산출물**: `src/services/DownloadManager.cpp` (진행률 로직)

**완료 기준**:
- 다운로드 진행 시 `downloadProgressChanged` 시그널 500ms 간격으로 수신
- 속도 계산 정확도 (실제 다운로드 속도 ±10%)
- ETA 추정 오차 ±30% 이내

---

#### Task 1.5: 완료 및 에러 처리 구현
- [ ] `onDownloadFinished()` 구현
  - 다운로드 상태 확인 (Completed vs Failed)
  - 성공 시:
    - 파일 존재 확인 (QFile::exists)
    - 파일 크기 검증 (QFileInfo::size == bytesTotal)
    - 상태 변경: Completed
    - finishTime 기록
    - `downloadCompleted` 시그널 emit
    - m_activeDownloads에서 제거
  - 실패 시:
    - 에러 메시지 생성 (네트워크/디스크/타임아웃)
    - 상태 변경: Failed
    - `downloadFailed` 시그널 emit
    - 부분 파일 삭제 (QFile::remove)
- [ ] 에러 메시지 매핑 로직
  - DownloadInterrupted → "네트워크 연결이 끊겼습니다"
  - DownloadCancelled → "사용자가 다운로드를 취소했습니다"
  - 기타 → "알 수 없는 에러가 발생했습니다"

**예상 산출물**: `src/services/DownloadManager.cpp` (완료/에러 로직)

**완료 기준**:
- 다운로드 완료 시 파일 존재 및 크기 검증 성공
- 네트워크 끊김 시 Failed 상태 전환 및 에러 메시지 표시
- 부분 파일 자동 삭제 확인

---

#### Task 1.6: 제어 API 구현 (일시정지/재개/취소)
- [ ] `pauseDownload()` 구현
  - m_activeDownloads에서 QWebEngineDownloadItem 조회
  - `pause()` 메서드 호출 (Qt 5.10+ 확인)
  - 미지원 시 qWarning 출력
- [ ] `resumeDownload()` 구현
  - `resume()` 메서드 호출
- [ ] `cancelDownload()` 구현
  - `cancel()` 메서드 호출
  - 상태 변경: Cancelled
  - m_activeDownloads에서 제거
- [ ] `removeDownload()` 구현
  - 완료/취소/실패 항목만 제거 (진행 중 불가)
  - m_downloads에서 제거
- [ ] `getDownloads()` 구현 (m_downloads 반환)
- [ ] `getDownloadById()` 구현 (id로 검색)

**예상 산출물**: `src/services/DownloadManager.cpp` (제어 API)

**완료 기준**:
- 일시 정지/재개 동작 확인 (Qt 5.15 지원 여부 확인)
- 취소 시 부분 파일 삭제 확인
- 진행 중 항목 제거 시도 시 에러 처리

---

#### Task 1.7: CMakeLists.txt 업데이트
- [ ] `src/services/DownloadManager.cpp` 추가
- [ ] Qt WebEngine 모듈 링크 확인 (`Qt5::WebEngine`)
- [ ] 빌드 테스트

**예상 산출물**: `CMakeLists.txt` (수정)

**완료 기준**:
- 빌드 성공
- DownloadManager 클래스 링크 에러 없음

---

### Phase 2: WebView 통합 (다운로드 이벤트 감지)

**담당**: cpp-dev
**예상 소요 시간**: 3~4시간
**의존성**: Phase 1 완료 필요

#### Task 2.1: WebView에 DownloadManager 연결
- [ ] `src/browser/WebView.h` 수정
  - `setupDownloadHandler(DownloadManager*)` 메서드 선언
- [ ] `src/browser/WebView.cpp` 수정
  - `setupDownloadHandler()` 구현
  - QWebEngineProfile 가져오기 (`page()->profile()`)
  - `downloadRequested` 시그널 연결
  - 람다 핸들러에서 `DownloadManager::startDownload()` 호출
- [ ] `src/browser/WebViewPrivate` (있는 경우) 수정
  - profile 멤버 추가
  - downloadRequested 연결 로직

**예상 산출물**: `src/browser/WebView.cpp` (수정)

**완료 기준**:
- 웹 페이지에서 다운로드 링크 클릭 시 `downloadRequested` 시그널 수신
- DownloadManager::startDownload() 호출 확인
- 파일 다운로드 시작 확인

---

### Phase 3: DownloadPanel UI 구현 (프론트엔드)

**담당**: cpp-dev
**예상 소요 시간**: 10~12시간
**의존성**: Phase 1 완료 필요 (DownloadManager API 사용)

#### Task 3.1: DownloadPanel 골격 구현
- [ ] `src/ui/DownloadPanel.h` 생성
- [ ] `DownloadPanel` 클래스 정의 (QWidget 상속)
- [ ] 생성자 (DownloadManager* 파라미터)
- [ ] 멤버 변수 선언
  - `DownloadManager* m_downloadManager`
  - UI 컴포넌트 (QVBoxLayout, QListWidget, QPushButton 등)
- [ ] 공개 메서드 선언
  - `show()`, `hide()`, `refreshDownloads()`
- [ ] 시그널 선언
  - `panelClosed()`
- [ ] 슬롯 선언
  - 버튼 클릭 핸들러 (onPauseClicked, onResumeClicked 등)
  - DownloadManager 시그널 핸들러 (onDownloadAdded 등)

**예상 산출물**: `src/ui/DownloadPanel.h`

**완료 기준**:
- 헤더 파일 컴파일 성공
- `Q_OBJECT` 매크로 포함

---

#### Task 3.2: UI 레이아웃 구현
- [ ] `src/ui/DownloadPanel.cpp` 생성
- [ ] `setupUI()` 구현
  - 메인 레이아웃 (QVBoxLayout)
  - 헤더 레이아웃 (제목 + 닫기 버튼)
  - 다운로드 리스트 (QListWidget)
    - `setUniformItemSizes(true)` (렌더링 최적화)
    - `setLayoutMode(QListView::Batched)` (스크롤 성능)
  - 액션 레이아웃 (제어 버튼)
    - 선택된 항목용: 일시정지/재개/취소/열기/삭제
    - 전체 제어: 모두 일시정지/모두 재개/완료된 항목 삭제
  - 스타일시트 적용 (다크 테마, 폰트 크기 16px+)
- [ ] `show()`, `hide()` 구현
  - 슬라이드 애니메이션 (QPropertyAnimation, 선택 사항)

**예상 산출물**: `src/ui/DownloadPanel.cpp` (UI 레이아웃)

**완료 기준**:
- 패널 표시/숨김 동작 확인
- 버튼 배치 및 가독성 확인 (대화면 최적화)
- 리모컨 포커스 이동 가능 (TabOrder 설정)

---

#### Task 3.3: 다운로드 항목 위젯 구현 (DownloadItemWidget)
- [ ] `DownloadItemWidget` 클래스 정의 (QWidget 상속)
- [ ] `setupUI()` 구현
  - 파일명 레이블
  - 프로그레스바 (QProgressBar, 높이 20px+)
  - 진행률 레이블 (12.5 MB / 20 MB)
  - 속도 레이블 (2.3 MB/s)
  - ETA 레이블 (3분 20초 남음)
  - 상태 레이블 (진행중/완료/에러)
- [ ] `updateProgress()` 구현
  - 프로그레스바 업데이트 (0~100)
  - 진행률 텍스트 업데이트 (formatFileSize 사용)
- [ ] `updateState()` 구현
  - 상태 레이블 업데이트
  - 에러 시 빨간색 표시
- [ ] `updateItem()` 구현 (전체 항목 재렌더링)
- [ ] 헬퍼 메서드 구현
  - `formatFileSize()` (bytes → MB/GB)
  - `formatSpeed()` (bytes/sec → KB/s, MB/s)
  - `formatTimeLeft()` (seconds → "3분 20초")

**예상 산출물**: `src/ui/DownloadPanel.cpp` (DownloadItemWidget)

**완료 기준**:
- 다운로드 항목이 리스트에 정상 표시
- 프로그레스바 애니메이션 확인
- 속도, ETA 텍스트 포맷 확인

---

#### Task 3.4: DownloadManager 시그널 연결 및 UI 업데이트
- [ ] `setupConnections()` 구현
  - DownloadManager 시그널 연결
    - `downloadAdded` → `onDownloadAdded()`
    - `downloadProgressChanged` → `onDownloadProgressChanged()`
    - `downloadStateChanged` → `onDownloadStateChanged()`
    - `downloadCompleted` → `onDownloadCompleted()`
    - `downloadFailed` → `onDownloadFailed()`
  - 버튼 시그널 연결
    - 일시정지/재개/취소/열기/삭제 버튼 → 각 슬롯
    - 닫기 버튼 → `hide()` + `panelClosed` emit
  - 리스트 선택 변경 연결
    - `currentItemChanged` → `onSelectionChanged()`
- [ ] `onDownloadAdded()` 구현
  - 새 DownloadItemWidget 생성
  - QListWidgetItem 생성 및 추가
  - setItemWidget()로 커스텀 위젯 설정
  - 항목 ID를 UserRole로 저장 (찾기 위해)
- [ ] `onDownloadProgressChanged()` 구현
  - ID로 항목 찾기
  - DownloadItemWidget::updateProgress() 호출
- [ ] `onDownloadStateChanged()` 구현
  - DownloadItemWidget::updateState() 호출
  - 버튼 활성화 상태 업데이트 (`updateButtonStates()`)
- [ ] `onDownloadCompleted()` 구현
  - 상태 업데이트
  - 토스트 메시지 표시 (선택 사항)
- [ ] `onDownloadFailed()` 구현
  - 에러 메시지 표시 (상태 레이블 빨간색)
- [ ] `refreshDownloads()` 구현
  - DownloadManager::getDownloads() 호출
  - 리스트 클리어 후 재렌더링

**예상 산출물**: `src/ui/DownloadPanel.cpp` (시그널 처리)

**완료 기준**:
- 다운로드 시작 시 패널 자동 갱신
- 진행률 실시간 업데이트 확인
- 완료 시 상태 변경 확인

---

#### Task 3.5: 제어 버튼 동작 구현
- [ ] `onPauseClicked()` 구현
  - 선택된 다운로드 ID 가져오기 (`getSelectedDownloadId()`)
  - `DownloadManager::pauseDownload()` 호출
- [ ] `onResumeClicked()` 구현
  - `DownloadManager::resumeDownload()` 호출
- [ ] `onCancelClicked()` 구현
  - `DownloadManager::cancelDownload()` 호출
- [ ] `onOpenClicked()` 구현
  - 완료된 항목만 가능
  - 파일 열기 로직 (`openDownloadedFile()`)
    - LS2 API 시도 (webOS Application Manager)
    - 실패 시 QDesktopServices::openUrl() 폴백
- [ ] `onDeleteClicked()` 구현
  - `DownloadManager::removeDownload()` 호출
  - 리스트에서 항목 제거
- [ ] `onPauseAllClicked()` 구현
  - 진행 중 다운로드 모두 일시정지
- [ ] `onResumeAllClicked()` 구현
  - 일시정지된 다운로드 모두 재개
- [ ] `onClearCompletedClicked()` 구현
  - 완료/취소/실패 항목 모두 제거
- [ ] `getSelectedDownloadId()` 구현
  - 현재 선택된 QListWidgetItem의 UserRole에서 ID 추출
- [ ] `updateButtonStates()` 구현
  - 선택된 항목 상태에 따라 버튼 활성화/비활성화
  - 진행 중: 일시정지/취소 활성화
  - 일시정지: 재개/취소 활성화
  - 완료: 열기/삭제 활성화
  - 에러: 삭제 활성화

**예상 산출물**: `src/ui/DownloadPanel.cpp` (제어 로직)

**완료 기준**:
- 일시정지/재개/취소 버튼 동작 확인
- 완료된 파일 열기 확인 (QDesktopServices 사용)
- 버튼 활성화 상태 동적 변경 확인

---

#### Task 3.6: 리모컨 입력 처리
- [ ] `keyPressEvent()` 오버라이드
- [ ] 십자키 위/아래 처리
  - 다운로드 항목 간 이동 (QListWidget::setCurrentRow)
- [ ] 십자키 좌/우 처리
  - 버튼 간 이동 (TabOrder 또는 수동 포커스 이동)
- [ ] Enter 키 처리
  - 완료된 항목: 파일 열기
  - 버튼 포커스 시: 버튼 클릭
- [ ] Back (Escape) 키 처리
  - 패널 닫기 (`hide()` + `panelClosed` emit)

**예상 산출물**: `src/ui/DownloadPanel.cpp` (키 이벤트)

**완료 기준**:
- 리모컨 십자키로 항목 이동 확인
- Enter 키로 파일 열기/버튼 실행 확인
- Back 버튼으로 패널 닫기 확인

---

#### Task 3.7: CMakeLists.txt 업데이트
- [ ] `src/ui/DownloadPanel.cpp` 추가
- [ ] Qt Widgets 모듈 링크 확인 (`Qt5::Widgets`)
- [ ] 빌드 테스트

**예상 산출물**: `CMakeLists.txt` (수정)

**완료 기준**:
- 빌드 성공
- DownloadPanel 클래스 링크 에러 없음

---

### Phase 4: BrowserWindow 통합 (패널 표시 및 알림)

**담당**: cpp-dev
**예상 소요 시간**: 3~4시간
**의존성**: Phase 1, Phase 3 완료 필요

#### Task 4.1: BrowserWindow에 DownloadManager/Panel 통합
- [ ] `src/browser/BrowserWindow.h` 수정
  - `DownloadManager*`, `DownloadPanel*` 멤버 추가
  - `showDownloadPanel()`, `hideDownloadPanel()`, `toggleDownloadPanel()` 선언
  - `onDownloadCompleted(const DownloadItem&)` 슬롯 선언
- [ ] `src/browser/BrowserWindow.cpp` 수정
  - 생성자에서 DownloadManager 생성
  - DownloadPanel 생성 (DownloadManager 전달)
  - 패널 초기 위치 설정 (하단 슬라이드업, 초기 숨김)
  - WebView::setupDownloadHandler() 호출
  - DownloadManager::downloadCompleted 시그널 연결
- [ ] `showDownloadPanel()` 구현
  - 패널 표시 (show() 또는 애니메이션)
- [ ] `hideDownloadPanel()` 구현
  - 패널 숨김
- [ ] `toggleDownloadPanel()` 구현
  - 패널 표시 상태에 따라 show/hide 토글

**예상 산출물**: `src/browser/BrowserWindow.cpp` (수정)

**완료 기준**:
- BrowserWindow 생성 시 DownloadManager/Panel 초기화 확인
- 패널 표시/숨김 메서드 동작 확인
- WebView에서 다운로드 이벤트 수신 확인

---

#### Task 4.2: 다운로드 완료 알림 구현
- [ ] `onDownloadCompleted()` 구현
  - webOS 토스트 알림 시도 (LS2 API)
    - `luna://com.webos.notification/createToast` 호출
    - 메시지: "다운로드 완료: {fileName}"
  - 실패 시 Qt 자체 토스트 구현 (`showToast()`)
    - QLabel 기반 토스트 위젯
    - 3초 후 자동 삭제 (QTimer::singleShot)
- [ ] `showToast()` 구현 (Qt 대안)
  - QLabel 생성 (반투명 배경, 흰색 텍스트)
  - 하단 중앙 배치
  - 3초 후 deleteLater()

**예상 산출물**: `src/browser/BrowserWindow.cpp` (알림 로직)

**완료 기준**:
- 다운로드 완료 시 토스트 알림 표시 확인
- LS2 API 실패 시 Qt 토스트 폴백 확인
- 토스트 3초 후 자동 삭제 확인

---

### Phase 5: 테스트 (통합 테스트 + 디바이스 테스트)

**담당**: test-runner
**예상 소요 시간**: 4~6시간
**의존성**: Phase 1~4 완료 필요

#### Task 5.1: 통합 테스트 작성 및 실행
- [ ] PDF 다운로드 테스트
  - 테스트 URL: https://www.w3.org/WAI/ER/tests/xhtml/testfiles/resources/pdf/dummy.pdf
  - 검증: 파일 존재, 크기 일치, downloadCompleted 시그널
- [ ] 이미지 다운로드 테스트
  - 테스트 URL: 작은 PNG 이미지
  - 검증: 파일 존재, 프로그레스바 100%
- [ ] 대용량 파일 테스트
  - 테스트 URL: 100MB+ 파일
  - 검증: 진행률 업데이트 (0% → 100%), 속도/ETA 표시
- [ ] 파일명 중복 테스트
  - 동일 파일 2회 다운로드
  - 검증: file (1).pdf 생성 확인
- [ ] 다운로드 일시정지/재개 테스트
  - 검증: 상태 변경 (InProgress → Paused → InProgress)
- [ ] 다운로드 취소 테스트
  - 검증: 상태 변경 (Cancelled), 부분 파일 삭제
- [ ] 네트워크 끊김 에러 테스트
  - Wi-Fi 끄기 시뮬레이션
  - 검증: 상태 Failed, 에러 메시지 표시
- [ ] 리모컨 입력 테스트
  - 십자키 이동, Enter 키 실행 확인

**예상 산출물**: 테스트 리포트 (docs/specs/download-manager/test-report.md)

**완료 기준**:
- 모든 통합 테스트 통과
- 에러 케이스 모두 정상 처리 확인
- 테스트 리포트 작성 완료

---

#### Task 5.2: 실제 디바이스 테스트 (LG 프로젝터 HU715QW)
- [ ] webOS 디바이스에 IPK 배포
  - `ares-package`, `ares-install` 실행
- [ ] QStandardPaths::DownloadLocation 경로 확인
  - 실제 디바이스에서 경로 로그 출력
- [ ] webOS LS2 API 동작 확인
  - 토스트 알림 표시 확인
  - 파일 열기 동작 확인
- [ ] 리모컨 단축키 확인 (F-13 연계)
  - 컬러 버튼으로 패널 토글
- [ ] 대화면 UI 가독성 확인
  - 프로그레스바 두께, 폰트 크기 확인
  - 리모컨 포커스 가시성 확인
- [ ] 다운로드 중 브라우징 성능 확인
  - 멀티태스킹 (다운로드 + 웹 페이지 스크롤)

**예상 산출물**: 디바이스 테스트 리포트 (test-report.md 업데이트)

**완료 기준**:
- 실제 디바이스에서 모든 기능 정상 동작
- LS2 API 호환성 확인 (토스트, 파일 열기)
- 리모컨 조작 확인

---

### Phase 6: 코드 및 문서 리뷰

**담당**: code-reviewer
**예상 소요 시간**: 2~3시간
**의존성**: Phase 1~5 완료 필요

#### Task 6.1: 코드 리뷰
- [ ] DownloadManager 클래스 리뷰
  - 메모리 누수 확인 (QWebEngineDownloadItem 관리)
  - 시그널/슬롯 연결 확인
  - 에러 처리 완전성 확인
- [ ] DownloadPanel 클래스 리뷰
  - UI 렌더링 성능 확인
  - 리모컨 포커스 경로 확인
  - 메모리 최적화 확인 (항목 재사용)
- [ ] BrowserWindow 통합 코드 리뷰
  - 패널 표시/숨김 로직 확인
  - 알림 처리 확인
- [ ] 코딩 컨벤션 확인
  - 주석 언어: 한국어
  - 변수명: camelCase
  - 들여쓰기: 스페이스 4칸
  - 스마트 포인터 사용 확인
  - 네임스페이스: `webosbrowser`

**완료 기준**:
- 코딩 컨벤션 100% 준수
- 메모리 누수 없음 (Valgrind 또는 Qt Creator Analyzer)
- 에러 처리 누락 없음

---

#### Task 6.2: 문서 리뷰
- [ ] DownloadManager 클래스 주석 확인
  - Doxygen 주석 완성도 80% 이상
  - 파라미터/반환값/시그널 설명 완전성
- [ ] DownloadPanel 클래스 주석 확인
- [ ] design.md 업데이트 확인
  - 실제 구현과 설계서 일치 확인
  - 변경 사항 기록
- [ ] API 문서 확인
  - 공개 API 사용 예시 추가

**완료 기준**:
- Doxygen 주석 완성도 80% 이상
- 설계서와 실제 구현 불일치 없음
- API 사용 예시 작성 완료

---

### Phase 7: 진행 로그 및 CHANGELOG 업데이트

**담당**: doc-writer
**예상 소요 시간**: 1~2시간
**의존성**: Phase 6 완료 필요

#### Task 7.1: dev-log.md 업데이트
- [ ] F-12 다운로드 관리 개발 로그 작성
  - Phase별 작업 내용 요약
  - 리스크 및 대응 방안
  - 실제 소요 시간 vs 예상 시간

**예상 산출물**: `docs/dev-log.md` (업데이트)

---

#### Task 7.2: CHANGELOG.md 업데이트
- [ ] F-12 다운로드 관리 변경 사항 추가
  - 새로운 기능 목록
  - API 변경 사항
  - 버그 수정 내역

**예상 산출물**: `CHANGELOG.md` (업데이트)

---

## 3. 태스크 의존성

```
Phase 1: DownloadManager 서비스 구현
    ├── Task 1.1: 구조체/열거형 정의
    ├── Task 1.2: 클래스 골격
    ├── Task 1.3: 다운로드 시작 로직
    ├── Task 1.4: 진행률 추적
    ├── Task 1.5: 완료/에러 처리
    ├── Task 1.6: 제어 API
    └── Task 1.7: CMakeLists.txt
           ↓
Phase 2: WebView 통합
    └── Task 2.1: WebView에 DownloadManager 연결
           ↓
Phase 3: DownloadPanel UI 구현 (Phase 1 완료 후 시작 가능)
    ├── Task 3.1: 골격 구현
    ├── Task 3.2: UI 레이아웃
    ├── Task 3.3: 다운로드 항목 위젯
    ├── Task 3.4: 시그널 연결
    ├── Task 3.5: 제어 버튼 동작
    ├── Task 3.6: 리모컨 입력
    └── Task 3.7: CMakeLists.txt
           ↓
Phase 4: BrowserWindow 통합 (Phase 1, 3 완료 후)
    ├── Task 4.1: DownloadManager/Panel 통합
    └── Task 4.2: 완료 알림
           ↓
Phase 5: 테스트 (Phase 1~4 완료 후)
    ├── Task 5.1: 통합 테스트
    └── Task 5.2: 디바이스 테스트
           ↓
Phase 6: 코드 및 문서 리뷰
    ├── Task 6.1: 코드 리뷰
    └── Task 6.2: 문서 리뷰
           ↓
Phase 7: 진행 로그 및 CHANGELOG
    ├── Task 7.1: dev-log.md
    └── Task 7.2: CHANGELOG.md
```

---

## 4. 병렬 실행 판단

### 4.1 Phase 내 병렬 가능성

#### Phase 1 (DownloadManager 서비스)
- **병렬 불가**: 순차적 의존성 강함
  - Task 1.1 (구조체 정의) → Task 1.2 (클래스 골격) → Task 1.3~1.6 (구현)
  - Task 1.4~1.6은 부분 병렬 가능 (진행률 추적 vs 제어 API)

#### Phase 3 (DownloadPanel UI)
- **부분 병렬 가능**:
  - Task 3.1~3.2 (골격 + 레이아웃) 완료 후:
    - **병렬 A**: Task 3.3 (다운로드 항목 위젯) + Task 3.5 (제어 버튼)
    - **병렬 B**: Task 3.4 (시그널 연결) + Task 3.6 (리모컨 입력)
  - 의존성: Task 3.4가 3.3, 3.5보다 먼저 완료되어야 시그널 연결 가능

### 4.2 Phase 간 병렬 가능성

#### Phase 2 (WebView 통합) vs Phase 3 (DownloadPanel UI)
- **병렬 가능**: 완전히 독립적
  - Phase 2: DownloadManager API 사용 (Phase 1 완료 필요)
  - Phase 3: DownloadManager API 사용 (Phase 1 완료 필요)
  - 두 Phase 모두 Phase 1 완료 후 병렬 시작 가능

#### Phase 4 (BrowserWindow 통합)
- **병렬 불가**: Phase 2, 3 모두 완료 필요
  - BrowserWindow가 DownloadManager + DownloadPanel 모두 사용

### 4.3 PG-3 병렬 배치 컨텍스트

**PG-3 (Phase Group 3)**: F-12 (다운로드 관리) + F-13 (리모컨 단축키) + F-14 (HTTPS 보안 표시)

#### 충돌 분석
| 기능 | 주요 컴포넌트 | 충돌 여부 |
|------|--------------|----------|
| F-12 (다운로드 관리) | DownloadManager, DownloadPanel, BrowserWindow | - |
| F-13 (리모컨 단축키) | KeyboardHandler, BrowserWindow | **부분 충돌** (BrowserWindow 수정) |
| F-14 (HTTPS 보안 표시) | SecurityIndicator, WebView | **부분 충돌** (WebView 수정) |

#### 충돌 상세
1. **BrowserWindow 충돌** (F-12 vs F-13):
   - F-12: `showDownloadPanel()` 메서드 추가
   - F-13: `onShortcutPressed()` 메서드 추가, 단축키 핸들러 연결
   - **충돌 가능성**: 낮음 (독립적인 메서드 추가)
   - **대응**: F-12 완료 후 F-13 시작 권장 (순차)

2. **WebView 충돌** (F-12 vs F-14):
   - F-12: `setupDownloadHandler()` 추가 (QWebEngineProfile::downloadRequested)
   - F-14: `setupSecurityHandler()` 추가 (QWebEngineView::urlChanged)
   - **충돌 가능성**: 낮음 (독립적인 시그널 연결)
   - **대응**: 병렬 가능 (merge 충돌 최소)

#### 병렬 실행 권장 여부
- **F-12 vs F-13**: **순차 권장**
  - 이유: BrowserWindow 수정 겹침, 리모컨 단축키가 다운로드 패널 토글 기능 포함 (연계 필요)
- **F-12 vs F-14**: **병렬 가능**
  - 이유: WebView 수정 독립적 (다른 시그널, 다른 핸들러)
  - 주의: Phase 2 (WebView 통합) 완료 후 F-14 Phase 2 병렬 진행 가능

#### Agent Team 사용 권장 여부
- **권장하지 않음** (F-12 단독 개발)
  - 이유:
    1. **순차적 의존성 강함**: Phase 1 → Phase 2/3 → Phase 4
    2. **Phase 내 병렬 이점 작음**: Task 간 의존성으로 병렬 이점 제한적
    3. **통합 복잡도 높음**: DownloadManager ↔ DownloadPanel 시그널/슬롯 연결 복잡
    4. **단일 개발자 집중력 필요**: UI 레이아웃 + 리모컨 입력 처리 세밀 조정 필요

- **Agent Team 사용 시나리오** (선택 사항):
  - Phase 1 완료 후, Phase 2(WebView) + Phase 3(DownloadPanel) 병렬 배치
  - Agent 1: Phase 2 (WebView 통합) - 3~4시간
  - Agent 2: Phase 3 (DownloadPanel UI) - 10~12시간
  - **단점**: Phase 3가 Phase 2보다 훨씬 길어 Agent 1 유휴 시간 발생

---

## 5. 리스크 및 대응 방안

### 5.1 기술적 리스크

| 리스크 | 확률 | 영향도 | 대응 방안 |
|--------|------|--------|-----------|
| Qt 5.15의 pause/resume 미지원 | 낮음 | 중 | 런타임에 메서드 존재 확인 (`QMetaObject::invokeMethod`), 미지원 시 UI에서 버튼 비활성화, Qt 6 업그레이드 검토 |
| webOS 파일 시스템 권한 제약 | 중 | 고 | 기본 경로 실패 시 폴백 경로 자동 사용 (AppDataLocation), 쓰기 권한 사전 확인 (QFileInfo::isWritable) |
| 대용량 파일(1GB+) 다운로드 시 메모리 과부하 | 중 | 중 | 동시 다운로드 최대 3개 제한, 진행률 업데이트 쓰로틀링 (500ms), 메모리 프로파일링 |
| 네트워크 불안정 시 다운로드 빈번한 실패 | 고 | 중 | 에러 처리 강화 (명확한 에러 메시지), 재시도 버튼 제공 (선택 사항), 일시정지/재개 기능 활용 |
| webOS LS2 API (토스트, 파일 열기) 제약 | 중 | 중 | Qt 대안 구현 (QLabel 토스트, QDesktopServices), LS2 API 실패 시 자동 폴백 |
| 리모컨 포커스 관리 복잡도 | 중 | 중 | TabOrder 세밀 조정, keyPressEvent 커스터마이징, 실제 디바이스 사용자 테스트 |
| 다운로드 중 앱 크래시 시 부분 파일 잔존 | 낮음 | 낮음 | 앱 시작 시 임시 파일 정리 로직 추가 (QDir::entryList로 .crdownload 파일 삭제) |

### 5.2 일정 리스크

| 리스크 | 확률 | 영향도 | 대응 방안 |
|--------|------|--------|-----------|
| Phase 3 (DownloadPanel UI) 예상 시간 초과 | 중 | 중 | 리모컨 입력 처리 복잡도 낮춤 (기본 TabOrder 사용), 슬라이드 애니메이션 생략, 핵심 기능 우선 구현 |
| Phase 5 (디바이스 테스트) 시간 부족 | 중 | 고 | 통합 테스트 (Phase 5.1)로 사전 검증 강화, 디바이스 테스트는 핵심 시나리오만 선별 실행 |
| Qt WebEngine API 학습 시간 필요 | 중 | 중 | Qt 공식 문서 및 예제 코드 사전 조사 (QWebEngineDownloadItem 예제), StackOverflow 참고 |

### 5.3 통합 리스크

| 리스크 | 확률 | 영향도 | 대응 방안 |
|--------|------|--------|-----------|
| F-13 (리모컨 단축키)와 충돌 | 중 | 중 | F-12 완료 후 F-13 시작 권장, BrowserWindow 수정 사항 명확히 문서화, merge 전 코드 리뷰 필수 |
| F-11 (설정 화면)과 연계 미흡 | 낮음 | 낮음 | DownloadManager::setDownloadPath() API 공개, F-11에서 호출 가능하도록 설계 |
| F-08 (히스토리)과 다운로드 기록 중복 | 낮음 | 낮음 | F-12는 다운로드 기록 저장 안 함 (선택 사항으로 남김), F-08의 StorageService 재사용 검토 |

---

## 6. 예상 소요 시간 총계

| Phase | 예상 시간 | 담당 |
|-------|----------|------|
| Phase 1: DownloadManager 서비스 | 8~10시간 | cpp-dev |
| Phase 2: WebView 통합 | 3~4시간 | cpp-dev |
| Phase 3: DownloadPanel UI | 10~12시간 | cpp-dev |
| Phase 4: BrowserWindow 통합 | 3~4시간 | cpp-dev |
| Phase 5: 테스트 | 4~6시간 | test-runner |
| Phase 6: 코드 및 문서 리뷰 | 2~3시간 | code-reviewer |
| Phase 7: 진행 로그 및 CHANGELOG | 1~2시간 | doc-writer |
| **총계** | **31~41시간** (약 4~5일) | - |

**예상 기간**: 4~5일 (1일 8시간 기준)

**주의사항**:
- 실제 디바이스 테스트 시간 포함 (LG 프로젝터 HU715QW)
- Qt WebEngine API 학습 시간 포함
- 리모컨 입력 처리 및 UI 조정 시간 포함

---

## 7. 완료 기준 (Definition of Done)

### 7.1 기능 완료 기준
- [ ] 웹 페이지에서 다운로드 링크 클릭 시 자동 다운로드 시작
- [ ] 다운로드 진행률 실시간 표시 (프로그레스바, 속도, ETA)
- [ ] 다운로드 일시정지/재개 동작 (Qt 5.15 지원 시)
- [ ] 다운로드 취소 시 부분 파일 삭제 확인
- [ ] 다운로드 완료 시 토스트 알림 표시
- [ ] 완료된 파일 열기 기능 (QDesktopServices 또는 LS2 API)
- [ ] 다운로드 패널 UI 표시/숨김 동작
- [ ] 리모컨 십자키로 다운로드 항목 이동 가능
- [ ] 네트워크 에러, 디스크 에러 명확히 처리 및 표시
- [ ] 파일명 중복 시 자동 번호 추가 (file (1).pdf)

### 7.2 코드 품질 기준
- [ ] 코딩 컨벤션 100% 준수 (CLAUDE.md 기준)
- [ ] 메모리 누수 없음 (Valgrind 또는 Qt Creator Analyzer 확인)
- [ ] Doxygen 주석 완성도 80% 이상
- [ ] 에러 처리 누락 없음 (모든 시나리오 처리)
- [ ] 스마트 포인터 사용 (raw 포인터 최소화)
- [ ] 네임스페이스 `webosbrowser` 사용

### 7.3 테스트 완료 기준
- [ ] 통합 테스트 모두 통과 (PDF, 이미지, 대용량 파일)
- [ ] 에러 시나리오 테스트 통과 (네트워크 끊김, 디스크 부족)
- [ ] 리모컨 입력 테스트 통과 (십자키, Enter, Back)
- [ ] 실제 디바이스 테스트 완료 (LG 프로젝터 HU715QW)
- [ ] 테스트 리포트 작성 완료

### 7.4 문서 완료 기준
- [ ] DownloadManager API 문서 작성 (Doxygen)
- [ ] DownloadPanel 클래스 주석 작성
- [ ] design.md와 실제 구현 일치 확인
- [ ] dev-log.md 업데이트
- [ ] CHANGELOG.md 업데이트

### 7.5 배포 준비 기준
- [ ] IPK 패키징 성공 (`ares-package`)
- [ ] webOS 디바이스 설치 성공 (`ares-install`)
- [ ] 실제 디바이스에서 모든 기능 정상 동작
- [ ] 성능 기준 충족 (진행률 업데이트 500ms, 동시 다운로드 3개)

---

## 8. 다음 단계 (F-12 완료 후)

### 8.1 F-13 (리모컨 단축키) 연계
- BrowserWindow에 단축키 핸들러 추가
- 컬러 버튼 (Blue) → `toggleDownloadPanel()` 매핑
- KeyboardHandler 클래스 연결

### 8.2 F-11 (설정 화면) 연계
- 설정 화면에 "다운로드 폴더" 항목 추가
- `DownloadManager::setDownloadPath()` 호출
- "자동 다운로드 시작" on/off 설정 (선택 사항)

### 8.3 F-08 (히스토리) 연계 (선택 사항)
- 다운로드 기록을 LS2 API로 영구 저장
- StorageService 재사용 검토
- 히스토리 화면에 "다운로드" 탭 추가

---

## 변경 이력

| 날짜 | 변경 내용 | 작성자 |
|------|-----------|--------|
| 2026-02-14 | 초기 구현 계획서 작성 | product-manager |
