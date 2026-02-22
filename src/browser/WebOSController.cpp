/**
 * @file WebOSController.cpp
 * @brief QML용 webOS 브라우저 컨트롤러 구현
 *
 * luna-send 명령으로 webOS ApplicationManager를 통해 시스템 브라우저를 실행합니다.
 * URL 히스토리 관리(뒤로/앞으로 가기)도 자체적으로 처리합니다.
 */

#include "WebOSController.h"
#include <QDebug>
#include <QUrl>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonObject>

namespace webosbrowser {

WebOSController::WebOSController(QObject *parent)
    : QObject(parent)
    , m_currentUrl("")
    , m_pageTitle("webOS 브라우저")
    , m_errorMessage("")
    , m_isLoading(false)
    , m_historyIndex(-1)
    , m_process(nullptr)
{
    qDebug() << "[WebOSController] 초기화 완료";
}

WebOSController::~WebOSController()
{
    if (m_process) {
        m_process->kill();
        m_process->waitForFinished(1000);
        m_process->deleteLater();
    }
}

// ─────────────────────────────────────────────────────────────
// 프로퍼티 게터
// ─────────────────────────────────────────────────────────────

QString WebOSController::currentUrl() const
{
    return m_currentUrl;
}

bool WebOSController::isLoading() const
{
    return m_isLoading;
}

QString WebOSController::pageTitle() const
{
    return m_pageTitle;
}

bool WebOSController::canGoBack() const
{
    return m_historyIndex > 0;
}

bool WebOSController::canGoForward() const
{
    return m_historyIndex < m_urlHistory.size() - 1;
}

QString WebOSController::errorMessage() const
{
    return m_errorMessage;
}

// ─────────────────────────────────────────────────────────────
// Q_INVOKABLE 메서드
// ─────────────────────────────────────────────────────────────

void WebOSController::navigateTo(const QString& url)
{
    if (url.isEmpty()) {
        qDebug() << "[WebOSController] 빈 URL 입력 무시";
        return;
    }

    // URL 정규화
    QString normalizedUrl = normalizeUrl(url);
    if (normalizedUrl.isEmpty()) {
        m_errorMessage = "유효하지 않은 URL입니다: " + url;
        emit errorMessageChanged(m_errorMessage);
        return;
    }

    qDebug() << "[WebOSController] 탐색 요청:" << normalizedUrl;

    // URL 히스토리 업데이트
    pushToHistory(normalizedUrl);

    // 현재 URL 업데이트
    m_currentUrl = normalizedUrl;
    emit currentUrlChanged(m_currentUrl);

    // 페이지 제목 업데이트 (URL 기반 임시 제목)
    QUrl parsedUrl(normalizedUrl);
    m_pageTitle = parsedUrl.host().isEmpty() ? normalizedUrl : parsedUrl.host();
    emit titleChanged(m_pageTitle);

    // 로딩 상태 시작
    m_isLoading = true;
    emit isLoadingChanged(m_isLoading);

    // 에러 초기화
    m_errorMessage = "";
    emit errorMessageChanged(m_errorMessage);

    // luna-send로 시스템 브라우저 실행
    launchBrowserViaLuna(normalizedUrl);
}

void WebOSController::goBack()
{
    if (!canGoBack()) {
        qDebug() << "[WebOSController] 뒤로 가기 불가 (히스토리 없음)";
        return;
    }

    m_historyIndex--;
    QString url = m_urlHistory.at(m_historyIndex);
    qDebug() << "[WebOSController] 뒤로 가기:" << url;

    m_currentUrl = url;
    emit currentUrlChanged(m_currentUrl);
    emit historyChanged();

    // 실제 브라우저로 이전 URL 전송
    launchBrowserViaLuna(url);
}

void WebOSController::goForward()
{
    if (!canGoForward()) {
        qDebug() << "[WebOSController] 앞으로 가기 불가 (히스토리 없음)";
        return;
    }

    m_historyIndex++;
    QString url = m_urlHistory.at(m_historyIndex);
    qDebug() << "[WebOSController] 앞으로 가기:" << url;

    m_currentUrl = url;
    emit currentUrlChanged(m_currentUrl);
    emit historyChanged();

    launchBrowserViaLuna(url);
}

void WebOSController::reload()
{
    if (m_currentUrl.isEmpty()) {
        qDebug() << "[WebOSController] 새로고침 불가 (현재 URL 없음)";
        return;
    }

    qDebug() << "[WebOSController] 새로고침:" << m_currentUrl;
    launchBrowserViaLuna(m_currentUrl);
}

void WebOSController::stop()
{
    if (m_process && m_process->state() != QProcess::NotRunning) {
        m_process->kill();
        qDebug() << "[WebOSController] 프로세스 중지";
    }

    m_isLoading = false;
    emit isLoadingChanged(m_isLoading);
}

QString WebOSController::getSecurityLevel(const QString& url) const
{
    if (url.startsWith("https://")) {
        return "secure";
    } else if (url.startsWith("http://")) {
        return "insecure";
    }
    return "unknown";
}

QString WebOSController::normalizeUrl(const QString& input) const
{
    if (input.isEmpty()) {
        return QString();
    }

    QString trimmed = input.trimmed();

    // 이미 http:// 또는 https:// 가 있으면 그대로 사용
    if (trimmed.startsWith("http://") || trimmed.startsWith("https://")) {
        return trimmed;
    }

    // 도메인처럼 보이는 경우 (점이 있고 공백이 없음)
    if (!trimmed.contains(' ') && trimmed.contains('.') && !trimmed.startsWith('.')) {
        return "https://" + trimmed;
    }

    // 검색어로 처리 → Google 검색 URL 생성
    QString encodedQuery = QUrl::toPercentEncoding(trimmed);
    return "https://www.google.com/search?q=" + encodedQuery;
}

void WebOSController::clearError()
{
    m_errorMessage = "";
    emit errorMessageChanged(m_errorMessage);
}

// ─────────────────────────────────────────────────────────────
// Private 메서드
// ─────────────────────────────────────────────────────────────

void WebOSController::launchBrowserViaLuna(const QString& url)
{
    // 기존 프로세스 정리
    if (m_process) {
        if (m_process->state() != QProcess::NotRunning) {
            m_process->kill();
            m_process->waitForFinished(500);
        }
        m_process->deleteLater();
        m_process = nullptr;
    }

#ifdef USE_WEBOS_TARGET
    // webOS 타겟: luna-send로 시스템 브라우저 실행
    m_process = new QProcess(this);

    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &WebOSController::onProcessFinished);
    connect(m_process, &QProcess::errorOccurred,
            this, &WebOSController::onProcessError);

    // JSON 파라미터 구성
    QJsonObject params;
    params["id"] = "com.webos.app.browser";
    QJsonObject browserParams;
    browserParams["target"] = url;
    params["params"] = browserParams;

    QString jsonStr = QJsonDocument(params).toJson(QJsonDocument::Compact);

    // luna-send 명령 실행
    QString program = "/usr/bin/luna-send";
    QStringList args;
    args << "-n" << "1"
         << "luna://com.webos.service.applicationmanager/launch"
         << jsonStr;

    qDebug() << "[WebOSController] luna-send 실행:" << program << args.join(' ');
    m_process->start(program, args);

    if (!m_process->waitForStarted(3000)) {
        qWarning() << "[WebOSController] luna-send 프로세스 시작 실패";
        m_errorMessage = "luna-send 실행 실패: 프로세스를 시작할 수 없습니다";
        emit errorMessageChanged(m_errorMessage);
        m_isLoading = false;
        emit isLoadingChanged(m_isLoading);
        emit pageOpenFailed(m_errorMessage);
    }
#else
    // macOS/개발 환경: 디버그 로그만 출력
    qDebug() << "[WebOSController] [개발 모드] 브라우저 실행 요청 URL:" << url;
    qDebug() << "[WebOSController] webOS 타겟에서는 다음 명령이 실행됩니다:";
    qDebug() << "  /usr/bin/luna-send -n 1 luna://com.webos.service.applicationmanager/launch";
    qDebug() << "  '{\"id\":\"com.webos.app.browser\",\"params\":{\"target\":\"" << url << "\"}}'";

    // 개발 환경에서는 바로 성공 처리
    m_isLoading = false;
    emit isLoadingChanged(m_isLoading);
    emit pageOpened(url);
#endif
}

void WebOSController::pushToHistory(const QString& url)
{
    // 현재 위치 이후의 앞으로 가기 히스토리 제거
    if (m_historyIndex < m_urlHistory.size() - 1) {
        m_urlHistory = m_urlHistory.mid(0, m_historyIndex + 1);
    }

    // 동일 URL 연속 추가 방지
    if (!m_urlHistory.isEmpty() && m_urlHistory.last() == url) {
        return;
    }

    m_urlHistory.append(url);
    m_historyIndex = m_urlHistory.size() - 1;

    // 최대 히스토리 크기 제한 (50개)
    if (m_urlHistory.size() > 50) {
        m_urlHistory.removeFirst();
        m_historyIndex--;
    }

    emit historyChanged();
}

// ─────────────────────────────────────────────────────────────
// Private Slots
// ─────────────────────────────────────────────────────────────

void WebOSController::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    m_isLoading = false;
    emit isLoadingChanged(m_isLoading);

    if (exitStatus == QProcess::NormalExit && exitCode == 0) {
        qDebug() << "[WebOSController] luna-send 성공 (exitCode=0)";
        emit pageOpened(m_currentUrl);
    } else {
        QString stderr_output;
        if (m_process) {
            stderr_output = m_process->readAllStandardError();
        }
        qWarning() << "[WebOSController] luna-send 실패 (exitCode=" << exitCode << "):" << stderr_output;
        m_errorMessage = QString("브라우저 실행 실패 (코드: %1). %2").arg(exitCode).arg(stderr_output);
        emit errorMessageChanged(m_errorMessage);
        emit pageOpenFailed(m_errorMessage);
    }
}

void WebOSController::onProcessError(QProcess::ProcessError error)
{
    m_isLoading = false;
    emit isLoadingChanged(m_isLoading);

    QString errorStr;
    switch (error) {
        case QProcess::FailedToStart:
            errorStr = "luna-send 실행 파일을 찾을 수 없습니다 (/usr/bin/luna-send)";
            break;
        case QProcess::Crashed:
            errorStr = "luna-send 프로세스가 비정상 종료되었습니다";
            break;
        case QProcess::Timedout:
            errorStr = "luna-send 응답 타임아웃";
            break;
        default:
            errorStr = QString("luna-send 실행 오류 (코드: %1)").arg(static_cast<int>(error));
            break;
    }

    qWarning() << "[WebOSController]" << errorStr;
    m_errorMessage = errorStr;
    emit errorMessageChanged(m_errorMessage);
    emit pageOpenFailed(m_errorMessage);
}

} // namespace webosbrowser
