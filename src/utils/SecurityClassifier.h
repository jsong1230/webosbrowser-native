/**
 * @file SecurityClassifier.h
 * @brief URL 보안 등급 분류 유틸리티
 */

#pragma once

#include <QString>
#include <QUrl>
#include <QRegularExpression>

namespace webosbrowser {

/**
 * @enum SecurityLevel
 * @brief URL 보안 등급
 */
enum class SecurityLevel {
    Secure,    ///< HTTPS 보안 연결
    Insecure,  ///< HTTP 비보안 연결 (localhost 제외)
    Local,     ///< localhost, file://, 사설 IP
    Unknown    ///< about:blank, 빈 URL, 기타 프로토콜
};

/**
 * @class SecurityClassifier
 * @brief URL 보안 등급 분류 유틸리티
 *
 * QUrl을 분석하여 SecurityLevel을 반환합니다.
 * 정적 메서드만 제공 (인스턴스 생성 불필요).
 */
class SecurityClassifier {
public:
    /**
     * @brief URL 보안 등급 분류
     * @param url 분류할 URL
     * @return SecurityLevel (Secure/Insecure/Local/Unknown)
     */
    static SecurityLevel classify(const QUrl &url);

    /**
     * @brief 로컬 주소 판별 (localhost, 사설 IP)
     * @param host 호스트명 또는 IP 주소
     * @return true면 로컬 주소
     */
    static bool isLocalAddress(const QString &host);

    /**
     * @brief SecurityLevel을 문자열로 변환 (디버깅용)
     * @param level SecurityLevel
     * @return "Secure", "Insecure", "Local", "Unknown"
     */
    static QString securityLevelToString(SecurityLevel level);

    /**
     * @brief SecurityLevel을 툴팁 텍스트로 변환
     * @param level SecurityLevel
     * @return "보안 연결", "안전하지 않음", "로컬 연결", ""
     */
    static QString securityLevelToTooltip(SecurityLevel level);

    /**
     * @brief SecurityLevel을 아이콘 파일 경로로 변환
     * @param level SecurityLevel
     * @return ":/icons/lock_secure.png", ":/icons/lock_insecure.png", etc.
     */
    static QString securityLevelToIconPath(SecurityLevel level);

private:
    // 생성자 삭제 (정적 클래스)
    SecurityClassifier() = delete;
};

} // namespace webosbrowser
