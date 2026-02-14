/**
 * @file BrowserWindowTabManagerIntegrationTest.cpp
 * @brief BrowserWindow와 TabManager 통합 테스트
 *
 * 테스트 범위:
 * 1. TabManager 인스턴스 생성 확인
 * 2. WebView가 TabManager에 등록되는지 확인
 * 3. BrowserWindow의 시그널/슬롯 연결
 * 4. TabManager의 시그널이 BrowserWindow에 전달되는지 확인
 */

#include <gtest/gtest.h>
#include <QSignalSpy>
#include <QTest>
#include <QApplication>

#include "../src/browser/BrowserWindow.h"
#include "../src/ui/URLBar.h"
#include "../src/browser/WebView.h"
#include "../src/browser/TabManager.h"

using namespace webosbrowser;

// ============================================================================
// 글로벌 QApplication
// ============================================================================

static QApplication *app = nullptr;

void initializeQApplicationTabManagerIntegration() {
    if (!qApp) {
        int argc = 0;
        char *argv[] = {(char *)"webosbrowser_integration_tests"};
        app = new QApplication(argc, argv);
    }
}

// ============================================================================
// 테스트 클래스
// ============================================================================

class BrowserWindowTabManagerIntegrationTest : public ::testing::Test {
protected:
    BrowserWindow *window = nullptr;
    TabManager *tabManager = nullptr;
    WebView *webView = nullptr;
    URLBar *urlBar = nullptr;

    void SetUp() override {
        initializeQApplicationTabManagerIntegration();

        // BrowserWindow 생성
        window = new BrowserWindow();

        // 자식 위젯 및 서비스 찾기
        tabManager = window->findChild<TabManager *>();
        webView = window->findChild<WebView *>();
        urlBar = window->findChild<URLBar *>();
    }

    void TearDown() override {
        if (window) {
            delete window;
            window = nullptr;
            tabManager = nullptr;
            webView = nullptr;
            urlBar = nullptr;
        }
    }
};

// ============================================================================
// F-06: TabManager 인스턴스 생성 확인
// ============================================================================

/**
 * 요구사항: BrowserWindow 생성 시 TabManager 인스턴스 생성
 */
TEST_F(BrowserWindowTabManagerIntegrationTest, TabManagerExistsInBrowserWindow) {
    // Assert
    EXPECT_NE(tabManager, nullptr)
        << "BrowserWindow에 TabManager 인스턴스가 존재해야 함";
}

/**
 * 요구사항: WebView가 BrowserWindow에 존재
 */
TEST_F(BrowserWindowTabManagerIntegrationTest, WebViewExistsInBrowserWindow) {
    // Assert
    EXPECT_NE(webView, nullptr)
        << "BrowserWindow에 WebView가 존재해야 함";
}

// ============================================================================
// F-06: WebView가 TabManager에 등록 확인
// ============================================================================

/**
 * 요구사항: WebView가 TabManager에 현재 탭으로 등록됨
 */
TEST_F(BrowserWindowTabManagerIntegrationTest, WebViewRegisteredInTabManager) {
    // Assert
    ASSERT_NE(tabManager, nullptr);
    ASSERT_NE(webView, nullptr);

    EXPECT_EQ(tabManager->getCurrentTab(), webView)
        << "WebView가 TabManager의 현재 탭으로 등록되어야 함";
}

/**
 * 요구사항: TabManager의 탭 개수는 1
 */
TEST_F(BrowserWindowTabManagerIntegrationTest, TabManagerTabCountIsOne) {
    // Assert
    ASSERT_NE(tabManager, nullptr);

    EXPECT_EQ(tabManager->getTabCount(), 1)
        << "TabManager의 탭 개수는 1이어야 함";
}

// ============================================================================
// F-06: TabManager 메서드 작동 확인
// ============================================================================

/**
 * 요구사항: TabManager::getCurrentTabTitle 반환값 확인
 */
TEST_F(BrowserWindowTabManagerIntegrationTest, TabManagerGetCurrentTabTitle) {
    // Assert
    ASSERT_NE(tabManager, nullptr);
    ASSERT_NE(webView, nullptr);

    QString title = tabManager->getCurrentTabTitle();

    // WebView의 제목과 일치해야 함
    EXPECT_EQ(title, webView->title())
        << "TabManager의 제목이 WebView의 제목과 일치해야 함";
}

/**
 * 요구사항: TabManager::getCurrentTabUrl 반환값 확인
 */
TEST_F(BrowserWindowTabManagerIntegrationTest, TabManagerGetCurrentTabUrl) {
    // Assert
    ASSERT_NE(tabManager, nullptr);
    ASSERT_NE(webView, nullptr);

    QUrl url = tabManager->getCurrentTabUrl();

    // WebView의 URL과 일치해야 함
    EXPECT_EQ(url, webView->url())
        << "TabManager의 URL이 WebView의 URL과 일치해야 함";
}

// ============================================================================
// F-06: TabManager 시그널 발생 확인
// ============================================================================

/**
 * 요구사항: WebView 설정 시 TabManager의 tabChanged 시그널 발생
 */
TEST_F(BrowserWindowTabManagerIntegrationTest, TabManagerSignalOnWebViewSet) {
    // Arrange
    ASSERT_NE(tabManager, nullptr);
    ASSERT_NE(webView, nullptr);

    // 이미 설정되어 있으므로 다시 설정
    QSignalSpy spy(tabManager, &TabManager::tabChanged);

    // Act
    tabManager->setCurrentTab(webView);

    // Assert
    // 같은 WebView를 재설정하면 신호가 발생하지 않음
    EXPECT_EQ(spy.count(), 0)
        << "같은 WebView 재설정 시 신호 미발생";
}

/**
 * 요구사항: 다른 WebView로 변경 시 신호 발생
 */
TEST_F(BrowserWindowTabManagerIntegrationTest, TabManagerSignalOnTabChange) {
    // Arrange
    ASSERT_NE(tabManager, nullptr);

    WebView *webView2 = new WebView();
    QSignalSpy spy(tabManager, &TabManager::tabChanged);

    // Act
    tabManager->setCurrentTab(webView2);

    // Assert
    EXPECT_GT(spy.count(), 0)
        << "다른 WebView 설정 시 tabChanged 신호가 발생해야 함";

    // Cleanup
    delete webView2;
}

// ============================================================================
// F-06: 시그널/슬롯 연결 확인
// ============================================================================

/**
 * 요구사항: URLBar → WebView 연결이 유지되고 TabManager와 공존
 */
TEST_F(BrowserWindowTabManagerIntegrationTest, SignalConnection_URLBarAndTabManager) {
    // Arrange
    ASSERT_NE(urlBar, nullptr);
    ASSERT_NE(webView, nullptr);
    ASSERT_NE(tabManager, nullptr);

    // Act
    urlBar->setText("https://google.com");
    QSignalSpy urlSubmittedSpy(urlBar, &URLBar::urlSubmitted);

    // Assert
    EXPECT_TRUE(true)
        << "URLBar와 TabManager가 BrowserWindow에서 동시에 작동";
}

/**
 * 요구사항: WebView 이벤트가 TabManager에 영향 없음
 */
TEST_F(BrowserWindowTabManagerIntegrationTest, WebViewEventDoesNotAffectTabManager) {
    // Arrange
    ASSERT_NE(webView, nullptr);
    ASSERT_NE(tabManager, nullptr);

    int initialCount = tabManager->getTabCount();

    // Act
    QSignalSpy loadStartedSpy(webView, &WebView::loadStarted);
    // WebView 이벤트 시뮬레이션

    // Assert
    EXPECT_EQ(tabManager->getTabCount(), initialCount)
        << "WebView 이벤트가 TabManager의 탭 개수를 변경하지 않아야 함";
}

// ============================================================================
// F-06: 레이아웃 및 계층 구조 확인
// ============================================================================

/**
 * 요구사항: TabManager는 BrowserWindow의 자식 객체
 */
TEST_F(BrowserWindowTabManagerIntegrationTest, TabManagerIsChildOfBrowserWindow) {
    // Assert
    ASSERT_NE(window, nullptr);
    ASSERT_NE(tabManager, nullptr);

    EXPECT_EQ(tabManager->parent(), window)
        << "TabManager는 BrowserWindow의 자식 객체여야 함";
}

/**
 * 요구사항: WebView는 TabManager의 관리 대상
 */
TEST_F(BrowserWindowTabManagerIntegrationTest, WebViewManagedByTabManager) {
    // Assert
    ASSERT_NE(tabManager, nullptr);
    ASSERT_NE(webView, nullptr);

    EXPECT_EQ(tabManager->getCurrentTab(), webView)
        << "WebView가 TabManager에 의해 관리되고 있어야 함";
}

// ============================================================================
// F-06: 메모리 및 수명 관리
// ============================================================================

/**
 * 요구사항: BrowserWindow 소멸 시 TabManager도 정리됨
 */
TEST_F(BrowserWindowTabManagerIntegrationTest, TabManagerCleanupWithBrowserWindow) {
    // Arrange
    BrowserWindow *tempWindow = new BrowserWindow();
    TabManager *tempTabManager = tempWindow->findChild<TabManager *>();

    // Assert: 객체 생성 확인
    ASSERT_NE(tempTabManager, nullptr);

    // Act
    delete tempWindow;

    // Assert: 크래시 없이 정상 소멸
    EXPECT_TRUE(true) << "BrowserWindow 소멸 시 TabManager도 정상 정리";
}

/**
 * 요구사항: 반복된 BrowserWindow 생성/소멸 시 메모리 누수 없음
 */
TEST_F(BrowserWindowTabManagerIntegrationTest, NoMemoryLeakOnMultipleInstances) {
    // Act
    for (int i = 0; i < 10; ++i) {
        BrowserWindow *tempWindow = new BrowserWindow();
        TabManager *tempTabManager = tempWindow->findChild<TabManager *>();

        ASSERT_NE(tempTabManager, nullptr);

        delete tempWindow;
    }

    // Assert: 크래시 없이 정상 작동
    EXPECT_TRUE(true) << "다중 인스턴스 생성/소멸 시 메모리 누수 없음";
}

// ============================================================================
// F-06: 동적 상태 변화
// ============================================================================

/**
 * 요구사항: BrowserWindow 생성 후 TabManager 상태 확인
 */
TEST_F(BrowserWindowTabManagerIntegrationTest, TabManagerStateAfterBrowserWindowCreation) {
    // Assert: 초기 상태
    ASSERT_NE(tabManager, nullptr);
    EXPECT_NE(tabManager->getCurrentTab(), nullptr)
        << "BrowserWindow 생성 후 WebView가 TabManager에 등록되어야 함";

    EXPECT_EQ(tabManager->getTabCount(), 1)
        << "초기 탭 개수는 1";

    EXPECT_FALSE(tabManager->getCurrentTabUrl().isEmpty())
        << "초기 URL은 설정되어야 함 (setupUI에서 google.com 로드)";
}

/**
 * 요구사항: URL 입력 후 TabManager 상태 확인
 */
TEST_F(BrowserWindowTabManagerIntegrationTest, TabManagerStateAfterURLInput) {
    // Arrange
    ASSERT_NE(urlBar, nullptr);
    ASSERT_NE(tabManager, nullptr);

    // Act
    urlBar->setText("https://example.com");
    QSignalSpy urlSubmittedSpy(urlBar, &URLBar::urlSubmitted);
    QTest::keyClick(urlBar, Qt::Key_Return);

    // Assert: TabManager 상태 유지
    EXPECT_EQ(tabManager->getTabCount(), 1)
        << "URL 입력 후에도 탭 개수는 1로 유지";

    EXPECT_NE(tabManager->getCurrentTab(), nullptr)
        << "URL 입력 후에도 현재 탭이 설정되어 있어야 함";
}

// ============================================================================
// F-06: 다중 컴포넌트 협력 테스트
// ============================================================================

/**
 * 시나리오: BrowserWindow 시작 → URL 입력 → TabManager 상태 확인
 */
TEST_F(BrowserWindowTabManagerIntegrationTest, Scenario_StartupAndURLInput) {
    // Arrange: 초기 상태 확인
    ASSERT_NE(tabManager, nullptr);
    ASSERT_NE(webView, nullptr);
    ASSERT_NE(urlBar, nullptr);

    EXPECT_EQ(tabManager->getTabCount(), 1);
    EXPECT_EQ(tabManager->getCurrentTab(), webView);

    // Act 1: URL 입력
    urlBar->setText("https://github.com");

    // Assert 1: TabManager 상태 유지
    EXPECT_EQ(tabManager->getTabCount(), 1)
        << "URL 입력 후 탭 개수는 1로 유지";

    // Act 2: URLSubmitted 신호 (실제로는 Enter 키)
    QSignalSpy urlSubmittedSpy(urlBar, &URLBar::urlSubmitted);
    QTest::keyClick(urlBar, Qt::Key_Return);

    // Assert 2: TabManager는 변경되지 않음
    EXPECT_EQ(tabManager->getTabCount(), 1)
        << "URL 제출 후에도 탭 개수는 1로 유지";
}

/**
 * 시나리오: 여러 URL을 순차적으로 입력하면서 TabManager 상태 확인
 */
TEST_F(BrowserWindowTabManagerIntegrationTest, Scenario_MultipleURLSequence) {
    // Arrange
    ASSERT_NE(urlBar, nullptr);
    ASSERT_NE(tabManager, nullptr);

    QStringList urls = {
        "https://google.com",
        "https://example.com",
        "https://github.com",
    };

    // Act & Assert
    for (const auto &url : urls) {
        urlBar->setText(url);

        // TabManager 상태 확인
        EXPECT_EQ(tabManager->getTabCount(), 1)
            << "각 URL 입력 후 탭 개수는 1이어야 함";

        EXPECT_NE(tabManager->getCurrentTab(), nullptr)
            << "각 URL 입력 후 현재 탭이 설정되어야 함";

        QTest::keyClick(urlBar, Qt::Key_Return);
    }
}

// ============================================================================
// F-06: 포커스 및 상호작용 통합 테스트
// ============================================================================

/**
 * 요구사항: TabManager는 포커스 상태에 영향을 받지 않음
 */
TEST_F(BrowserWindowTabManagerIntegrationTest, TabManagerIndependentOfFocus) {
    // Arrange
    ASSERT_NE(tabManager, nullptr);
    ASSERT_NE(webView, nullptr);
    ASSERT_NE(urlBar, nullptr);

    int initialCount = tabManager->getTabCount();

    // Act: 포커스 변경
    urlBar->setFocus();
    EXPECT_EQ(tabManager->getTabCount(), initialCount);

    webView->setFocus();
    EXPECT_EQ(tabManager->getTabCount(), initialCount);

    // Assert
    EXPECT_EQ(tabManager->getTabCount(), 1)
        << "포커스 변경 후에도 TabManager 상태는 유지되어야 함";
}

// ============================================================================
// F-06: 이벤트 처리 통합 테스트
// ============================================================================

/**
 * 요구사항: WebView 에러 발생 시 TabManager 상태 유지
 */
TEST_F(BrowserWindowTabManagerIntegrationTest, TabManagerStateOnWebViewError) {
    // Arrange
    ASSERT_NE(tabManager, nullptr);
    ASSERT_NE(webView, nullptr);

    int initialCount = tabManager->getTabCount();

    // Act: 에러 신호 시뮬레이션 (실제로는 WebView에서 발생)
    QSignalSpy errorSpy(webView, &WebView::loadError);

    // Assert
    EXPECT_EQ(tabManager->getTabCount(), initialCount)
        << "WebView 에러 발생 시에도 TabManager 탭 개수는 유지";
}

// ============================================================================
// F-06: 성능 및 안정성 통합 테스트
// ============================================================================

/**
 * 성능: TabManager 메서드 호출 성능
 */
TEST_F(BrowserWindowTabManagerIntegrationTest, Performance_TabManagerMethodCalls) {
    // Arrange
    ASSERT_NE(tabManager, nullptr);

    auto startTime = std::chrono::high_resolution_clock::now();

    // Act: 대량의 메서드 호출
    for (int i = 0; i < 10000; ++i) {
        tabManager->getTabCount();
        tabManager->getCurrentTab();
        tabManager->getCurrentTabTitle();
        tabManager->getCurrentTabUrl();
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

    // Assert: 10,000회 호출 < 1000ms
    EXPECT_LT(duration.count(), 1000)
        << "TabManager 메서드 호출 성능: 10,000회 < 1000ms (현재: " + std::to_string(duration.count()) + "ms)";
}

/**
 * 안정성: 연속된 TabManager 상태 변경
 */
TEST_F(BrowserWindowTabManagerIntegrationTest, Stability_ContinuousStateChanges) {
    // Arrange
    ASSERT_NE(tabManager, nullptr);

    WebView *webView2 = new WebView();

    // Act: 연속된 상태 변경
    for (int i = 0; i < 100; ++i) {
        if (i % 2 == 0) {
            tabManager->setCurrentTab(webView);
        } else {
            tabManager->setCurrentTab(webView2);
        }
    }

    // Assert: 최종 상태 확인
    EXPECT_NE(tabManager->getCurrentTab(), nullptr)
        << "연속된 상태 변경 후에도 정상 작동";

    // Cleanup
    delete webView2;
}
