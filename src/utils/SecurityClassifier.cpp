/**
 * @file SecurityClassifier.cpp
 * @brief URL 보안 등급 분류 유틸리티 구현
 */

#include "SecurityClassifier.h"
#include <QDebug>

namespace webosbrowser {

SecurityLevel SecurityClassifier::classify(const QUrl &url) {
    // 빈 URL 또는 유효하지 않은 URL
    if (url.isEmpty() || !url.isValid()) {
        return SecurityLevel::Unknown;
    }

    QString scheme = url.scheme().toLower();
    QString host = url.host().toLower();

    // HTTPS → Secure
    if (scheme == "https") {
        return SecurityLevel::Secure;
    }

    // HTTP → Insecure (단, localhost/사설IP는 Local)
    if (scheme == "http") {
        if (isLocalAddress(host)) {
            return SecurityLevel::Local;
        }
        return SecurityLevel::Insecure;
    }

    // file:// → Local
    if (scheme == "file") {
        return SecurityLevel::Local;
    }

    // about, qrc, chrome 등 → Unknown
    return SecurityLevel::Unknown;
}

bool SecurityClassifier::isLocalAddress(const QString &host) {
    // localhost, 127.0.0.1, ::1
    if (host == "localhost" || host == "127.0.0.1" || host == "::1") {
        return true;
    }

    // 사설 IP 정규표현식 체크

    // 10.0.0.0/8 (10.0.0.0 ~ 10.255.255.255)
    QRegularExpression ip10("^10\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}$");
    if (ip10.match(host).hasMatch()) {
        return true;
    }

    // 172.16.0.0/12 (172.16.0.0 ~ 172.31.255.255)
    QRegularExpression ip172("^172\\.(1[6-9]|2\\d|3[01])\\.\\d{1,3}\\.\\d{1,3}$");
    if (ip172.match(host).hasMatch()) {
        return true;
    }

    // 192.168.0.0/16 (192.168.0.0 ~ 192.168.255.255)
    QRegularExpression ip192("^192\\.168\\.\\d{1,3}\\.\\d{1,3}$");
    if (ip192.match(host).hasMatch()) {
        return true;
    }

    return false;
}

QString SecurityClassifier::securityLevelToString(SecurityLevel level) {
    switch (level) {
        case SecurityLevel::Secure:
            return "Secure";
        case SecurityLevel::Insecure:
            return "Insecure";
        case SecurityLevel::Local:
            return "Local";
        case SecurityLevel::Unknown:
            return "Unknown";
        default:
            return "Unknown";
    }
}

QString SecurityClassifier::securityLevelToTooltip(SecurityLevel level) {
    switch (level) {
        case SecurityLevel::Secure:
            return "보안 연결";
        case SecurityLevel::Insecure:
            return "안전하지 않음";
        case SecurityLevel::Local:
            return "로컬 연결";
        case SecurityLevel::Unknown:
            return "";
        default:
            return "";
    }
}

QString SecurityClassifier::securityLevelToIconPath(SecurityLevel level) {
    switch (level) {
        case SecurityLevel::Secure:
            return ":/icons/lock_secure.png";
        case SecurityLevel::Insecure:
            return ":/icons/lock_insecure.png";
        case SecurityLevel::Local:
            return ":/icons/info.png";
        case SecurityLevel::Unknown:
            return "";
        default:
            return "";
    }
}

} // namespace webosbrowser
