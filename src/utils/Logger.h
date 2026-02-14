/**
 * @file Logger.h
 * @brief 로거 유틸리티 헤더
 */

#pragma once

#include <QString>

namespace webosbrowser {

/**
 * @enum LogLevel
 * @brief 로그 레벨
 */
enum class LogLevel {
    Debug,      ///< 디버그 메시지
    Info,       ///< 정보 메시지
    Warning,    ///< 경고 메시지
    Error       ///< 에러 메시지
};

/**
 * @class Logger
 * @brief 간단한 로깅 유틸리티 클래스
 *
 * 정적 메서드를 사용하여 콘솔에 로그 메시지를 출력합니다.
 */
class Logger {
public:
    /**
     * @brief 디버그 로그 출력
     * @param message 메시지
     */
    static void debug(const QString &message);

    /**
     * @brief 정보 로그 출력
     * @param message 메시지
     */
    static void info(const QString &message);

    /**
     * @brief 경고 로그 출력
     * @param message 메시지
     */
    static void warning(const QString &message);

    /**
     * @brief 에러 로그 출력
     * @param message 메시지
     */
    static void error(const QString &message);

    /**
     * @brief 로그 출력 (레벨 지정)
     * @param level 로그 레벨
     * @param message 메시지
     */
    static void log(LogLevel level, const QString &message);

    /**
     * @brief 로그 레벨을 문자열로 변환
     * @param level 로그 레벨
     * @return 레벨 문자열
     */
    static QString levelToString(LogLevel level);

private:
    Logger() = default;
    ~Logger() = default;
};

} // namespace webosbrowser
