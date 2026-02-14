/**
 * @file URLValidator.cpp
 * @brief URL 검증 유틸리티 구현
 */

#include "URLValidator.h"
#include <QDebug>

namespace webosbrowser {

// 정규표현식 초기화
const QRegularExpression URLValidator::urlPattern_(
    R"(^(https?|ftp)://[a-zA-Z0-9\-._~:/?#\[\]@!$&'()*+,;=%]+$)",
    QRegularExpression::CaseInsensitiveOption
);

const QRegularExpression URLValidator::domainPattern_(
    R"(^(?:[a-zA-Z0-9](?:[a-zA-Z0-9-]{0,61}[a-zA-Z0-9])?\.)+[a-zA-Z]{2,}(?:/.*)?$)",
    QRegularExpression::CaseInsensitiveOption
);

URLValidator::ValidationResult URLValidator::validate(const QString &input) {
    const QString trimmed = input.trimmed();

    if (trimmed.isEmpty()) {
        return ValidationResult::Empty;
    }

    // 이미 프로토콜이 있는 경우
    if (trimmed.startsWith("http://") || trimmed.startsWith("https://") ||
        trimmed.startsWith("ftp://")) {
        QUrl url(trimmed);
        if (url.isValid() && !url.host().isEmpty()) {
            return ValidationResult::Valid;
        }
        return ValidationResult::InvalidFormat;
    }

    // 도메인 형식인지 확인
    if (domainPattern_.match(trimmed).hasMatch()) {
        return ValidationResult::MissingScheme;
    }

    return ValidationResult::InvalidFormat;
}

QString URLValidator::normalize(const QString &input) {
    const QString trimmed = input.trimmed();

    if (trimmed.isEmpty()) {
        return QString();
    }

    // 이미 프로토콜이 있으면 그대로 반환
    if (trimmed.startsWith("http://") || trimmed.startsWith("https://") ||
        trimmed.startsWith("ftp://")) {
        return trimmed;
    }

    // 도메인 형식이면 https:// 추가
    if (domainPattern_.match(trimmed).hasMatch()) {
        return "https://" + trimmed;
    }

    // 그 외는 그대로 반환 (검색어로 처리됨)
    return trimmed;
}

bool URLValidator::isUrl(const QString &input) {
    const QString trimmed = input.trimmed();

    if (trimmed.isEmpty()) {
        return false;
    }

    // 프로토콜이 있으면 URL
    if (trimmed.startsWith("http://") || trimmed.startsWith("https://") ||
        trimmed.startsWith("ftp://")) {
        return true;
    }

    // 도메인 형식이면 URL
    if (domainPattern_.match(trimmed).hasMatch()) {
        return true;
    }

    // localhost 또는 IP 주소
    if (trimmed.startsWith("localhost") ||
        QRegularExpression(R"(^\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3})").match(trimmed).hasMatch()) {
        return true;
    }

    return false;
}

bool URLValidator::isValidUrlFormat(const QString &input) {
    QString normalized = normalize(input);
    QUrl url(normalized);
    return url.isValid() && !url.host().isEmpty();
}

bool URLValidator::isSearchQuery(const QString &input) {
    const QString trimmed = input.trimmed();

    if (trimmed.isEmpty()) {
        return false;
    }

    // 1. 프로토콜 포함 시 URL로 판단
    if (trimmed.startsWith("http://") || trimmed.startsWith("https://") ||
        trimmed.startsWith("ftp://")) {
        return false;
    }

    // 2. 도메인 형식 → URL로 판단
    if (domainPattern_.match(trimmed).hasMatch()) {
        return false;
    }

    // 3. localhost 또는 IP 주소 → URL로 판단
    if (trimmed.startsWith("localhost") ||
        QRegularExpression(R"(^\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3})").match(trimmed).hasMatch()) {
        return false;
    }

    // 4. 위의 모든 URL 형식에 해당하지 않으면 검색어로 간주
    return true;
}

} // namespace webosbrowser
