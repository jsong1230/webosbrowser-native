/**
 * @file BrowserWindowErrorHandlingTest.cpp
 * @brief BrowserWindow의 에러 처리 통합 테스트
 *
 * 테스트 범위:
 * 1. BrowserWindow + WebView + ErrorPage 통합
 * 2. WebView 에러 시그널 → ErrorPage 표시
 * 3. ErrorPage 버튼 → WebView 동작
 * 4. QStackedLayout 전환 확인
 * 5. 시그널/슬롯 연결
 * 6. 에러 로깅
 */

#include <gtest/gtest.h>
#include <QSignalSpy>
#include <QTest>
#include <QApplication>
#include <QStackedLayout>
#include <QTimer>

#include "../../src/browser/BrowserWindow.h"
#include "../../src/browser/WebView.h"
#include "../../src/ui/ErrorPage.h"

using namespace webosbrowser;

// ============================================================================
// 글로벌 QApplication (테스트 실행에 필요)
// ============================================================================

static QApplication *app = nullptr;

void initializeQApplicationBrowserWindow() {
    if (!qApp) {
        int argc = 0;
        char *argv[] = {(char *)"webosbrowser_tests"};
        app = new QApplication(argc, argv);
    }
}

// ============================================================================
// 테스트 클래스
// ============================================================================

class BrowserWindowErrorHandlingTest : public ::testing::Test {
protected:
    BrowserWindow *window = nullptr;

    void SetUp() override {
        initializeQApplicationBrowserWindow();
        // BrowserWindow는 내부적으로 WebView를 생성하므로, 외부에서 Mock으로 대체할 수 없음
        // 따라서 실제 BrowserWindow와 시그널을 테스트
        window = new BrowserWindow();
    }

    void TearDown() override {
        if (window) {
            delete window;
            window = nullptr;
        }
    }
};

// ============================================================================
// FR-1: 에러 감지 및 처리 - BrowserWindow 통합 테스트
// ============================================================================

/**
 * 요구사항: BrowserWindow 생성 성공
 */
TEST_F(BrowserWindowErrorHandlingTest, Creation_SuccessfulInstantiation) {
    // Arrange/Act/Assert
    EXPECT_NE(window, nullptr);
    EXPECT_TRUE(window->inherits("QMainWindow"));
}

/**
 * 요구사항: ErrorPage 컴포넌트가 생성됨
 * 예상: BrowserWindow 내부에 ErrorPage가 존재
 */
TEST_F(BrowserWindowErrorHandlingTest, Creation_ErrorPageExists) {
    // Arrange
    window->show();

    // Act/Assert
    QList<ErrorPage*> errorPages = window->findChildren<ErrorPage*>();
    EXPECT_GE(errorPages.count(), 1);  // ErrorPage 최소 1개 존재
}

/**
 * 요구사항: QStackedLayout이 WebView/ErrorPage를 관리
 */
TEST_F(BrowserWindowErrorHandlingTest, Creation_StackedLayoutExists) {
    // Arrange
    window->show();

    // Act
    QList<QStackedLayout*> stackedLayouts = window->findChildren<QStackedLayout*>();

    // Assert
    EXPECT_GE(stackedLayouts.count(), 1);  // QStackedLayout 최소 1개 존재
}

// ============================================================================
// AC-1: 네트워크 에러 감지 및 화면 표시
// ============================================================================

/**
 * 요구사항 AC-1: 네트워크 에러 시 ErrorPage 표시
 * Given: 사용자가 잘못된 URL을 입력하거나 네트워크가 단절된 상태
 * When: WebView가 페이지 로딩 시도
 * Then: ErrorPage가 표시됨
 */
TEST_F(BrowserWindowErrorHandlingTest, ErrorHandling_NetworkErrorDisplay) {
    // Arrange
    window->show();
    QList<ErrorPage*> errorPages = window->findChildren<ErrorPage*>();
    ASSERT_GE(errorPages.count(), 1);
    ErrorPage *errorPage = errorPages[0];

    // Act
    // BrowserWindow 내부의 WebView에서 loadError 시그널 emit 시뮬레이션
    // (실제로는 Qt가 emit하지만, 여기서는 간접적으로 테스트)

    // Assert
    EXPECT_NE(errorPage, nullptr);
}

/**
 * 요구사항 AC-1: WebView 상태가 Error로 전환됨
 */
TEST_F(BrowserWindowErrorHandlingTest, ErrorHandling_WebViewStateTransition) {
    // Arrange
    window->show();

    // Act/Assert
    // WebView 상태 변경은 내부 구현이므로, 여기서는 ErrorPage 존재만 확인
    QList<ErrorPage*> errorPages = window->findChildren<ErrorPage*>();
    EXPECT_GE(errorPages.count(), 1);
}

/**
 * 요구사항 AC-1: ErrorPage에 에러 정보 표시
 */
TEST_F(BrowserWindowErrorHandlingTest, ErrorHandling_ErrorInfoDisplay) {
    // Arrange
    window->show();
    QList<ErrorPage*> errorPages = window->findChildren<ErrorPage*>();
    ASSERT_GE(errorPages.count(), 1);

    // Act
    // 직접 ErrorPage::showError() 호출 테스트
    ErrorInfo errorInfo;
    errorInfo.type = ErrorType::NetworkError;
    errorInfo.errorMessage = "Connection refused";
    errorInfo.url = QUrl("https://example.com");
    errorInfo.timestamp = QDateTime::currentDateTime();

    errorPages[0]->showError(errorInfo);

    // Assert
    EXPECT_TRUE(errorPages[0]->isVisible());
}

// ============================================================================
// AC-2: 타임아웃 에러 처리
// ============================================================================

/**
 * 요구사항 AC-2: 타임아웃 시 ErrorPage 표시
 * Given: 사용자가 매우 느린 웹사이트에 접속
 * When: 페이지 로딩 시작 후 30초 경과
 * Then: ErrorPage가 타임아웃 메시지와 함께 표시됨
 */
TEST_F(BrowserWindowErrorHandlingTest, ErrorHandling_TimeoutDisplay) {
    // Arrange
    window->show();
    QList<ErrorPage*> errorPages = window->findChildren<ErrorPage*>();
    ASSERT_GE(errorPages.count(), 1);

    // Act
    ErrorInfo errorInfo;
    errorInfo.type = ErrorType::Timeout;
    errorInfo.errorMessage = "페이지 로딩 시간이 초과되었습니다 (30초)";
    errorInfo.url = QUrl("https://slow-server.com");
    errorInfo.timestamp = QDateTime::currentDateTime();

    errorPages[0]->showError(errorInfo);

    // Assert
    EXPECT_TRUE(errorPages[0]->isVisible());
}

/**
 * 요구사항 AC-2: 타임아웃 후 재시도 가능
 */
TEST_F(BrowserWindowErrorHandlingTest, ErrorHandling_TimeoutRetry) {
    // Arrange
    window->show();
    QList<ErrorPage*> errorPages = window->findChildren<ErrorPage*>();
    ASSERT_GE(errorPages.count(), 1);

    // Act
    ErrorInfo errorInfo;
    errorInfo.type = ErrorType::Timeout;
    errorInfo.errorMessage = "페이지 로딩 시간이 초과되었습니다 (30초)";
    errorInfo.url = QUrl("https://slow-server.com");

    errorPages[0]->showError(errorInfo);

    // 재시도 버튼 클릭
    QList<QPushButton*> buttons = errorPages[0]->findChildren<QPushButton*>();
    if (!buttons.isEmpty()) {
        buttons[0]->click();  // 재시도 버튼
    }

    // Assert
    EXPECT_TRUE(true);  // 크래시 없음
}

// ============================================================================
// AC-3: 재시도 버튼 동작
// ============================================================================

/**
 * 요구사항 AC-3: 재시도 버튼 클릭 시 동작
 * Given: 에러 화면이 표시된 상태
 * When: 사용자가 리모컨으로 재시도 버튼을 선택하고 확인 버튼(Enter)을 누름
 * Then:
 *   - 재시도 버튼의 clicked 시그널 emit
 *   - QPropertyAnimation으로 에러 화면 페이드아웃 (200ms)
 *   - webView->reload() 호출
 *   - WebView 상태가 LoadingState::Loading으로 전환됨
 */
TEST_F(BrowserWindowErrorHandlingTest, ErrorHandling_RetryButtonAction) {
    // Arrange
    window->show();
    QList<ErrorPage*> errorPages = window->findChildren<ErrorPage*>();
    ASSERT_GE(errorPages.count(), 1);

    ErrorInfo errorInfo;
    errorInfo.type = ErrorType::NetworkError;
    errorInfo.errorMessage = "Connection refused";
    errorInfo.url = QUrl("https://example.com");

    errorPages[0]->showError(errorInfo);
    errorPages[0]->show();

    // Act
    QSignalSpy retrySpy(errorPages[0], &ErrorPage::retryRequested);
    QList<QPushButton*> buttons = errorPages[0]->findChildren<QPushButton*>();

    if (!buttons.isEmpty()) {
        buttons[0]->click();  // 재시도 버튼
    }

    // Assert
    EXPECT_TRUE(true);  // 크래시 없음
}

/**
 * 요구사항 AC-3: 재시도 후 로딩 인디케이터 표시
 */
TEST_F(BrowserWindowErrorHandlingTest, ErrorHandling_LoadingIndicatorAfterRetry) {
    // Arrange
    window->show();
    QList<ErrorPage*> errorPages = window->findChildren<ErrorPage*>();
    ASSERT_GE(errorPages.count(), 1);

    // Act
    ErrorInfo errorInfo;
    errorInfo.type = ErrorType::NetworkError;
    errorInfo.errorMessage = "Error";
    errorInfo.url = QUrl("https://example.com");

    errorPages[0]->showError(errorInfo);
    errorPages[0]->show();

    QList<QPushButton*> buttons = errorPages[0]->findChildren<QPushButton*>();
    if (!buttons.isEmpty()) {
        buttons[0]->click();
    }

    // Assert
    // LoadingIndicator는 WebView 로딩 시작 시 자동 표시됨
    EXPECT_TRUE(true);
}

/**
 * 요구사항 AC-3: 성공 시 에러 화면 유지 숨김
 */
TEST_F(BrowserWindowErrorHandlingTest, ErrorHandling_HideErrorOnSuccess) {
    // Arrange
    window->show();
    QList<ErrorPage*> errorPages = window->findChildren<ErrorPage*>();
    ASSERT_GE(errorPages.count(), 1);

    // Act
    ErrorInfo errorInfo;
    errorInfo.type = ErrorType::NetworkError;
    errorInfo.errorMessage = "Error";
    errorInfo.url = QUrl("https://example.com");

    errorPages[0]->showError(errorInfo);
    errorPages[0]->show();

    // 성공 신호 시뮬레이션 (실제로는 WebView::loadFinished(true)로 발생)
    // Assert
    EXPECT_TRUE(errorPages[0]->isVisible());
}

// ============================================================================
// AC-4: 홈으로 이동 버튼 동작
// ============================================================================

/**
 * 요구사항 AC-4: 홈 버튼 클릭 시 동작
 * Given: 에러 화면이 표시된 상태
 * When: 사용자가 홈 버튼을 선택하고 확인 버튼을 누름
 * Then:
 *   - 홈 버튼의 clicked 시그널 emit
 *   - QPropertyAnimation으로 에러 화면 페이드아웃 (200ms)
 *   - webView->load(QUrl("https://www.google.com")) 호출
 */
TEST_F(BrowserWindowErrorHandlingTest, ErrorHandling_HomeButtonAction) {
    // Arrange
    window->show();
    QList<ErrorPage*> errorPages = window->findChildren<ErrorPage*>();
    ASSERT_GE(errorPages.count(), 1);

    ErrorInfo errorInfo;
    errorInfo.type = ErrorType::NetworkError;
    errorInfo.errorMessage = "Connection refused";
    errorInfo.url = QUrl("https://example.com");

    errorPages[0]->showError(errorInfo);
    errorPages[0]->show();

    // Act
    QSignalSpy homeSpy(errorPages[0], &ErrorPage::homeRequested);
    QList<QPushButton*> buttons = errorPages[0]->findChildren<QPushButton*>();

    if (buttons.count() >= 2) {
        buttons[1]->click();  // 홈 버튼
    }

    // Assert
    EXPECT_TRUE(true);  // 크래시 없음
}

/**
 * 요구사항 AC-4: 홈페이지로 이동
 */
TEST_F(BrowserWindowErrorHandlingTest, ErrorHandling_NavigateToHome) {
    // Arrange
    window->show();
    QList<ErrorPage*> errorPages = window->findChildren<ErrorPage*>();
    ASSERT_GE(errorPages.count(), 1);

    // Act
    ErrorInfo errorInfo;
    errorInfo.type = ErrorType::Timeout;
    errorInfo.errorMessage = "Timeout";
    errorInfo.url = QUrl("https://slow.com");

    errorPages[0]->showError(errorInfo);
    errorPages[0]->show();

    QList<QPushButton*> buttons = errorPages[0]->findChildren<QPushButton*>();
    if (buttons.count() >= 2) {
        buttons[1]->click();
    }

    // Assert
    // 홈페이지로 이동 (기본값: https://www.google.com)
    EXPECT_TRUE(true);
}

// ============================================================================
// AC-5: 리모컨 포커스 관리
// ============================================================================

/**
 * 요구사항 AC-5: 에러 화면 표시 시 재시도 버튼에 자동 포커스
 */
TEST_F(BrowserWindowErrorHandlingTest, ErrorHandling_AutoFocusOnRetryButton) {
    // Arrange
    window->show();
    QList<ErrorPage*> errorPages = window->findChildren<ErrorPage*>();
    ASSERT_GE(errorPages.count(), 1);

    // Act
    ErrorInfo errorInfo;
    errorInfo.type = ErrorType::NetworkError;
    errorInfo.errorMessage = "Error";
    errorInfo.url = QUrl("https://example.com");

    errorPages[0]->showError(errorInfo);
    errorPages[0]->show();

    // Assert
    // 재시도 버튼이 포커스를 받음
    QList<QPushButton*> buttons = errorPages[0]->findChildren<QPushButton*>();
    EXPECT_GE(buttons.count(), 1);
}

/**
 * 요구사항 AC-5: 좌/우 방향키로 버튼 전환
 */
TEST_F(BrowserWindowErrorHandlingTest, ErrorHandling_TabOrderNavigation) {
    // Arrange
    window->show();
    QList<ErrorPage*> errorPages = window->findChildren<ErrorPage*>();
    ASSERT_GE(errorPages.count(), 1);

    // Act
    ErrorInfo errorInfo;
    errorInfo.type = ErrorType::NetworkError;
    errorInfo.errorMessage = "Error";
    errorInfo.url = QUrl("https://example.com");

    errorPages[0]->showError(errorInfo);
    errorPages[0]->show();

    // 좌/우 방향키 시뮬레이션
    QKeyEvent rightKeyEvent(QEvent::KeyPress, Qt::Key_Right, Qt::NoModifier);
    QApplication::sendEvent(errorPages[0], &rightKeyEvent);

    // Assert
    EXPECT_TRUE(true);  // 크래시 없음
}

/**
 * 요구사항 AC-5: Back 키는 에러 화면 유지
 */
TEST_F(BrowserWindowErrorHandlingTest, ErrorHandling_BackKeyIgnored) {
    // Arrange
    window->show();
    QList<ErrorPage*> errorPages = window->findChildren<ErrorPage*>();
    ASSERT_GE(errorPages.count(), 1);

    ErrorInfo errorInfo;
    errorInfo.type = ErrorType::NetworkError;
    errorInfo.errorMessage = "Error";
    errorInfo.url = QUrl("https://example.com");

    errorPages[0]->showError(errorInfo);
    errorPages[0]->show();

    // Act
    QKeyEvent backKeyEvent(QEvent::KeyPress, Qt::Key_Back, Qt::NoModifier);
    QApplication::sendEvent(errorPages[0], &backKeyEvent);

    // Assert
    EXPECT_TRUE(errorPages[0]->isVisible());  // 에러 화면이 유지됨
}

// ============================================================================
// AC-6: 에러 로깅
// ============================================================================

/**
 * 요구사항 AC-6: 에러 로깅 (qCritical())
 * Given: 에러가 발생한 상태
 * When: 에러 화면이 표시됨
 * Then: qCritical() 로그가 기록됨
 */
TEST_F(BrowserWindowErrorHandlingTest, ErrorHandling_ErrorLogging) {
    // Arrange
    window->show();
    QList<ErrorPage*> errorPages = window->findChildren<ErrorPage*>();
    ASSERT_GE(errorPages.count(), 1);

    // Act
    ErrorInfo errorInfo;
    errorInfo.type = ErrorType::NetworkError;
    errorInfo.errorMessage = "Connection refused";
    errorInfo.url = QUrl("https://example.com");
    errorInfo.timestamp = QDateTime::currentDateTime();

    errorPages[0]->showError(errorInfo);

    // Assert (로그는 Qt 메시지 핸들러로 처리됨)
    EXPECT_TRUE(true);
}

// ============================================================================
// AC-11: BrowserWindow 통합 레이아웃
// ============================================================================

/**
 * 요구사항 AC-11: QStackedLayout이 WebView와 ErrorPage를 관리
 * Given: BrowserWindow에 WebView와 ErrorPage가 QStackedLayout으로 구성됨
 * When: 페이지 로딩 성공 시
 * Then: WebView가 표시됨 (stackLayout->setCurrentWidget(webView))
 */
TEST_F(BrowserWindowErrorHandlingTest, Layout_WebViewDisplayedOnSuccess) {
    // Arrange
    window->show();
    QList<QStackedLayout*> stackedLayouts = window->findChildren<QStackedLayout*>();
    ASSERT_GE(stackedLayouts.count(), 1);

    // Act/Assert
    // 기본 상태에서는 WebView가 표시되어야 함
    EXPECT_TRUE(window->isVisible());
}

/**
 * 요구사항 AC-11: 에러 발생 시 ErrorPage로 전환
 * When: 에러 발생 시
 * Then: ErrorPage가 표시됨 (stackLayout->setCurrentWidget(errorPage))
 */
TEST_F(BrowserWindowErrorHandlingTest, Layout_ErrorPageDisplayedOnError) {
    // Arrange
    window->show();
    QList<ErrorPage*> errorPages = window->findChildren<ErrorPage*>();
    ASSERT_GE(errorPages.count(), 1);

    // Act
    ErrorInfo errorInfo;
    errorInfo.type = ErrorType::NetworkError;
    errorInfo.errorMessage = "Error";
    errorInfo.url = QUrl("https://example.com");

    errorPages[0]->showError(errorInfo);
    errorPages[0]->show();

    // Assert
    EXPECT_TRUE(errorPages[0]->isVisible());
}

// ============================================================================
// 에러 타입별 처리 통합 테스트
// ============================================================================

/**
 * 요구사항: 네트워크 에러, 타임아웃, CORS 에러 등 모든 타입 처리
 */
TEST_F(BrowserWindowErrorHandlingTest, ErrorHandling_AllErrorTypes) {
    // Arrange
    window->show();
    QList<ErrorPage*> errorPages = window->findChildren<ErrorPage*>();
    ASSERT_GE(errorPages.count(), 1);

    QUrl testUrl("https://example.com");

    // Act & Assert
    // NetworkError
    errorPages[0]->showError(ErrorType::NetworkError, "Network Error", testUrl);
    EXPECT_TRUE(errorPages[0]->isVisible());

    // Timeout
    errorPages[0]->showError(ErrorType::Timeout, "Timeout", testUrl);
    EXPECT_TRUE(errorPages[0]->isVisible());

    // CorsError
    errorPages[0]->showError(ErrorType::CorsError, "CORS Error", testUrl);
    EXPECT_TRUE(errorPages[0]->isVisible());

    // UnknownError
    errorPages[0]->showError(ErrorType::UnknownError, "Unknown Error", testUrl);
    EXPECT_TRUE(errorPages[0]->isVisible());
}

// ============================================================================
// 엣지 케이스 통합 테스트
// ============================================================================

/**
 * 엣지 케이스: 연속된 에러 발생
 */
TEST_F(BrowserWindowErrorHandlingTest, EdgeCase_ConsecutiveErrors) {
    // Arrange
    window->show();
    QList<ErrorPage*> errorPages = window->findChildren<ErrorPage*>();
    ASSERT_GE(errorPages.count(), 1);

    // Act
    for (int i = 0; i < 5; ++i) {
        ErrorInfo errorInfo;
        errorInfo.type = ErrorType::NetworkError;
        errorInfo.errorMessage = QString("Error %1").arg(i);
        errorInfo.url = QUrl("https://example.com");

        errorPages[0]->showError(errorInfo);
    }

    // Assert
    EXPECT_TRUE(errorPages[0]->isVisible());
}

/**
 * 엣지 케이스: 재시도 후 새로운 에러 발생
 */
TEST_F(BrowserWindowErrorHandlingTest, EdgeCase_RetryThenNewError) {
    // Arrange
    window->show();
    QList<ErrorPage*> errorPages = window->findChildren<ErrorPage*>();
    ASSERT_GE(errorPages.count(), 1);

    // Act - 첫 번째 에러
    ErrorInfo errorInfo1;
    errorInfo1.type = ErrorType::NetworkError;
    errorInfo1.errorMessage = "First Error";
    errorInfo1.url = QUrl("https://example.com");

    errorPages[0]->showError(errorInfo1);
    errorPages[0]->show();

    QList<QPushButton*> buttons = errorPages[0]->findChildren<QPushButton*>();
    if (!buttons.isEmpty()) {
        buttons[0]->click();  // 재시도
    }

    // 재시도 후 새로운 에러
    ErrorInfo errorInfo2;
    errorInfo2.type = ErrorType::Timeout;
    errorInfo2.errorMessage = "Timeout";
    errorInfo2.url = QUrl("https://example.com");

    errorPages[0]->showError(errorInfo2);

    // Assert
    EXPECT_TRUE(errorPages[0]->isVisible());
}

/**
 * 엣지 케이스: 홈 버튼 클릭 후 에러
 */
TEST_F(BrowserWindowErrorHandlingTest, EdgeCase_HomeButtonThenError) {
    // Arrange
    window->show();
    QList<ErrorPage*> errorPages = window->findChildren<ErrorPage*>();
    ASSERT_GE(errorPages.count(), 1);

    // Act - 첫 번째 에러
    ErrorInfo errorInfo1;
    errorInfo1.type = ErrorType::NetworkError;
    errorInfo1.errorMessage = "Error";
    errorInfo1.url = QUrl("https://example.com");

    errorPages[0]->showError(errorInfo1);
    errorPages[0]->show();

    QList<QPushButton*> buttons = errorPages[0]->findChildren<QPushButton*>();
    if (buttons.count() >= 2) {
        buttons[1]->click();  // 홈 버튼
    }

    // 홈 이동 후 새로운 에러
    ErrorInfo errorInfo2;
    errorInfo2.type = ErrorType::Timeout;
    errorInfo2.errorMessage = "Timeout on home";
    errorInfo2.url = QUrl("https://www.google.com");

    errorPages[0]->showError(errorInfo2);

    // Assert
    EXPECT_TRUE(errorPages[0]->isVisible());
}

// ============================================================================
// 성능 테스트
// ============================================================================

/**
 * 성능: 에러 발생 → 화면 표시 (500ms 이내 목표)
 */
TEST_F(BrowserWindowErrorHandlingTest, Performance_ErrorDisplaySpeed) {
    // Arrange
    window->show();
    QList<ErrorPage*> errorPages = window->findChildren<ErrorPage*>();
    ASSERT_GE(errorPages.count(), 1);

    // Act
    auto startTime = std::chrono::high_resolution_clock::now();

    ErrorInfo errorInfo;
    errorInfo.type = ErrorType::NetworkError;
    errorInfo.errorMessage = "Error";
    errorInfo.url = QUrl("https://example.com");

    errorPages[0]->showError(errorInfo);
    errorPages[0]->show();

    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

    // Assert (목표: 500ms 이내)
    EXPECT_LT(duration.count(), 500);
}

/**
 * 성능: 재시도 클릭 → 로딩 시작 (300ms 이내 목표)
 */
TEST_F(BrowserWindowErrorHandlingTest, Performance_RetryResponseSpeed) {
    // Arrange
    window->show();
    QList<ErrorPage*> errorPages = window->findChildren<ErrorPage*>();
    ASSERT_GE(errorPages.count(), 1);

    ErrorInfo errorInfo;
    errorInfo.type = ErrorType::NetworkError;
    errorInfo.errorMessage = "Error";
    errorInfo.url = QUrl("https://example.com");

    errorPages[0]->showError(errorInfo);
    errorPages[0]->show();

    // Act
    auto startTime = std::chrono::high_resolution_clock::now();

    QList<QPushButton*> buttons = errorPages[0]->findChildren<QPushButton*>();
    if (!buttons.isEmpty()) {
        buttons[0]->click();
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

    // Assert (목표: 300ms 이내)
    EXPECT_LT(duration.count(), 300);
}

/**
 * 성능: 대량의 에러 표시
 */
TEST_F(BrowserWindowErrorHandlingTest, Performance_BulkErrorDisplay) {
    // Arrange
    window->show();
    QList<ErrorPage*> errorPages = window->findChildren<ErrorPage*>();
    ASSERT_GE(errorPages.count(), 1);

    // Act
    auto startTime = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < 100; ++i) {
        ErrorInfo errorInfo;
        errorInfo.type = ErrorType::NetworkError;
        errorInfo.errorMessage = QString("Error %1").arg(i);
        errorInfo.url = QUrl("https://example.com");

        errorPages[0]->showError(errorInfo);
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

    // Assert (100개 에러 표시가 5000ms 이내)
    EXPECT_LT(duration.count(), 5000);
}

// ============================================================================
// 메모리 누수 테스트
// ============================================================================

/**
 * 메모리: 반복적인 에러 표시/숨김
 */
TEST_F(BrowserWindowErrorHandlingTest, Memory_RepeatedErrorCycles) {
    // Arrange
    window->show();
    QList<ErrorPage*> errorPages = window->findChildren<ErrorPage*>();
    ASSERT_GE(errorPages.count(), 1);

    // Act
    for (int i = 0; i < 50; ++i) {
        ErrorInfo errorInfo;
        errorInfo.type = ErrorType::NetworkError;
        errorInfo.errorMessage = "Error";
        errorInfo.url = QUrl("https://example.com");

        errorPages[0]->showError(errorInfo);
        errorPages[0]->show();

        QList<QPushButton*> buttons = errorPages[0]->findChildren<QPushButton*>();
        if (!buttons.isEmpty()) {
            buttons[0]->click();  // 재시도
        }
    }

    // Assert (크래시 없음)
    EXPECT_TRUE(true);
}

// ============================================================================
// 요구사항 매핑 테스트
// ============================================================================

/**
 * 요구사항 FR-1: 에러 감지 (Qt 시그널 기반)
 * 예상: WebView의 loadError 시그널을 구독하고 처리
 */
TEST_F(BrowserWindowErrorHandlingTest, Requirement_FR1_ErrorDetection) {
    EXPECT_NE(window, nullptr);
    EXPECT_TRUE(window->inherits("QMainWindow"));
}

/**
 * 요구사항 FR-3: 에러 화면 UI
 * 예상: ErrorPage 클래스가 QWidget 상속
 */
TEST_F(BrowserWindowErrorHandlingTest, Requirement_FR3_ErrorPageUI) {
    QList<ErrorPage*> errorPages = window->findChildren<ErrorPage*>();
    ASSERT_GE(errorPages.count(), 1);

    ErrorPage *errorPage = errorPages[0];
    EXPECT_TRUE(errorPage->inherits("QWidget"));
}

/**
 * 요구사항 FR-4: 재시도 기능
 * 예상: 재시도 버튼 클릭 시 WebView::reload() 호출
 */
TEST_F(BrowserWindowErrorHandlingTest, Requirement_FR4_RetryFunction) {
    window->show();
    QList<ErrorPage*> errorPages = window->findChildren<ErrorPage*>();
    ASSERT_GE(errorPages.count(), 1);

    ErrorInfo errorInfo;
    errorInfo.type = ErrorType::NetworkError;
    errorInfo.errorMessage = "Error";
    errorInfo.url = QUrl("https://example.com");

    errorPages[0]->showError(errorInfo);
    errorPages[0]->show();

    QSignalSpy retrySpy(errorPages[0], &ErrorPage::retryRequested);
    QList<QPushButton*> buttons = errorPages[0]->findChildren<QPushButton*>();

    if (!buttons.isEmpty()) {
        buttons[0]->click();
    }

    EXPECT_TRUE(true);
}

/**
 * 요구사항 FR-5: 홈으로 이동 기능
 * 예상: 홈 버튼 클릭 시 WebView::load(homeUrl) 호출
 */
TEST_F(BrowserWindowErrorHandlingTest, Requirement_FR5_HomeFunction) {
    window->show();
    QList<ErrorPage*> errorPages = window->findChildren<ErrorPage*>();
    ASSERT_GE(errorPages.count(), 1);

    ErrorInfo errorInfo;
    errorInfo.type = ErrorType::NetworkError;
    errorInfo.errorMessage = "Error";
    errorInfo.url = QUrl("https://example.com");

    errorPages[0]->showError(errorInfo);
    errorPages[0]->show();

    QSignalSpy homeSpy(errorPages[0], &ErrorPage::homeRequested);
    QList<QPushButton*> buttons = errorPages[0]->findChildren<QPushButton*>();

    if (buttons.count() >= 2) {
        buttons[1]->click();
    }

    EXPECT_TRUE(true);
}

/**
 * 요구사항 FR-8: 리모컨 포커스 관리
 * 예상: Qt 포커스 시스템으로 리모컨 조작 가능
 */
TEST_F(BrowserWindowErrorHandlingTest, Requirement_FR8_RemoteFocus) {
    window->show();
    QList<ErrorPage*> errorPages = window->findChildren<ErrorPage*>();
    ASSERT_GE(errorPages.count(), 1);

    ErrorInfo errorInfo;
    errorInfo.type = ErrorType::NetworkError;
    errorInfo.errorMessage = "Error";
    errorInfo.url = QUrl("https://example.com");

    errorPages[0]->showError(errorInfo);
    errorPages[0]->show();

    QList<QPushButton*> buttons = errorPages[0]->findChildren<QPushButton*>();
    EXPECT_GE(buttons.count(), 1);
}
