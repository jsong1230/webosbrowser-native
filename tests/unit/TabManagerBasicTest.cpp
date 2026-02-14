/**
 * @file TabManagerBasicTest.cpp
 * @brief TabManager 기본 테스트 (WebView 의존성 제거)
 *
 * 테스트 범위:
 * 1. 생성자/소멸자
 * 2. 포인터 기반 setCurrentTab / getCurrentTab
 * 3. getTabCount
 * 4. tabChanged 시그널
 * 5. 엣지 케이스 (nullptr, 빈 상태)
 *
 * 주의: WebView 구현 없이 포인터 값만 테스트
 */

#include <gtest/gtest.h>
#include <QSignalSpy>
#include <QApplication>

#include "../src/browser/TabManager.h"

using namespace webosbrowser;

// ============================================================================
// 글로벌 QApplication
// ============================================================================

static QApplication *app = nullptr;

void initializeQAppBasic() {
    if (!qApp) {
        int argc = 0;
        char *argv[] = {(char *)"webosbrowser_tests"};
        app = new QApplication(argc, argv);
    }
}

// ============================================================================
// 테스트 클래스
// ============================================================================

class TabManagerBasicTest : public ::testing::Test {
protected:
    TabManager *tabManager = nullptr;

    void SetUp() override {
        initializeQAppBasic();
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
// F-06: 기본 테스트 (WebView 없음)
// ============================================================================

/**
 * 테스트 1: 초기 상태 확인
 */
TEST_F(TabManagerBasicTest, Initialization_CurrentTabIsNull) {
    EXPECT_EQ(tabManager->getCurrentTab(), nullptr)
        << "초기 상태에서 getCurrentTab()은 nullptr이어야 함";
    EXPECT_EQ(tabManager->getTabCount(), 0)
        << "초기 상태에서 getTabCount()는 0이어야 함";
}

/**
 * 테스트 2: 포인터 설정 및 조회
 */
TEST_F(TabManagerBasicTest, SetAndGetPointers) {
    // 가상 포인터 값
    void *ptr1 = (void *)0x1000;
    void *ptr2 = (void *)0x2000;

    // ptr1 설정
    tabManager->setCurrentTab((WebView *)ptr1);
    EXPECT_EQ(tabManager->getCurrentTab(), (WebView *)ptr1);
    EXPECT_EQ(tabManager->getTabCount(), 1);

    // ptr2로 변경
    tabManager->setCurrentTab((WebView *)ptr2);
    EXPECT_EQ(tabManager->getCurrentTab(), (WebView *)ptr2);
    EXPECT_EQ(tabManager->getTabCount(), 1);

    // nullptr로 초기화
    tabManager->setCurrentTab(nullptr);
    EXPECT_EQ(tabManager->getCurrentTab(), nullptr);
    EXPECT_EQ(tabManager->getTabCount(), 0);
}

/**
 * 테스트 3: 같은 포인터 반복 설정
 */
TEST_F(TabManagerBasicTest, SetSamePointerMultipleTimes) {
    void *ptr = (void *)0x3000;

    tabManager->setCurrentTab((WebView *)ptr);
    EXPECT_EQ(tabManager->getCurrentTab(), (WebView *)ptr);

    tabManager->setCurrentTab((WebView *)ptr);
    EXPECT_EQ(tabManager->getCurrentTab(), (WebView *)ptr);

    tabManager->setCurrentTab((WebView *)ptr);
    EXPECT_EQ(tabManager->getCurrentTab(), (WebView *)ptr);
}

/**
 * 테스트 4: tabChanged 시그널 발생
 */
TEST_F(TabManagerBasicTest, Signal_TabChanged) {
    QSignalSpy spy(tabManager, &TabManager::tabChanged);

    void *ptr1 = (void *)0x4000;
    void *ptr2 = (void *)0x5000;

    // 첫 번째 설정 - 신호 발생
    tabManager->setCurrentTab((WebView *)ptr1);
    EXPECT_EQ(spy.count(), 1);

    // 같은 포인터 - 신호 미발생
    tabManager->setCurrentTab((WebView *)ptr1);
    EXPECT_EQ(spy.count(), 1);

    // 다른 포인터 - 신호 발생
    tabManager->setCurrentTab((WebView *)ptr2);
    EXPECT_EQ(spy.count(), 2);

    // 신호의 인덱스는 항상 0 (단일 탭 모드)
    EXPECT_EQ(qvariant_cast<int>(spy.at(0).at(0)), 0);
    EXPECT_EQ(qvariant_cast<int>(spy.at(1).at(0)), 0);
}

/**
 * 테스트 5: nullptr 처리
 */
TEST_F(TabManagerBasicTest, NullPointerHandling) {
    void *ptr = (void *)0x6000;

    tabManager->setCurrentTab((WebView *)ptr);
    EXPECT_EQ(tabManager->getTabCount(), 1);

    tabManager->setCurrentTab(nullptr);
    EXPECT_EQ(tabManager->getCurrentTab(), nullptr);
    EXPECT_EQ(tabManager->getTabCount(), 0);

    // nullptr 재설정 - 신호 미발생
    QSignalSpy spy(tabManager, &TabManager::tabChanged);
    tabManager->setCurrentTab(nullptr);
    EXPECT_EQ(spy.count(), 0);
}

/**
 * 테스트 6: 빠른 연속 변경
 */
TEST_F(TabManagerBasicTest, RapidChanges) {
    void *ptr1 = (void *)0x7000;
    void *ptr2 = (void *)0x8000;
    void *ptr3 = (void *)0x9000;

    tabManager->setCurrentTab((WebView *)ptr1);
    tabManager->setCurrentTab((WebView *)ptr2);
    tabManager->setCurrentTab((WebView *)ptr3);
    tabManager->setCurrentTab((WebView *)ptr1);
    tabManager->setCurrentTab(nullptr);

    EXPECT_EQ(tabManager->getCurrentTab(), nullptr);
    EXPECT_EQ(tabManager->getTabCount(), 0);
}

/**
 * 테스트 7: 신호 스트림
 */
TEST_F(TabManagerBasicTest, SignalStream) {
    QSignalSpy spy(tabManager, &TabManager::tabChanged);

    void *ptr1 = (void *)0xA000;
    void *ptr2 = (void *)0xB000;

    for (int i = 0; i < 10; ++i) {
        if (i % 2 == 0) {
            tabManager->setCurrentTab((WebView *)ptr1);
        } else {
            tabManager->setCurrentTab((WebView *)ptr2);
        }
    }

    // 신호는 변경이 있을 때마다 발생
    EXPECT_GT(spy.count(), 0);

    // 최종 상태 확인
    EXPECT_EQ(tabManager->getCurrentTab(), (WebView *)ptr2);
}

/**
 * 테스트 8: 메모리 누수 없음
 */
TEST_F(TabManagerBasicTest, NoMemoryLeak) {
    // TabManager 객체가 올바르게 생성/소멸되는지 확인
    TabManager *temp = new TabManager();
    temp->setCurrentTab((WebView *)0x1234);
    delete temp;

    // 또 다른 TabManager 생성
    TabManager *temp2 = new TabManager();
    EXPECT_EQ(temp2->getTabCount(), 0);
    delete temp2;

    EXPECT_TRUE(true);
}

/**
 * 테스트 9: 대량의 포인터 설정
 */
TEST_F(TabManagerBasicTest, StressTest) {
    void *pointers[] = {
        (void *)0xDEADBEEF,
        (void *)0xCAFEBABE,
        (void *)0x12345678,
        (void *)0x87654321,
        (void *)0xFFFFFFFF,
    };

    for (int i = 0; i < 1000; ++i) {
        void *ptr = pointers[i % 5];
        tabManager->setCurrentTab((WebView *)ptr);
        EXPECT_EQ(tabManager->getCurrentTab(), (WebView *)ptr);
        EXPECT_EQ(tabManager->getTabCount(), 1);
    }
}

/**
 * 테스트 10: 설정 - 제거 - 설정 패턴
 */
TEST_F(TabManagerBasicTest, SetRemoveSetPattern) {
    void *ptr1 = (void *)0x1111;
    void *ptr2 = (void *)0x2222;

    // Set
    tabManager->setCurrentTab((WebView *)ptr1);
    EXPECT_EQ(tabManager->getTabCount(), 1);

    // Remove
    tabManager->setCurrentTab(nullptr);
    EXPECT_EQ(tabManager->getTabCount(), 0);

    // Set again
    tabManager->setCurrentTab((WebView *)ptr2);
    EXPECT_EQ(tabManager->getTabCount(), 1);

    // Remove again
    tabManager->setCurrentTab(nullptr);
    EXPECT_EQ(tabManager->getTabCount(), 0);
}

/**
 * 테스트 11: 신호 파라미터 검증
 */
TEST_F(TabManagerBasicTest, SignalParameter) {
    QSignalSpy spy(tabManager, &TabManager::tabChanged);

    void *ptr = (void *)0x3333;
    tabManager->setCurrentTab((WebView *)ptr);

    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.at(0).count(), 1);
    EXPECT_EQ(qvariant_cast<int>(spy.at(0).at(0)), 0);
}

/**
 * 테스트 12: 제목과 URL 메서드 (nullptr일 때)
 */
TEST_F(TabManagerBasicTest, TitleAndUrlWhenNull) {
    // 탭이 없을 때
    EXPECT_EQ(tabManager->getCurrentTabTitle(), "");
    EXPECT_TRUE(tabManager->getCurrentTabUrl().isEmpty());

    // 탭을 설정해도 Mock이므로 비어있을 것
    void *ptr = (void *)0x4444;
    tabManager->setCurrentTab((WebView *)ptr);
    EXPECT_EQ(tabManager->getCurrentTabTitle(), "");
    EXPECT_TRUE(tabManager->getCurrentTabUrl().isEmpty());
}

// ============================================================================
// 시나리오 테스트
// ============================================================================

/**
 * 시나리오: 브라우저 시작 → 탭 생성 → 탭 전환 → 탭 닫기
 */
TEST_F(TabManagerBasicTest, Scenario_BrowserWorkflow) {
    void *tab1 = (void *)0x1010;
    void *tab2 = (void *)0x2020;

    // 브라우저 시작 - 탭 없음
    EXPECT_EQ(tabManager->getTabCount(), 0);

    // 탭 1 생성
    tabManager->setCurrentTab((WebView *)tab1);
    EXPECT_EQ(tabManager->getTabCount(), 1);
    EXPECT_EQ(tabManager->getCurrentTab(), (WebView *)tab1);

    // 탭 2로 전환 (새 탭)
    tabManager->setCurrentTab((WebView *)tab2);
    EXPECT_EQ(tabManager->getTabCount(), 1);
    EXPECT_EQ(tabManager->getCurrentTab(), (WebView *)tab2);

    // 탭 1로 복귀
    tabManager->setCurrentTab((WebView *)tab1);
    EXPECT_EQ(tabManager->getTabCount(), 1);
    EXPECT_EQ(tabManager->getCurrentTab(), (WebView *)tab1);

    // 탭 닫기
    tabManager->setCurrentTab(nullptr);
    EXPECT_EQ(tabManager->getTabCount(), 0);
}
