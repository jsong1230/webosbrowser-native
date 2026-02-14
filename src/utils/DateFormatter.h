/**
 * @file DateFormatter.h
 * @brief 날짜/시간 포맷팅 유틸리티 헤더
 */

#pragma once

#include <QString>
#include <QDateTime>

namespace webosbrowser {

/**
 * @class DateFormatter
 * @brief 날짜/시간을 사용자 친화적인 형식으로 변환하는 유틸리티
 */
class DateFormatter {
public:
    /**
     * @brief 상대 시간 문자열 반환 (예: "방금 전", "5분 전", "어제")
     * @param dateTime 기준 날짜/시간
     * @return 상대 시간 문자열
     */
    static QString toRelativeTime(const QDateTime &dateTime);

    /**
     * @brief 시간을 HH:mm 형식으로 반환
     * @param dateTime 날짜/시간
     * @return 시간 문자열 (예: "14:35")
     */
    static QString toTimeString(const QDateTime &dateTime);

    /**
     * @brief 날짜를 YYYY-MM-DD 형식으로 반환
     * @param dateTime 날짜/시간
     * @return 날짜 문자열 (예: "2026-02-14")
     */
    static QString toDateString(const QDateTime &dateTime);

    /**
     * @brief 날짜/시간을 YYYY-MM-DD HH:mm 형식으로 반환
     * @param dateTime 날짜/시간
     * @return 날짜/시간 문자열 (예: "2026-02-14 14:35")
     */
    static QString toDateTimeString(const QDateTime &dateTime);

    /**
     * @brief 날짜 그룹 이름 반환 (UI 표시용)
     * @param group 날짜 그룹 enum
     * @return 그룹 이름 (예: "오늘", "어제", "지난 7일")
     */
    static QString getDateGroupName(int group);

private:
    DateFormatter() = default;
    ~DateFormatter() = default;
};

} // namespace webosbrowser
