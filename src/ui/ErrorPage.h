/**
 * @file ErrorPage.h
 * @brief 에러 화면 위젯 (WebView 로딩 실패 시 표시)
 */

#pragma once

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QUrl>
#include <QDateTime>

namespace webosbrowser {

/**
 * @enum ErrorType
 * @brief 에러 타입 열거형
 */
enum class ErrorType {
    NetworkError = -1,   ///< 네트워크 연결 실패 (DNS, 연결 거부 등)
    Timeout = -2,        ///< 타임아웃 (30초 초과)
    CorsError = -3,      ///< CORS 정책 위반
    UnknownError = -99   ///< 알 수 없는 에러
};

/**
 * @struct ErrorInfo
 * @brief 에러 상세 정보
 */
struct ErrorInfo {
    ErrorType type;          ///< 에러 타입
    QString errorMessage;    ///< 사용자 표시용 메시지
    QUrl url;                ///< 실패한 URL
    QDateTime timestamp;     ///< 에러 발생 시각

    ErrorInfo()
        : type(ErrorType::UnknownError)
        , timestamp(QDateTime::currentDateTime())
    {}
};

/**
 * @class ErrorPage
 * @brief 에러 화면 위젯 (WebView 로딩 실패 시 표시)
 *
 * QWidget 기반, WebView와 QStackedLayout으로 전환 표시.
 * 리모컨 포커스 관리 및 재시도/홈 이동 기능 제공.
 */
class ErrorPage : public QWidget {
    Q_OBJECT

public:
    /**
     * @brief 생성자
     * @param parent 부모 위젯 (BrowserWindow)
     */
    explicit ErrorPage(QWidget *parent = nullptr);

    /**
     * @brief 소멸자
     */
    ~ErrorPage() override;

    // 복사 생성자 및 대입 연산자 삭제
    ErrorPage(const ErrorPage&) = delete;
    ErrorPage& operator=(const ErrorPage&) = delete;

    /**
     * @brief 에러 화면 표시
     * @param type 에러 타입
     * @param errorMessage 에러 메시지
     * @param url 실패한 URL
     */
    void showError(ErrorType type, const QString &errorMessage, const QUrl &url);

    /**
     * @brief 에러 정보로 화면 표시 (오버로드)
     * @param errorInfo 에러 정보 구조체
     */
    void showError(const ErrorInfo &errorInfo);

signals:
    /**
     * @brief 재시도 버튼 클릭 시그널
     */
    void retryRequested();

    /**
     * @brief 홈으로 이동 버튼 클릭 시그널
     */
    void homeRequested();

protected:
    /**
     * @brief 키 이벤트 핸들러 (리모컨 입력)
     * @param event 키 이벤트
     */
    void keyPressEvent(QKeyEvent *event) override;

    /**
     * @brief 표시 이벤트 핸들러 (페이드인 애니메이션)
     * @param event 표시 이벤트
     */
    void showEvent(QShowEvent *event) override;

private:
    /**
     * @brief UI 초기화
     */
    void setupUI();

    /**
     * @brief 스타일시트 적용 (QSS)
     */
    void applyStyles();

    /**
     * @brief 시그널/슬롯 연결
     */
    void setupConnections();

    /**
     * @brief 에러 타입에 따른 아이콘 경로 반환
     * @param type 에러 타입
     * @return 아이콘 파일 경로
     */
    QString getErrorIcon(ErrorType type) const;

    /**
     * @brief URL 문자열 truncate (최대 50자)
     * @param url URL
     * @return truncate된 URL 문자열
     */
    QString truncateUrl(const QUrl &url) const;

    /**
     * @brief 페이드인 애니메이션 시작
     */
    void startFadeInAnimation();

private:
    // UI 컴포넌트
    QVBoxLayout *mainLayout_;       ///< 메인 레이아웃
    QLabel *iconLabel_;             ///< 에러 아이콘
    QLabel *titleLabel_;            ///< 에러 제목
    QLabel *messageLabel_;          ///< 에러 메시지
    QLabel *urlLabel_;              ///< 실패한 URL
    QPushButton *retryButton_;      ///< 재시도 버튼
    QPushButton *homeButton_;       ///< 홈으로 이동 버튼

    // 현재 에러 정보
    ErrorInfo currentError_;        ///< 현재 표시 중인 에러 정보
};

} // namespace webosbrowser
