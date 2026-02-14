/**
 * @file Logger.cpp
 * @brief 로거 유틸리티 구현
 */

#include "Logger.h"
#include <QDateTime>
#include <QDebug>

namespace webosbrowser {

void Logger::debug(const QString &message) {
    log(LogLevel::Debug, message);
}

void Logger::info(const QString &message) {
    log(LogLevel::Info, message);
}

void Logger::warning(const QString &message) {
    log(LogLevel::Warning, message);
}

void Logger::error(const QString &message) {
    log(LogLevel::Error, message);
}

void Logger::log(LogLevel level, const QString &message) {
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz");
    QString levelStr = levelToString(level);
    QString logMessage = QString("[%1] [%2] %3").arg(timestamp, levelStr, message);

    // Qt 디버그 출력
    switch (level) {
        case LogLevel::Debug:
            qDebug().noquote() << logMessage;
            break;
        case LogLevel::Info:
            qInfo().noquote() << logMessage;
            break;
        case LogLevel::Warning:
            qWarning().noquote() << logMessage;
            break;
        case LogLevel::Error:
            qCritical().noquote() << logMessage;
            break;
    }
}

QString Logger::levelToString(LogLevel level) {
    switch (level) {
        case LogLevel::Debug:
            return "DEBUG";
        case LogLevel::Info:
            return "INFO";
        case LogLevel::Warning:
            return "WARN";
        case LogLevel::Error:
            return "ERROR";
        default:
            return "UNKNOWN";
    }
}

} // namespace webosbrowser
