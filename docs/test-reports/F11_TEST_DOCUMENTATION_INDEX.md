# F-11 (설정 화면) - 테스트 문서 색인

**작성일**: 2026-02-14
**문서 상태**: ✅ APPROVED FOR TESTING
**총 문서**: 5개 (총 93KB)

---

## 문서 개요

F-11 (설정 화면) 기능의 종합적인 검증 및 테스트를 위해 다음 문서들이 작성되었습니다.

| # | 문서명 | 파일명 | 크기 | 대상 | 목적 |
|---|--------|--------|------|------|------|
| 1 | 종합 검증 보고서 | F11_VALIDATION_REPORT.md | 46KB | QA, Developer | 상세 검증 결과 |
| 2 | 검증 요약본 | F11_VALIDATION_SUMMARY.txt | 10KB | 경영진, 팀장 | 빠른 현황 파악 |
| 3 | 테스트 시나리오 | F11_TEST_SCENARIOS.md | 25KB | QA 엔지니어 | 테스트 케이스 작성 |
| 4 | 테스트 전 체크리스트 | F11_PRE_TEST_CHECKLIST.md | 8.6KB | Developer | 구현 완료 확인 |
| 5 | 문서 색인 | F11_TEST_DOCUMENTATION_INDEX.md | 이 파일 | 모두 | 문서 네비게이션 |

---

## 각 문서의 사용 시기 및 방법

### 1. 종합 검증 보고서 (F11_VALIDATION_REPORT.md)

**용도**: 정적 코드 검증, 로직 검증, 설계 일치성 검증

**대상**: QA 엔지니어, 개발자, 아키텍처

**읽는 시점**:
- 개발 완료 직후
- 상세한 검증 내용이 필요할 때

**주요 내용**:
- ✅ CMakeLists.txt 검증 (100점)
- ✅ Include 경로 검증 (100점)
- ✅ Qt 의존성 검증 (100점)
- ✅ SettingsService 로직 검증 (10/10점)
- ✅ SettingsPanel 로직 검증 (10/10점)
- ✅ BrowserWindow 통합 검증 (10/10점)
- ✅ 설계서 일치성 검증 (100% 구현)
- ✅ Issue 목록 (Critical 0개, Warning 2개, Info 3개)

**섹션 구조**:
```
1. 검증 요약 (점수, 시각적 요약)
2. 정적 코드 검증 (CMakeLists, Include, Qt)
3. 로직 검증 (SettingsService, SettingsPanel, BrowserWindow)
4. 설계 일치성 검증
5. 테스트 시나리오 (개요만)
6. Issue 목록
7. 최종 체크리스트
8. 결론 및 다음 단계
```

**사용 예시**:
```
상황: "SettingsService의 로직이 설계서와 맞는지 확인하고 싶다"
→ 섹션 3.1 "SettingsService 로직 검증" 참조
→ 8개 함수 각각의 로직이 라인별로 검증되어 있음
→ 통과 기준과 결과 확인 가능
```

---

### 2. 검증 요약본 (F11_VALIDATION_SUMMARY.txt)

**용도**: 빠른 상황 파악, 경영진 보고

**대상**: 프로젝트 매니저, 팀장, 경영진

**읽는 시점**:
- 일일 스탠드업 미팅
- 주간 진행 상황 보고
- 높은 수준의 검증 결과 확인

**주요 내용**:
- 검증 결과 요약표 (Status → Result)
- 점수 분석 (98/100점)
- 배포 준비 상태 ✅
- Issue 요약 (Critical 0개)
- 다음 단계 (7시간)

**읽기 시간**: 5분

**사용 예시**:
```
상황: "오늘 F-11 테스트 진행 가능한가요?"
→ 검증 요약본 확인
→ "검증 완료, APPROVED FOR TESTING"
→ "다음 단계는 단위 테스트 (2시간)"
```

---

### 3. 테스트 시나리오 (F11_TEST_SCENARIOS.md)

**용도**: 테스트 케이스 작성, 테스트 실행

**대상**: QA 엔지니어, 테스트 자동화 팀

**읽는 시점**:
- 테스트 시작 전
- 테스트 케이스 코드 작성할 때
- E2E 테스트 실행 중

**주요 내용**:
- 단위 테스트: 8개 TC (SettingsServiceTest.cpp)
- 통합 테스트: 7개 TC (SettingsPanelIntegrationTest.cpp)
- E2E 테스트: 5개 시나리오 (수동)

**각 TC의 구조**:
```
[테스트 ID]
- 사전 조건
- 단계별 동작
- 예상 결과
- 검증 내용
- 테스트 데이터 (해당 시 사용)
```

**코드 예시**:
```cpp
// TC-1: 기본값 로드
TEST_F(SettingsServiceTest, LoadDefaultSettings) {
    // Given
    SettingsService settings(&storageService);

    // When
    bool result = settings.loadSettings();

    // Then
    ASSERT_TRUE(result);
    EXPECT_EQ(settings.searchEngine(), "google");
}
```

**사용 예시**:
```
상황: "SettingsService의 단위 테스트를 작성해야 한다"
→ TC-1 ~ TC-8 참조
→ 각 TC의 코드 템플릿 복사
→ 테스트 파일 작성 시작
```

---

### 4. 테스트 전 체크리스트 (F11_PRE_TEST_CHECKLIST.md)

**용도**: 구현 완료 확인, 테스트 시작 전 점검

**대상**: 개발자, QA 엔지니어

**읽는 시점**:
- 개발 완료 후 (테스트 시작 전)
- 빌드 에러 발생 시
- 기능 동작 확인 중

**주요 내용**:
- 빌드 환경 확인
- 코드 완성도 확인
- 테마 파일 확인
- CMakeLists.txt 확인
- 빌드 확인
- 테스트 파일 준비
- 코드 품질 확인
- 기능 동작 확인
- 성능 확인
- 에러 처리 확인

**체크리스트 예시**:
```
[ ] src/services/SettingsService.h 존재
[✓] src/services/SettingsService.cpp 존재
[ ] 생성자/소멸자 구현됨
...
```

**사용 예시**:
```
상황: "F-11 개발이 완료되었습니다. 테스트를 시작할 수 있나요?"
→ PRE_TEST_CHECKLIST 모든 항목 확인
→ 10분 내에 완료 여부 판단
→ 문제 발견 시 수정 후 재확인
```

---

## 문서 간 연관성

```
┌─────────────────────────────────────┐
│  F11_VALIDATION_REPORT.md           │
│  (종합 검증 보고서)                  │
│  - 모든 검증 결과 상세 기록           │
└────────┬────────────────────────────┘
         │
         ├─────────────────────────────────┐
         │                                 │
    ┌────▼──────────────────┐    ┌───────▼──────────────┐
    │ SUMMARY (5분 요약)      │    │ TEST_SCENARIOS (구현)│
    │ - 경영진 보고         │    │ - 테스트 작성       │
    └────┬──────────────────┘    └───────┬──────────────┘
         │                               │
         └───────────────┬───────────────┘
                         │
                    ┌────▼──────────────┐
                    │ PRE_TEST_CHECKLIST│
                    │ - 최종 확인        │
                    └───────────────────┘
                         │
                         ▼
                    테스트 시작
```

---

## 빠른 참조 가이드

### "지금 당신이 해야 할 일은?"

#### 1. 개발자
```
현재: "F-11 구현 완료"

1단계: PRE_TEST_CHECKLIST 모든 항목 점검
       → 15분 소요

2단계: 문제 발견 시 수정
       → 10분~1시간 (문제 복잡도에 따라)

3단계: VALIDATION_REPORT 검증 결과 확인
       → 5분 (상세 사항 확인 필요 시 추가)

4단계: 다음 팀에 인계
```

#### 2. QA 엔지니어
```
현재: "F-11 테스트 시작"

1단계: VALIDATION_SUMMARY 읽기
       → 5분 (전체 상황 파악)

2단계: TEST_SCENARIOS 검토
       → 30분 (테스트 계획 수립)

3단계: 테스트 코드 작성
       - TC-1~8: SettingsServiceTest.cpp (2시간)
       - TC-9~15: SettingsPanelIntegrationTest.cpp (2시간)

4단계: E2E 테스트 실행
       - 5개 시나리오 (3시간)
```

#### 3. 프로젝트 매니저
```
현재: "F-11 진행 상황 확인"

1단계: VALIDATION_SUMMARY 읽기
       → 5분 (상태 파악)

2단계: Issue 목록 확인
       → 2분 (위험 요소 확인)

3단계: 다음 단계 및 일정 확인
       → 3분 (리소스 계획)

→ 총 10분
```

---

## 문서 버전 관리

| 버전 | 날짜 | 변경 사항 | 상태 |
|------|------|---------|------|
| 1.0 | 2026-02-14 | 초안 작성 | ✅ APPROVED |
| - | - | - | - |

---

## 다음 단계 타임라인

```
2026-02-14 (오늘)
├─ 10:00 - 11:00: 개발자 PRE_TEST_CHECKLIST 확인 (1시간)
├─ 11:00 - 12:00: 문제 수정 (필요 시)
└─ 12:00 - 13:00: QA 엔지니어에게 인계

2026-02-15 (내일)
├─ 09:00 - 11:00: 단위 테스트 작성 (2시간)
├─ 11:00 - 13:00: 통합 테스트 작성 (2시간)
└─ 14:00 - 17:00: E2E 테스트 실행 (3시간)

2026-02-16 (모레)
├─ 09:00 - 10:00: 테스트 결과 분석 (1시간)
├─ 10:00 - 11:00: 코드 리뷰 (1시간)
└─ 11:00 - 12:00: 최종 승인 (1시간)

2026-02-17 (금요일)
└─ 배포 준비 완료
```

---

## 문서 위치

```
/Users/jsong/dev/jsong1230-github/webosbrowser-native/
├── F11_VALIDATION_REPORT.md           (46KB)
├── F11_VALIDATION_SUMMARY.txt         (10KB)
├── F11_TEST_SCENARIOS.md              (25KB)
├── F11_PRE_TEST_CHECKLIST.md          (8.6KB)
└── F11_TEST_DOCUMENTATION_INDEX.md    (이 파일)
```

**전체 크기**: ~93KB

---

## 자주 묻는 질문 (FAQ)

### Q: "문서를 모두 읽어야 하나요?"
**A**: 역할에 따라 읽을 문서를 선택하세요.
- 개발자: PRE_TEST_CHECKLIST + VALIDATION_REPORT (주요 섹션)
- QA: VALIDATION_SUMMARY + TEST_SCENARIOS
- 매니저: VALIDATION_SUMMARY만

### Q: "어떤 문서부터 읽어야 하나요?"
**A**: VALIDATION_SUMMARY부터 시작하세요. (5분)

### Q: "테스트 코드를 작성하려면?"
**A**: TEST_SCENARIOS.md 의 각 TC 템플릿을 참고하세요.

### Q: "문제가 발견되었습니다. 어디를 봐야 하나요?"
**A**:
1. VALIDATION_REPORT의 "Issue 목록" 확인
2. 해당 Issue의 "대응 방법" 참고
3. 필요시 VALIDATION_REPORT의 로직 검증 섹션 참고

### Q: "최종 결과는?"
**A**: ✅ **APPROVED FOR TESTING** (점수: 98/100)

---

## 지원 및 문의

**검증자**: test-runner (QA Engineer)
**작성일**: 2026-02-14
**문의**: [프로젝트 Slack 채널]

---

**문서 색인 완료**
**모든 문서가 준비되었습니다.**
**테스트를 시작하세요! 🚀**
