/**
 * @file DateFormatter.cpp
 * @brief 날짜/시간 포맷팅 유틸리티 구현
 */

#include "DateFormatter.h"

namespace webosbrowser {

QString DateFormatter::toRelativeTime(const QDateTime &dateTime) {
    QDateTime now = QDateTime::currentDateTime();
    qint64 diffSeconds = dateTime.secsTo(now);

    if (diffSeconds < 0) {
        return "미래"; // 잘못된 시간
    }

    if (diffSeconds < 60) {
        return "방금 전";
    } else if (diffSeconds < 60 * 60) {
        int minutes = diffSeconds / 60;
        return QString("%1분 전").arg(minutes);
    } else if (diffSeconds < 24 * 60 * 60) {
        int hours = diffSeconds / (60 * 60);
        return QString("%1시간 전").arg(hours);
    } else if (diffSeconds < 2 * 24 * 60 * 60) {
        return "어제";
    } else if (diffSeconds < 7 * 24 * 60 * 60) {
        int days = diffSeconds / (24 * 60 * 60);
        return QString("%1일 전").arg(days);
    } else if (diffSeconds < 30 * 24 * 60 * 60) {
        int weeks = diffSeconds / (7 * 24 * 60 * 60);
        return QString("%1주 전").arg(weeks);
    } else if (diffSeconds < 365 * 24 * 60 * 60) {
        int months = diffSeconds / (30 * 24 * 60 * 60);
        return QString("%1개월 전").arg(months);
    } else {
        int years = diffSeconds / (365 * 24 * 60 * 60);
        return QString("%1년 전").arg(years);
    }
}

QString DateFormatter::toTimeString(const QDateTime &dateTime) {
    return dateTime.toString("HH:mm");
}

QString DateFormatter::toDateString(const QDateTime &dateTime) {
    return dateTime.toString("yyyy-MM-dd");
}

QString DateFormatter::toDateTimeString(const QDateTime &dateTime) {
    return dateTime.toString("yyyy-MM-dd HH:mm");
}

QString DateFormatter::getDateGroupName(int group) {
    switch (group) {
        case 0: // HistoryDateGroup::Today
            return "오늘";
        case 1: // HistoryDateGroup::Yesterday
            return "어제";
        case 2: // HistoryDateGroup::Last7Days
            return "지난 7일";
        case 3: // HistoryDateGroup::ThisMonth
            return "이번 달";
        case 4: // HistoryDateGroup::Older
            return "이전";
        default:
            return "알 수 없음";
    }
}

} // namespace webosbrowser
