/**
 * @file URLValidator.h
 * @brief URL 검증 및 자동 보완 유틸리티
 */

#pragma once

#include <QString>
#include <QUrl>
#include <QRegularExpression>

namespace webosbrowser {

/**
 * @class URLValidator
 * @brief URL 검증, 자동 보완, 정규화 기능 제공
 *
 * 사용자 입력 URL을 검증하고, 필요시 http:// 프로토콜을 자동으로 추가합니다.
 */
class URLValidator {
public:
    /**
     * @brief URL 검증 결과
     */
    enum class ValidationResult {
        Valid,          ///< 유효한 URL
        InvalidFormat,  ///< 형식 오류
        MissingScheme,  ///< 프로토콜 누락 (자동 보완 가능)
        Empty           ///< 빈 문자열
    };

    /**
     * @brief URL 검증
     * @param input 사용자 입력 문자열
     * @return 검증 결과
     */
    static ValidationResult validate(const QString &input);

    /**
     * @brief URL 자동 보완 (http:// 추가)
     * @param input 사용자 입력 문자열
     * @return 보완된 URL 문자열
     */
    static QString normalize(const QString &input);

    /**
     * @brief URL 또는 검색어 판단
     * @param input 사용자 입력 문자열
     * @return true면 URL, false면 검색어
     */
    static bool isUrl(const QString &input);

    /**
     * @brief URL 형식 검증 (정규표현식)
     * @param input URL 문자열
     * @return 유효하면 true
     */
    static bool isValidUrlFormat(const QString &input);

    /**
     * @brief 검색어인지 판단
     * @param input 사용자 입력 문자열
     * @return true면 검색어, false면 URL로 간주
     */
    static bool isSearchQuery(const QString &input);

private:
    // URL 패턴 정규표현식
    static const QRegularExpression urlPattern_;
    static const QRegularExpression domainPattern_;
};

} // namespace webosbrowser
