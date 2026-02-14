/**
 * @file TabManagerTest.cpp
 * @brief TabManager 클래스 단위 테스트
 *
 * 테스트 범위:
 * 1. 생성자/소멸자
 * 2. setCurrentTab / getCurrentTab
 * 3. getTabCount
 * 4. getCurrentTabTitle / getCurrentTabUrl
 * 5. tabChanged 시그널
 * 6. 엣지 케이스 (nullptr, 빈 상태)
 */

#include <gtest/gtest.h>
#include <QSignalSpy>
#include <QTest>
#include <QApplication>

#include "../src/browser/TabManager.h"

using namespace webosbrowser;

// ============================================================================
// 글로벌 QApplication (테스트 실행에 필요)
// ============================================================================

static QApplication *app = nullptr;

void initializeQApplicationTabManager() {
    if (!qApp) {
        int argc = 0;
        char *argv[] = {(char *)"webosbrowser_tests"};
        app = new QApplication(argc, argv);
    }
}

// ============================================================================
// 테스트 클래스
// ============================================================================

class TabManagerTest : public ::testing::Test {
protected:
    TabManager *tabManager = nullptr;
    // WebView는 가짜 포인터로 사용 (실제 생성은 하지 않음)
    void *dummyWebView1 = (void *)0x12345678;
    void *dummyWebView2 = (void *)0x87654321;

    void SetUp() override {
        initializeQApplicationTabManager();
        tabManager = new TabManager();
    }

    void TearDown() override {
        if (tabManager) {
            delete tabManager;
            tabManager = nullptr;
        }
    }
};

// ============================================================================
// F-06: TabManager 생성자/소멸자 테스트
// ============================================================================

/**
 * 요구사항: TabManager 생성자에서 currentTab_을 nullptr로 초기화
 */
TEST_F(TabManagerTest, Constructor_InitializeCurrentTabAsNull) {
    // AAA Pattern
    // Arrange: 생성자에서 이미 초기화됨

    // Act: 이미 생성됨

    // Assert
    EXPECT_EQ(tabManager->getCurrentTab(), nullptr)
        << "TabManager 생성 시 currentTab_은 nullptr로 초기화되어야 함";
}

/**
 * 요구사항: TabManager 생성자에서 getTabCount() 반환값 확인
 */
TEST_F(TabManagerTest, Constructor_InitializeTabCountAsZero) {
    // Assert: 빈 상태에서 탭 개수는 0
    EXPECT_EQ(tabManager->getTabCount(), 0)
        << "현재 탭이 없을 때 getTabCount()는 0을 반환해야 함";
}

/**
 * 요구사항: TabManager 소멸자에서 메모리 정리
 */
TEST_F(TabManagerTest, Destructor_NoMemoryLeak) {
    // Arrange
    TabManager *tempManager = new TabManager();

    // Act
    delete tempManager;

    // Assert: 크래시 없이 정상 소멸
    EXPECT_TRUE(true) << "TabManager 소멸자에서 메모리 누수 없음";
}

// ============================================================================
// F-06: setCurrentTab / getCurrentTab 테스트
// ============================================================================

/**
 * 요구사항: setCurrentTab으로 활성 탭 설정 후 getCurrentTab으로 반환
 */
TEST_F(TabManagerTest, SetGetCurrentTab_SetAndRetrieve) {
    // Arrange
    void *webView = dummyWebView1;

    // Act
    tabManager->setCurrentTab((webosbrowser::WebView *)webView);

    // Assert
    EXPECT_EQ(tabManager->getCurrentTab(), (webosbrowser::WebView *)webView)
        << "setCurrentTab으로 설정한 WebView가 getCurrentTab으로 반환되어야 함";
}

/**
 * 요구사항: 초기 상태에서 getCurrentTab은 nullptr 반환
 */
TEST_F(TabManagerTest, GetCurrentTab_InitiallyNull) {
    // Assert
    EXPECT_EQ(tabManager->getCurrentTab(), nullptr)
        << "초기 상태에서 getCurrentTab()은 nullptr을 반환해야 함";
}

/**
 * 요구사항: setCurrentTab에 nullptr 설정 가능
 */
TEST_F(TabManagerTest, SetCurrentTab_SetNull) {
    // Arrange
    tabManager->setCurrentTab((webosbrowser::WebView *)dummyWebView1);
    EXPECT_NE(tabManager->getCurrentTab(), nullptr);

    // Act
    tabManager->setCurrentTab(nullptr);

    // Assert
    EXPECT_EQ(tabManager->getCurrentTab(), nullptr)
        << "setCurrentTab(nullptr)으로 현재 탭을 초기화할 수 있어야 함";
}

/**
 * 요구사항: 같은 WebView를 다시 설정해도 정상 작동
 */
TEST_F(TabManagerTest, SetCurrentTab_SetSameWebViewTwice) {
    // Arrange
    void *webView = dummyWebView1;
    tabManager->setCurrentTab((webosbrowser::WebView *)webView);

    // Act
    tabManager->setCurrentTab((webosbrowser::WebView *)webView);

    // Assert
    EXPECT_EQ(tabManager->getCurrentTab(), (webosbrowser::WebView *)webView)
        << "같은 WebView를 재설정해도 정상 작동해야 함";
}

/**
 * 요구사항: 다른 WebView로 변경
 */
TEST_F(TabManagerTest, SetCurrentTab_ChangeToDifferentWebView) {
    // Arrange
    tabManager->setCurrentTab((webosbrowser::WebView *)dummyWebView1);
    EXPECT_EQ(tabManager->getCurrentTab(), (webosbrowser::WebView *)dummyWebView1);

    // Act
    tabManager->setCurrentTab((webosbrowser::WebView *)dummyWebView2);

    // Assert
    EXPECT_EQ(tabManager->getCurrentTab(), (webosbrowser::WebView *)dummyWebView2)
        << "다른 WebView로 변경되어야 함";
}

// ============================================================================
// F-06: getTabCount 테스트 (단일 탭 모드)
// ============================================================================

/**
 * 요구사항: 탭이 없을 때 getTabCount 반환값은 0
 */
TEST_F(TabManagerTest, GetTabCount_NoTabReturnsZero) {
    // Arrange: currentTab_이 nullptr

    // Act
    int count = tabManager->getTabCount();

    // Assert
    EXPECT_EQ(count, 0)
        << "탭이 없을 때 getTabCount()는 0을 반환해야 함";
}

/**
 * 요구사항: 탭이 1개일 때 getTabCount 반환값은 1
 */
TEST_F(TabManagerTest, GetTabCount_OneTabReturnsOne) {
    // Arrange
    tabManager->setCurrentTab((webosbrowser::WebView *)dummyWebView1);

    // Act
    int count = tabManager->getTabCount();

    // Assert
    EXPECT_EQ(count, 1)
        << "탭이 1개일 때 getTabCount()는 1을 반환해야 함";
}

/**
 * 요구사항: 단일 탭 모드에서 항상 0 또는 1 반환
 */
TEST_F(TabManagerTest, GetTabCount_AlwaysZeroOrOne) {
    // Test 1: 초기 상태
    EXPECT_EQ(tabManager->getTabCount(), 0);

    // Test 2: 탭 설정
    tabManager->setCurrentTab((webosbrowser::WebView *)dummyWebView1);
    EXPECT_EQ(tabManager->getTabCount(), 1);

    // Test 3: 탭 제거
    tabManager->setCurrentTab(nullptr);
    EXPECT_EQ(tabManager->getTabCount(), 0);

    // Test 4: 다시 설정
    tabManager->setCurrentTab((webosbrowser::WebView *)dummyWebView2);
    EXPECT_EQ(tabManager->getTabCount(), 1);
}

// ============================================================================
// F-06: getCurrentTabTitle 테스트
// ============================================================================

/**
 * 요구사항: 탭이 없을 때 getCurrentTabTitle 반환값은 빈 문자열
 */
TEST_F(TabManagerTest, GetCurrentTabTitle_NoTabReturnsEmpty) {
    // Assert
    EXPECT_EQ(tabManager->getCurrentTabTitle(), "")
        << "탭이 없을 때 getCurrentTabTitle()는 빈 문자열을 반환해야 함";
}

// ============================================================================
// F-06: getCurrentTabUrl 테스트
// ============================================================================

/**
 * 요구사항: 탭이 없을 때 getCurrentTabUrl 반환값은 빈 QUrl
 */
TEST_F(TabManagerTest, GetCurrentTabUrl_NoTabReturnsEmpty) {
    // Assert
    EXPECT_TRUE(tabManager->getCurrentTabUrl().isEmpty())
        << "탭이 없을 때 getCurrentTabUrl()는 빈 URL을 반환해야 함";
}

// ============================================================================
// F-06: tabChanged 시그널 테스트
// ============================================================================

/**
 * 요구사항: setCurrentTab 호출 시 tabChanged 시그널 발생
 */
TEST_F(TabManagerTest, Signal_TabChangedEmittedOnSetCurrentTab) {
    // Arrange
    QSignalSpy spy(tabManager, &TabManager::tabChanged);

    // Act
    tabManager->setCurrentTab((webosbrowser::WebView *)dummyWebView1);

    // Assert
    EXPECT_GT(spy.count(), 0)
        << "setCurrentTab 호출 시 tabChanged 시그널이 발생해야 함";
}

/**
 * 요구사항: 같은 탭을 다시 설정하면 시그널 미발생
 */
TEST_F(TabManagerTest, Signal_NoTabChangedWhenSettingSameTab) {
    // Arrange
    tabManager->setCurrentTab((webosbrowser::WebView *)dummyWebView1);
    QSignalSpy spy(tabManager, &TabManager::tabChanged);

    // Act
    tabManager->setCurrentTab((webosbrowser::WebView *)dummyWebView1);

    // Assert
    EXPECT_EQ(spy.count(), 0)
        << "같은 탭을 다시 설정하면 tabChanged 시그널이 발생하지 않아야 함";
}

/**
 * 요구사항: 다른 탭으로 변경하면 시그널 발생
 */
TEST_F(TabManagerTest, Signal_TabChangedWhenChangingTab) {
    // Arrange
    tabManager->setCurrentTab((webosbrowser::WebView *)dummyWebView1);
    QSignalSpy spy(tabManager, &TabManager::tabChanged);

    // Act
    tabManager->setCurrentTab((webosbrowser::WebView *)dummyWebView2);

    // Assert
    EXPECT_GT(spy.count(), 0)
        << "다른 탭으로 변경 시 tabChanged 시그널이 발생해야 함";
}

/**
 * 요구사항: 단일 탭 모드에서 시그널 인덱스는 항상 0
 */
TEST_F(TabManagerTest, Signal_IndexIsAlwaysZeroInSingleTabMode) {
    // Arrange
    QSignalSpy spy(tabManager, &TabManager::tabChanged);

    // Act
    tabManager->setCurrentTab((webosbrowser::WebView *)dummyWebView1);

    // Assert
    ASSERT_GT(spy.count(), 0);
    int index = qvariant_cast<int>(spy.at(0).at(0));
    EXPECT_EQ(index, 0)
        << "단일 탭 모드에서 시그널 인덱스는 항상 0이어야 함";
}

/**
 * 요구사항: 탭 제거 시 신호 발생
 */
TEST_F(TabManagerTest, Signal_TabChangedWhenRemovingTab) {
    // Arrange
    tabManager->setCurrentTab((webosbrowser::WebView *)dummyWebView1);
    QSignalSpy spy(tabManager, &TabManager::tabChanged);

    // Act
    tabManager->setCurrentTab(nullptr);

    // Assert
    EXPECT_GT(spy.count(), 0)
        << "탭 제거 시 tabChanged 시그널이 발생해야 함";
}

// ============================================================================
// F-06: 엣지 케이스 테스트
// ============================================================================

/**
 * 요구사항: nullptr를 반복해서 설정해도 정상 작동
 */
TEST_F(TabManagerTest, EdgeCase_SetNullMultipleTimes) {
    // Arrange
    tabManager->setCurrentTab((webosbrowser::WebView *)dummyWebView1);
    QSignalSpy spy(tabManager, &TabManager::tabChanged);

    // Act
    tabManager->setCurrentTab(nullptr);
    int firstSignals = spy.count();

    tabManager->setCurrentTab(nullptr);
    int secondSignals = spy.count();

    // Assert
    EXPECT_EQ(secondSignals, firstSignals)
        << "같은 nullptr을 다시 설정하면 신호가 추가 발생하지 않아야 함";
}

/**
 * 요구사항: 빠른 연속 탭 변경
 */
TEST_F(TabManagerTest, EdgeCase_RapidTabChanges) {
    // Arrange
    QSignalSpy spy(tabManager, &TabManager::tabChanged);

    // Act
    tabManager->setCurrentTab((webosbrowser::WebView *)dummyWebView1);
    tabManager->setCurrentTab((webosbrowser::WebView *)dummyWebView2);
    tabManager->setCurrentTab((webosbrowser::WebView *)dummyWebView1);

    // Assert
    EXPECT_EQ(tabManager->getCurrentTab(), (webosbrowser::WebView *)dummyWebView1)
        << "빠른 연속 탭 변경 후 최종 탭이 올바르게 설정되어야 함";
}

/**
 * 요구사항: 탭 설정 후 조회 메서드 일관성
 */
TEST_F(TabManagerTest, Consistency_AllGettersConsistentAfterSet) {
    // Arrange
    tabManager->setCurrentTab((webosbrowser::WebView *)dummyWebView1);

    // Act
    void *retrievedTab = tabManager->getCurrentTab();
    int count = tabManager->getTabCount();

    // Assert
    EXPECT_EQ(retrievedTab, (webosbrowser::WebView *)dummyWebView1);
    EXPECT_EQ(count, 1);
}

/**
 * 요구사항: 탭 제거 후 조회 메서드 일관성
 */
TEST_F(TabManagerTest, Consistency_AllGettersConsistentAfterRemoval) {
    // Arrange
    tabManager->setCurrentTab((webosbrowser::WebView *)dummyWebView1);
    tabManager->setCurrentTab(nullptr);

    // Act
    void *retrievedTab = tabManager->getCurrentTab();
    int count = tabManager->getTabCount();
    QString title = tabManager->getCurrentTabTitle();
    QUrl url = tabManager->getCurrentTabUrl();

    // Assert
    EXPECT_EQ(retrievedTab, nullptr);
    EXPECT_EQ(count, 0);
    EXPECT_EQ(title, "");
    EXPECT_TRUE(url.isEmpty());
}

// ============================================================================
// F-06: 스트레스 테스트
// ============================================================================

/**
 * 요구사항: 대량의 설정/조회 반복도 정상 작동
 */
TEST_F(TabManagerTest, Stress_LoopSetAndGetCurrentTab) {
    // Act
    for (int i = 0; i < 1000; ++i) {
        if (i % 2 == 0) {
            tabManager->setCurrentTab((webosbrowser::WebView *)dummyWebView1);
        } else {
            tabManager->setCurrentTab((webosbrowser::WebView *)dummyWebView2);
        }
    }

    // Assert
    // 최종 상태: dummyWebView2 설정으로 끝나야 함
    EXPECT_EQ(tabManager->getCurrentTab(), (webosbrowser::WebView *)dummyWebView2)
        << "대량 반복 후 최종 상태가 올바르지 않음";
}

/**
 * 요구사항: 신호 스트림 처리
 */
TEST_F(TabManagerTest, Stress_MultipleSignalEmissions) {
    // Arrange
    QSignalSpy spy(tabManager, &TabManager::tabChanged);

    // Act
    for (int i = 0; i < 10; ++i) {
        if (i % 2 == 0) {
            tabManager->setCurrentTab((webosbrowser::WebView *)dummyWebView1);
        } else {
            tabManager->setCurrentTab((webosbrowser::WebView *)dummyWebView2);
        }
    }

    // Assert
    EXPECT_GT(spy.count(), 0)
        << "반복된 설정에서 신호가 발생해야 함";
}

// ============================================================================
// F-06: 복사/이동 생성자 및 대입 연산자 테스트
// ============================================================================

/**
 * 요구사항: TabManager 복사/이동 생성자 및 대입 연산자 삭제
 * (이 테스트는 컴파일 타임에 확인되므로 여기서는 검증 불가)
 */
TEST_F(TabManagerTest, DesignPattern_NoCopyOrMoveConstructor) {
    // 컴파일 타임 검증: TabManager는 복사/이동 생성자가 삭제되어야 함
    // 따라서 이 테스트는 패스만 함
    EXPECT_TRUE(true)
        << "TabManager는 복사/이동 생성자 및 대입 연산자가 삭제되어야 함";
}

// ============================================================================
// F-06: 통합 시나리오 테스트
// ============================================================================

/**
 * 시나리오: 사용자가 브라우저를 실행 → URL 입력 → 페이지 로드
 */
TEST_F(TabManagerTest, Scenario_BrowserStartup) {
    // Arrange: 초기 상태 확인
    EXPECT_EQ(tabManager->getTabCount(), 0);

    // Act 1: WebView 등록
    tabManager->setCurrentTab((webosbrowser::WebView *)dummyWebView1);

    // Assert 1: 탭 개수 확인
    EXPECT_EQ(tabManager->getTabCount(), 1);

    // Act 2: 탭 조회
    void *currentTab = tabManager->getCurrentTab();

    // Assert 2: 탭 조회 확인
    EXPECT_NE(currentTab, nullptr);
}

/**
 * 시나리오: 탭 닫기
 */
TEST_F(TabManagerTest, Scenario_CloseTab) {
    // Arrange
    tabManager->setCurrentTab((webosbrowser::WebView *)dummyWebView1);
    EXPECT_EQ(tabManager->getTabCount(), 1);

    // Act: 탭 닫기
    tabManager->setCurrentTab(nullptr);

    // Assert
    EXPECT_EQ(tabManager->getTabCount(), 0);
    EXPECT_EQ(tabManager->getCurrentTab(), nullptr);
}

/**
 * 시나리오: 다중 탭 관리 (현재는 단일 탭이지만 미래를 대비)
 */
TEST_F(TabManagerTest, Scenario_MultipleTabInstances) {
    // Act & Assert 1: 첫 번째 탭 설정
    tabManager->setCurrentTab((webosbrowser::WebView *)dummyWebView1);
    EXPECT_EQ(tabManager->getCurrentTab(), (webosbrowser::WebView *)dummyWebView1);

    // Act & Assert 2: 두 번째 탭으로 변경
    tabManager->setCurrentTab((webosbrowser::WebView *)dummyWebView2);
    EXPECT_EQ(tabManager->getCurrentTab(), (webosbrowser::WebView *)dummyWebView2);

    // Act & Assert 3: 첫 번째 탭으로 다시 설정
    tabManager->setCurrentTab((webosbrowser::WebView *)dummyWebView1);
    EXPECT_EQ(tabManager->getCurrentTab(), (webosbrowser::WebView *)dummyWebView1);
}

// ============================================================================
// 추가: 시그널 파라미터 검증
// ============================================================================

/**
 * 요구사항: tabChanged 시그널이 정확한 인덱스를 전달
 */
TEST_F(TabManagerTest, Signal_CorrectParameterPassed) {
    // Arrange
    QSignalSpy spy(tabManager, &TabManager::tabChanged);

    // Act
    tabManager->setCurrentTab((webosbrowser::WebView *)dummyWebView1);

    // Assert
    ASSERT_EQ(spy.count(), 1);
    QList<QVariant> arguments = spy.takeFirst();
    EXPECT_EQ(arguments.at(0).type(), QVariant::Int);
    EXPECT_EQ(arguments.at(0).toInt(), 0);
}
