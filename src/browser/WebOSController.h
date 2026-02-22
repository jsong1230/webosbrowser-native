/**
 * @file WebOSController.h
 * @brief QML용 webOS 브라우저 컨트롤러
 *
 * QML UI와 C++ 비즈니스 로직을 연결하는 컨트롤러.
 * luna-send 명령으로 webOS 시스템 브라우저에 URL을 전달합니다.
 */

#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QProcess>

namespace webosbrowser {

/**
 * @class WebOSController
 * @brief QML 인터페이스용 컨트롤러 클래스
 *
 * QML에서 C++ 로직에 접근할 수 있도록 Q_PROPERTY와 Q_INVOKABLE을 노출합니다.
 * webOS Luna Service API를 통해 시스템 브라우저를 실행합니다.
 */
class WebOSController : public QObject {
    Q_OBJECT

    Q_PROPERTY(QString currentUrl READ currentUrl NOTIFY currentUrlChanged)
    Q_PROPERTY(bool isLoading READ isLoading NOTIFY isLoadingChanged)
    Q_PROPERTY(QString pageTitle READ pageTitle NOTIFY titleChanged)
    Q_PROPERTY(bool canGoBack READ canGoBack NOTIFY historyChanged)
    Q_PROPERTY(bool canGoForward READ canGoForward NOTIFY historyChanged)
    Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY errorMessageChanged)

public:
    explicit WebOSController(QObject *parent = nullptr);
    ~WebOSController() override;

    WebOSController(const WebOSController&) = delete;
    WebOSController& operator=(const WebOSController&) = delete;

    // 프로퍼티 게터
    QString currentUrl() const;
    bool isLoading() const;
    QString pageTitle() const;
    bool canGoBack() const;
    bool canGoForward() const;
    QString errorMessage() const;

    /**
     * @brief URL 탐색 (luna-send로 시스템 브라우저 실행)
     * @param url 탐색할 URL 또는 검색어
     */
    Q_INVOKABLE void navigateTo(const QString& url);

    /**
     * @brief 뒤로 가기 (URL 히스토리 기반)
     */
    Q_INVOKABLE void goBack();

    /**
     * @brief 앞으로 가기 (URL 히스토리 기반)
     */
    Q_INVOKABLE void goForward();

    /**
     * @brief 현재 URL 새로고침
     */
    Q_INVOKABLE void reload();

    /**
     * @brief 로딩 중지
     */
    Q_INVOKABLE void stop();

    /**
     * @brief URL의 보안 등급 반환
     * @param url 확인할 URL
     * @return "secure" (HTTPS) / "insecure" (HTTP) / "unknown"
     */
    Q_INVOKABLE QString getSecurityLevel(const QString& url) const;

    /**
     * @brief URL 유효성 검증 및 정규화
     * @param input 사용자 입력 문자열
     * @return 정규화된 URL 또는 검색 URL
     */
    Q_INVOKABLE QString normalizeUrl(const QString& input) const;

    /**
     * @brief 에러 메시지 초기화
     */
    Q_INVOKABLE void clearError();

signals:
    void currentUrlChanged(const QString& url);
    void isLoadingChanged(bool loading);
    void titleChanged(const QString& title);
    void historyChanged();
    void errorMessageChanged(const QString& message);

    /**
     * @brief luna-send 성공 후 페이지가 열렸을 때 발생
     * @param url 열린 URL
     */
    void pageOpened(const QString& url);

    /**
     * @brief luna-send 실패 시 발생
     * @param errorMsg 에러 메시지
     */
    void pageOpenFailed(const QString& errorMsg);

private slots:
    /**
     * @brief QProcess 완료 처리
     */
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);

    /**
     * @brief QProcess 에러 처리
     */
    void onProcessError(QProcess::ProcessError error);

private:
    /**
     * @brief luna-send로 URL을 시스템 브라우저에 전달
     * @param url 전달할 URL
     */
    void launchBrowserViaLuna(const QString& url);

    /**
     * @brief 히스토리에 URL 추가
     * @param url 추가할 URL
     */
    void pushToHistory(const QString& url);

private:
    QString m_currentUrl;           ///< 현재 표시 중인 URL
    QString m_pageTitle;            ///< 현재 페이지 제목
    QString m_errorMessage;         ///< 마지막 에러 메시지
    bool m_isLoading;               ///< 로딩 상태
    QStringList m_urlHistory;       ///< URL 히스토리 (뒤로/앞으로 가기)
    int m_historyIndex;             ///< 현재 히스토리 위치
    QProcess* m_process;            ///< luna-send 프로세스
};

} // namespace webosbrowser
