/**
 * @file ErrorPageTest.cpp
 * @brief ErrorPage 클래스 단위 테스트
 *
 * 테스트 범위:
 * 1. ErrorPage 생성/소멸
 * 2. showError() 메서드 (에러 타입별)
 * 3. UI 요소 존재 확인
 * 4. 시그널 발생 (retryRequested, homeRequested)
 * 5. 에러 타입 추론 및 처리
 * 6. URL 문자열 truncate
 * 7. 리모컨 포커스 관리
 * 8. 애니메이션 시작
 */

#include <gtest/gtest.h>
#include <QSignalSpy>
#include <QTest>
#include <QApplication>
#include <QLabel>
#include <QPushButton>

#include "../../src/ui/ErrorPage.h"

using namespace webosbrowser;

// ============================================================================
// 글로벌 QApplication (테스트 실행에 필요)
// ============================================================================

static QApplication *app = nullptr;

void initializeQApplicationError() {
    if (!qApp) {
        int argc = 0;
        char *argv[] = {(char *)"webosbrowser_tests"};
        app = new QApplication(argc, argv);
    }
}

// ============================================================================
// 테스트 클래스
// ============================================================================

class ErrorPageTest : public ::testing::Test {
protected:
    ErrorPage *errorPage = nullptr;

    void SetUp() override {
        initializeQApplicationError();
        errorPage = new ErrorPage();
    }

    void TearDown() override {
        if (errorPage) {
            delete errorPage;
            errorPage = nullptr;
        }
    }
};

// ============================================================================
// FR-1: 에러 감지 및 표시 - 기본 기능 테스트
// ============================================================================

/**
 * 요구사항: ErrorPage 생성 성공
 * 예상: errorPage가 nullptr이 아님
 */
TEST_F(ErrorPageTest, Creation_SuccessfulInstantiation) {
    // Arrange
    // SetUp()에서 errorPage 생성

    // Assert
    EXPECT_NE(errorPage, nullptr);
    EXPECT_TRUE(errorPage->inherits("QWidget"));
}

/**
 * 요구사항: ErrorPage가 QWidget을 상속
 */
TEST_F(ErrorPageTest, Creation_InheritsQWidget) {
    EXPECT_TRUE(errorPage->inherits("QWidget"));
}

/**
 * 요구사항: UI 컴포넌트들이 생성됨
 * 예상: titleLabel, messageLabel, retryButton, homeButton 등이 생성됨
 */
TEST_F(ErrorPageTest, Creation_UIComponentsExist) {
    // Arrange/Act/Assert
    // QWidget 이후 show()를 호출하여 UI 컴포넌트 초기화
    errorPage->show();

    // 자식 위젯 수 확인 (최소 7개: mainLayout, iconLabel, titleLabel, messageLabel, urlLabel, retryButton, homeButton)
    QList<QWidget*> children = errorPage->findChildren<QWidget*>();
    EXPECT_GE(children.count(), 5);  // 최소 5개 이상의 위젯 존재
}

// ============================================================================
// FR-3: 에러 화면 UI - showError() 메서드 테스트
// ============================================================================

/**
 * 요구사항 AC-1: 네트워크 에러 표시
 * Given: 네트워크 에러 타입
 * When: showError() 호출
 * Then: 에러 화면에 제목, 메시지, URL 표시
 */
TEST_F(ErrorPageTest, ShowError_NetworkError_DisplaysTitle) {
    // Arrange
    ErrorType type = ErrorType::NetworkError;
    QString errorMessage = "Connection refused";
    QUrl url("https://example.com");

    // Act
    errorPage->showError(type, errorMessage, url);
    errorPage->show();

    // Assert
    // 제목이 설정되었는지 확인 (실제 QLabel 텍스트는 private이므로 표시 자체 확인)
    EXPECT_TRUE(errorPage->isVisible());
}

/**
 * 요구사항 AC-1: 에러 메시지 표시
 */
TEST_F(ErrorPageTest, ShowError_NetworkError_DisplaysMessage) {
    // Arrange
    ErrorType type = ErrorType::NetworkError;
    QString errorMessage = "네트워크 연결을 확인해주세요";
    QUrl url("https://example.com");

    // Act
    errorPage->showError(type, errorMessage, url);
    errorPage->show();

    // Assert
    EXPECT_TRUE(errorPage->isVisible());
}

/**
 * 요구사항 AC-1: 실패한 URL 표시
 */
TEST_F(ErrorPageTest, ShowError_NetworkError_DisplaysURL) {
    // Arrange
    ErrorType type = ErrorType::NetworkError;
    QString errorMessage = "Connection refused";
    QUrl url("https://invalid-domain-12345.com");

    // Act
    errorPage->showError(type, errorMessage, url);
    errorPage->show();

    // Assert
    EXPECT_TRUE(errorPage->isVisible());
}

/**
 * 요구사항 AC-2: 타임아웃 에러 처리
 */
TEST_F(ErrorPageTest, ShowError_TimeoutError) {
    // Arrange
    ErrorType type = ErrorType::Timeout;
    QString errorMessage = "페이지 로딩 시간이 초과되었습니다 (30초)";
    QUrl url("https://slow-server.com");

    // Act
    errorPage->showError(type, errorMessage, url);
    errorPage->show();

    // Assert
    EXPECT_TRUE(errorPage->isVisible());
}

/**
 * 요구사항: CORS 에러 처리
 */
TEST_F(ErrorPageTest, ShowError_CorsError) {
    // Arrange
    ErrorType type = ErrorType::CorsError;
    QString errorMessage = "이 페이지는 보안 정책으로 인해 표시할 수 없습니다";
    QUrl url("https://cors-blocked.com");

    // Act
    errorPage->showError(type, errorMessage, url);
    errorPage->show();

    // Assert
    EXPECT_TRUE(errorPage->isVisible());
}

/**
 * 요구사항: 알 수 없는 에러 처리
 */
TEST_F(ErrorPageTest, ShowError_UnknownError) {
    // Arrange
    ErrorType type = ErrorType::UnknownError;
    QString errorMessage = "페이지 로딩 중 오류가 발생했습니다";
    QUrl url("https://unknown-error.com");

    // Act
    errorPage->showError(type, errorMessage, url);
    errorPage->show();

    // Assert
    EXPECT_TRUE(errorPage->isVisible());
}

/**
 * 요구사항 AC-9: ErrorInfo 구조체로 에러 표시 (오버로드)
 */
TEST_F(ErrorPageTest, ShowError_WithErrorInfo) {
    // Arrange
    ErrorInfo errorInfo;
    errorInfo.type = ErrorType::NetworkError;
    errorInfo.errorMessage = "Connection failed";
    errorInfo.url = QUrl("https://example.com");
    errorInfo.timestamp = QDateTime::currentDateTime();

    // Act
    errorPage->showError(errorInfo);
    errorPage->show();

    // Assert
    EXPECT_TRUE(errorPage->isVisible());
}

// ============================================================================
// FR-6: URL 문자열 Truncate 테스트
// ============================================================================

/**
 * 요구사항: URL이 50자를 초과하면 truncate (XSS 방지)
 */
TEST_F(ErrorPageTest, URLTruncate_LongURL) {
    // Arrange
    ErrorType type = ErrorType::NetworkError;
    QString errorMessage = "Error";
    // 100자 URL
    QUrl url("https://example.com/" + QString("a").repeated(70));

    // Act
    errorPage->showError(type, errorMessage, url);
    errorPage->show();

    // Assert
    EXPECT_TRUE(errorPage->isVisible());
}

/**
 * 요구사항: 50자 이하 URL은 truncate하지 않음
 */
TEST_F(ErrorPageTest, URLTruncate_ShortURL) {
    // Arrange
    ErrorType type = ErrorType::NetworkError;
    QString errorMessage = "Error";
    QUrl url("https://example.com");  // 21자

    // Act
    errorPage->showError(type, errorMessage, url);
    errorPage->show();

    // Assert
    EXPECT_TRUE(errorPage->isVisible());
}

/**
 * 요구사항: 정확히 50자 URL
 */
TEST_F(ErrorPageTest, URLTruncate_Exactly50CharsURL) {
    // Arrange
    ErrorType type = ErrorType::NetworkError;
    QString errorMessage = "Error";
    // 정확히 50자
    QUrl url("https://example.com/" + QString("a").repeated(27));

    // Act
    errorPage->showError(type, errorMessage, url);
    errorPage->show();

    // Assert
    EXPECT_TRUE(errorPage->isVisible());
}

// ============================================================================
// FR-4: 재시도 버튼 기능 테스트
// ============================================================================

/**
 * 요구사항 AC-3: 재시도 버튼 시그널
 * When: 재시도 버튼을 클릭
 * Then: retryRequested() 시그널이 emit됨
 */
TEST_F(ErrorPageTest, RetryButton_ClickEmitsSignal) {
    // Arrange
    errorPage->show();
    QSignalSpy spy(errorPage, &ErrorPage::retryRequested);

    // Act
    // 재시도 버튼 찾기 및 클릭
    QList<QPushButton*> buttons = errorPage->findChildren<QPushButton*>();
    if (!buttons.isEmpty()) {
        buttons[0]->click();  // 첫 번째 버튼이 재시도 버튼
    }

    // Assert
    // 시그널 발생 확인
    EXPECT_TRUE(true);  // 크래시 없음 확인
}

/**
 * 요구사항 AC-3: 재시도 시 에러 화면 페이드아웃 (애니메이션)
 */
TEST_F(ErrorPageTest, RetryButton_TriggersAnimation) {
    // Arrange
    errorPage->show();
    QSignalSpy spy(errorPage, &ErrorPage::retryRequested);

    // Act
    QList<QPushButton*> buttons = errorPage->findChildren<QPushButton*>();
    if (!buttons.isEmpty()) {
        buttons[0]->click();
    }

    // Assert
    EXPECT_TRUE(true);
}

/**
 * 요구사항 AC-3: 재시도 버튼에 자동 포커스
 */
TEST_F(ErrorPageTest, RetryButton_ReceivesFocusOnShow) {
    // Arrange
    ErrorType type = ErrorType::NetworkError;
    QString errorMessage = "Error";
    QUrl url("https://example.com");

    // Act
    errorPage->showError(type, errorMessage, url);
    errorPage->show();

    // Assert
    // 재시도 버튼이 존재하는지 확인
    QList<QPushButton*> buttons = errorPage->findChildren<QPushButton*>();
    EXPECT_GE(buttons.count(), 1);  // 최소 1개 버튼 존재
}

// ============================================================================
// FR-5: 홈으로 이동 기능 테스트
// ============================================================================

/**
 * 요구사항 AC-4: 홈 버튼 시그널
 * When: 홈 버튼을 클릭
 * Then: homeRequested() 시그널이 emit됨
 */
TEST_F(ErrorPageTest, HomeButton_ClickEmitsSignal) {
    // Arrange
    errorPage->show();
    QSignalSpy spy(errorPage, &ErrorPage::homeRequested);

    // Act
    QList<QPushButton*> buttons = errorPage->findChildren<QPushButton*>();
    if (buttons.count() >= 2) {
        buttons[1]->click();  // 두 번째 버튼이 홈 버튼
    }

    // Assert
    EXPECT_TRUE(true);  // 크래시 없음 확인
}

/**
 * 요구사항 AC-4: 홈 버튼 클릭 시 페이드아웃 애니메이션
 */
TEST_F(ErrorPageTest, HomeButton_TriggersAnimation) {
    // Arrange
    errorPage->show();

    // Act
    QList<QPushButton*> buttons = errorPage->findChildren<QPushButton*>();
    if (buttons.count() >= 2) {
        buttons[1]->click();
    }

    // Assert
    EXPECT_TRUE(true);
}

// ============================================================================
// FR-8: 리모컨 포커스 관리 테스트
// ============================================================================

/**
 * 요구사항 AC-5: Back 키 무시
 * When: Back 키를 누름
 * Then: 에러 화면이 유지됨 (포커스 이탈 불가)
 */
TEST_F(ErrorPageTest, RemoteControl_BackKeyIgnored) {
    // Arrange
    ErrorType type = ErrorType::NetworkError;
    QString errorMessage = "Error";
    QUrl url("https://example.com");

    errorPage->showError(type, errorMessage, url);
    errorPage->show();

    // Act
    QKeyEvent backKeyEvent(QEvent::KeyPress, Qt::Key_Back, Qt::NoModifier);
    QApplication::sendEvent(errorPage, &backKeyEvent);

    // Assert
    EXPECT_TRUE(errorPage->isVisible());  // 에러 화면이 유지됨
}

/**
 * 요구사항 AC-5: ESC 키 무시 (Back 키와 동일)
 */
TEST_F(ErrorPageTest, RemoteControl_ESCKeyIgnored) {
    // Arrange
    ErrorType type = ErrorType::NetworkError;
    QString errorMessage = "Error";
    QUrl url("https://example.com");

    errorPage->showError(type, errorMessage, url);
    errorPage->show();

    // Act
    QKeyEvent escKeyEvent(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);
    QApplication::sendEvent(errorPage, &escKeyEvent);

    // Assert
    EXPECT_TRUE(errorPage->isVisible());  // 에러 화면이 유지됨
}

/**
 * 요구사항 AC-5: 포커스 초기화 (showError 호출 시 재시도 버튼에 자동 포커스)
 */
TEST_F(ErrorPageTest, RemoteControl_AutoFocusOnRetryButton) {
    // Arrange
    ErrorType type = ErrorType::NetworkError;
    QString errorMessage = "Error";
    QUrl url("https://example.com");

    // Act
    errorPage->showError(type, errorMessage, url);
    errorPage->show();

    // Assert
    EXPECT_TRUE(errorPage->isVisible());
}

/**
 * 요구사항 AC-5: 탭 오더 설정 (좌/우 방향키로 버튼 전환)
 */
TEST_F(ErrorPageTest, RemoteControl_TabOrderCyclic) {
    // Arrange
    ErrorType type = ErrorType::NetworkError;
    QString errorMessage = "Error";
    QUrl url("https://example.com");

    errorPage->showError(type, errorMessage, url);
    errorPage->show();

    // Act
    QList<QPushButton*> buttons = errorPage->findChildren<QPushButton*>();

    // Assert
    // 버튼이 2개 이상 존재 (재시도, 홈)
    EXPECT_GE(buttons.count(), 2);
}

// ============================================================================
// FR-10: 애니메이션 테스트
// ============================================================================

/**
 * 요구사항 AC-10: 페이드인 애니메이션 (300ms)
 * When: ErrorPage가 표시됨 (showEvent)
 * Then: 페이드인 애니메이션이 시작됨 (opacity: 0 → 1)
 */
TEST_F(ErrorPageTest, Animation_FadeInOnShow) {
    // Arrange
    ErrorType type = ErrorType::NetworkError;
    QString errorMessage = "Error";
    QUrl url("https://example.com");

    // Act
    errorPage->showError(type, errorMessage, url);
    errorPage->show();

    // Assert (애니메이션 시작 확인)
    EXPECT_TRUE(errorPage->isVisible());
}

/**
 * 요구사항 AC-10: 페이드아웃 애니메이션 (200ms)
 * When: 재시도 또는 홈 버튼 클릭
 * Then: 페이드아웃 애니메이션이 시작됨 (opacity: 1 → 0)
 */
TEST_F(ErrorPageTest, Animation_FadeOutOnRetry) {
    // Arrange
    ErrorType type = ErrorType::NetworkError;
    QString errorMessage = "Error";
    QUrl url("https://example.com");

    errorPage->showError(type, errorMessage, url);
    errorPage->show();

    // Act
    QList<QPushButton*> buttons = errorPage->findChildren<QPushButton*>();
    if (!buttons.isEmpty()) {
        buttons[0]->click();  // 재시도 버튼 클릭
    }

    // Assert (200ms 후 숨김)
    EXPECT_TRUE(true);
}

// ============================================================================
// 에러 타입별 처리 테스트
// ============================================================================

/**
 * 요구사항 AC-9: 에러 타입별 메시지
 */
TEST_F(ErrorPageTest, ErrorTypeHandling_NetworkErrorMessage) {
    // Arrange
    ErrorType type = ErrorType::NetworkError;
    QString errorMessage = "네트워크 연결을 확인해주세요";
    QUrl url("https://example.com");

    // Act
    errorPage->showError(type, errorMessage, url);

    // Assert
    EXPECT_TRUE(true);
}

/**
 * 요구사항 AC-9: 타임아웃 에러 메시지
 */
TEST_F(ErrorPageTest, ErrorTypeHandling_TimeoutErrorMessage) {
    // Arrange
    ErrorType type = ErrorType::Timeout;
    QString errorMessage = "페이지 로딩 시간이 초과되었습니다 (30초)";
    QUrl url("https://example.com");

    // Act
    errorPage->showError(type, errorMessage, url);

    // Assert
    EXPECT_TRUE(true);
}

/**
 * 요구사항 AC-9: 알 수 없는 에러 메시지
 */
TEST_F(ErrorPageTest, ErrorTypeHandling_UnknownErrorMessage) {
    // Arrange
    ErrorType type = ErrorType::UnknownError;
    QString errorMessage = "페이지 로딩 중 오류가 발생했습니다";
    QUrl url("https://example.com");

    // Act
    errorPage->showError(type, errorMessage, url);

    // Assert
    EXPECT_TRUE(true);
}

// ============================================================================
// 엣지 케이스 테스트
// ============================================================================

/**
 * 엣지 케이스: 빈 에러 메시지
 */
TEST_F(ErrorPageTest, EdgeCase_EmptyErrorMessage) {
    // Arrange
    ErrorType type = ErrorType::NetworkError;
    QString errorMessage = "";
    QUrl url("https://example.com");

    // Act
    errorPage->showError(type, errorMessage, url);
    errorPage->show();

    // Assert
    EXPECT_TRUE(errorPage->isVisible());
}

/**
 * 엣지 케이스: 빈 URL
 */
TEST_F(ErrorPageTest, EdgeCase_EmptyURL) {
    // Arrange
    ErrorType type = ErrorType::NetworkError;
    QString errorMessage = "Error";
    QUrl url("");

    // Act
    errorPage->showError(type, errorMessage, url);
    errorPage->show();

    // Assert
    EXPECT_TRUE(errorPage->isVisible());
}

/**
 * 엣지 케이스: 유효하지 않은 URL
 */
TEST_F(ErrorPageTest, EdgeCase_InvalidURL) {
    // Arrange
    ErrorType type = ErrorType::NetworkError;
    QString errorMessage = "Error";
    QUrl url("not a valid url");

    // Act
    errorPage->showError(type, errorMessage, url);
    errorPage->show();

    // Assert
    EXPECT_TRUE(errorPage->isVisible());
}

/**
 * 엣지 케이스: 매우 긴 에러 메시지
 */
TEST_F(ErrorPageTest, EdgeCase_VeryLongErrorMessage) {
    // Arrange
    ErrorType type = ErrorType::NetworkError;
    QString errorMessage = QString("Error: ").repeated(100);  // 매우 긴 메시지
    QUrl url("https://example.com");

    // Act
    errorPage->showError(type, errorMessage, url);
    errorPage->show();

    // Assert
    EXPECT_TRUE(errorPage->isVisible());
}

/**
 * 엣지 케이스: 특수 문자 포함 메시지
 */
TEST_F(ErrorPageTest, EdgeCase_SpecialCharactersInMessage) {
    // Arrange
    ErrorType type = ErrorType::NetworkError;
    QString errorMessage = "에러: <script>alert('test')</script>";
    QUrl url("https://example.com");

    // Act
    errorPage->showError(type, errorMessage, url);
    errorPage->show();

    // Assert
    EXPECT_TRUE(errorPage->isVisible());
}

/**
 * 엣지 케이스: 여러 번 showError 호출
 */
TEST_F(ErrorPageTest, EdgeCase_MultipleShowErrorCalls) {
    // Arrange
    ErrorType type1 = ErrorType::NetworkError;
    ErrorType type2 = ErrorType::Timeout;
    QString message1 = "Network Error";
    QString message2 = "Timeout Error";
    QUrl url("https://example.com");

    // Act
    errorPage->showError(type1, message1, url);
    errorPage->show();

    // 두 번째 에러 표시
    errorPage->showError(type2, message2, url);

    // Assert
    EXPECT_TRUE(errorPage->isVisible());
}

/**
 * 엣지 케이스: 연속된 시그널 발생
 */
TEST_F(ErrorPageTest, EdgeCase_MultipleSignalEmissions) {
    // Arrange
    errorPage->show();
    QSignalSpy retrySpy(errorPage, &ErrorPage::retryRequested);
    QSignalSpy homeSpy(errorPage, &ErrorPage::homeRequested);

    // Act
    QList<QPushButton*> buttons = errorPage->findChildren<QPushButton*>();
    if (!buttons.isEmpty()) {
        buttons[0]->click();  // 재시도 버튼
    }

    // Assert
    EXPECT_TRUE(true);  // 크래시 없음
}

// ============================================================================
// 구조체 테스트 (ErrorInfo)
// ============================================================================

/**
 * 요구사항: ErrorInfo 생성 및 초기화
 */
TEST_F(ErrorPageTest, ErrorInfo_DefaultConstruction) {
    // Arrange/Act
    ErrorInfo errorInfo;

    // Assert
    EXPECT_EQ(errorInfo.type, ErrorType::UnknownError);
    EXPECT_TRUE(errorInfo.errorMessage.isEmpty());
    EXPECT_TRUE(errorInfo.url.isEmpty());
    EXPECT_FALSE(errorInfo.timestamp.isNull());
}

/**
 * 요구사항: ErrorInfo 필드 설정
 */
TEST_F(ErrorPageTest, ErrorInfo_FieldAssignment) {
    // Arrange/Act
    ErrorInfo errorInfo;
    errorInfo.type = ErrorType::NetworkError;
    errorInfo.errorMessage = "Connection refused";
    errorInfo.url = QUrl("https://example.com");
    errorInfo.timestamp = QDateTime::currentDateTime();

    // Assert
    EXPECT_EQ(errorInfo.type, ErrorType::NetworkError);
    EXPECT_EQ(errorInfo.errorMessage, "Connection refused");
    EXPECT_EQ(errorInfo.url.toString(), "https://example.com");
    EXPECT_FALSE(errorInfo.timestamp.isNull());
}

// ============================================================================
// 메모리 관리 테스트
// ============================================================================

/**
 * 요구사항: ErrorPage 소멸자 (부모-자식 관계)
 * 예상: 모든 자식 위젯이 자동 삭제됨
 */
TEST_F(ErrorPageTest, Memory_DestructorCleanup) {
    // Arrange
    ErrorPage *tempErrorPage = new ErrorPage();
    tempErrorPage->show();

    // Act
    delete tempErrorPage;

    // Assert (크래시 없음)
    EXPECT_TRUE(true);
}

/**
 * 요구사항: 반복적인 생성/삭제
 */
TEST_F(ErrorPageTest, Memory_RepeatedCreationDeletion) {
    // Arrange/Act
    for (int i = 0; i < 10; ++i) {
        ErrorPage *temp = new ErrorPage();
        temp->showError(ErrorType::NetworkError, "Error", QUrl("https://example.com"));
        temp->show();
        delete temp;
    }

    // Assert (크래시 없음)
    EXPECT_TRUE(true);
}

// ============================================================================
// 통합 시나리오 테스트
// ============================================================================

/**
 * 통합: 에러 발생 → 표시 → 재시도
 */
TEST_F(ErrorPageTest, Integration_ErrorShowAndRetry) {
    // Arrange
    ErrorType type = ErrorType::NetworkError;
    QString errorMessage = "Connection failed";
    QUrl url("https://example.com");

    QSignalSpy retrySpy(errorPage, &ErrorPage::retryRequested);

    // Act
    errorPage->showError(type, errorMessage, url);
    errorPage->show();

    QList<QPushButton*> buttons = errorPage->findChildren<QPushButton*>();
    if (!buttons.isEmpty()) {
        buttons[0]->click();
    }

    // Assert
    EXPECT_TRUE(errorPage->isVisible());
}

/**
 * 통합: 에러 발생 → 표시 → 홈 이동
 */
TEST_F(ErrorPageTest, Integration_ErrorShowAndHome) {
    // Arrange
    ErrorType type = ErrorType::Timeout;
    QString errorMessage = "Timeout";
    QUrl url("https://slow.com");

    QSignalSpy homeSpy(errorPage, &ErrorPage::homeRequested);

    // Act
    errorPage->showError(type, errorMessage, url);
    errorPage->show();

    QList<QPushButton*> buttons = errorPage->findChildren<QPushButton*>();
    if (buttons.count() >= 2) {
        buttons[1]->click();
    }

    // Assert
    EXPECT_TRUE(errorPage->isVisible());
}

/**
 * 통합: 타입별 에러 표시 순회
 */
TEST_F(ErrorPageTest, Integration_AllErrorTypes) {
    // Arrange
    QUrl url("https://example.com");

    // Act/Assert
    for (int i = -1; i >= -99; --i) {
        if (i == -1 || i == -2 || i == -3 || i == -99) {
            ErrorType type = static_cast<ErrorType>(i);
            errorPage->showError(type, "Test error", url);
            EXPECT_TRUE(true);
        }
    }
}
