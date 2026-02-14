/**
 * @file HistoryService.h
 * @brief 히스토리 관리 서비스 헤더
 *
 * 웹 페이지 방문 기록을 자동으로 저장하고 관리하는 서비스.
 * webOS LS2 API를 사용하여 영속 데이터 저장을 구현.
 */

#pragma once

#include <QObject>
#include <QString>
#include <QDateTime>
#include <QList>
#include <QMap>
#include <QJsonObject>
#include <QSharedPointer>

namespace webosbrowser {

// Forward declaration
class StorageService;

/**
 * @struct HistoryEntry
 * @brief 히스토리 항목 데이터 구조
 */
struct HistoryEntry {
    QString id;             ///< 고유 식별자 (UUID)
    QString url;            ///< 방문한 URL
    QString title;          ///< 웹 페이지 제목
    QString favicon;        ///< 파비콘 URL (선택적)
    QDateTime visitedAt;    ///< 방문 시각
    int visitCount;         ///< 방문 횟수 (중복 방지용)

    /**
     * @brief JSON으로 변환
     * @return JSON 객체
     */
    QJsonObject toJson() const;

    /**
     * @brief JSON에서 파싱
     * @param json JSON 객체
     * @return HistoryEntry 객체
     */
    static HistoryEntry fromJson(const QJsonObject &json);
};

/**
 * @enum HistoryDateGroup
 * @brief 날짜별 그룹화 카테고리
 */
enum class HistoryDateGroup {
    Today,      ///< 오늘 (0~24시간 전)
    Yesterday,  ///< 어제 (24~48시간 전)
    Last7Days,  ///< 지난 7일 (2~7일 전)
    ThisMonth,  ///< 이번 달 (8~30일 전)
    Older       ///< 이전 (30일 이상)
};

/**
 * @class HistoryService
 * @brief 히스토리 관리 서비스 클래스
 *
 * 웹 페이지 방문 기록의 CRUD 작업을 담당하며, 중복 방지, 용량 제한,
 * 검색, 날짜별 그룹화 등의 기능을 제공합니다.
 */
class HistoryService : public QObject {
    Q_OBJECT

public:
    /**
     * @brief 생성자
     * @param storageService 스토리지 서비스 (LS2 API 래퍼)
     * @param parent 부모 객체 (기본값: nullptr)
     */
    explicit HistoryService(StorageService *storageService, QObject *parent = nullptr);

    /**
     * @brief 소멸자
     */
    ~HistoryService() override;

    // 복사 생성자 및 대입 연산자 삭제 (QObject 규칙)
    HistoryService(const HistoryService&) = delete;
    HistoryService& operator=(const HistoryService&) = delete;

    /**
     * @brief 히스토리 자동 기록 (페이지 방문 시 호출)
     * @param url 방문한 URL
     * @param title 웹 페이지 제목
     * @param favicon 파비콘 URL (선택적, 기본값: 빈 문자열)
     *
     * 동일 URL을 1분 내 재방문 시 중복 기록하지 않고
     * visitedAt과 visitCount만 업데이트합니다.
     */
    void recordVisit(const QString &url, const QString &title, const QString &favicon = QString());

    /**
     * @brief 모든 히스토리 조회 (방문 시각 역순)
     * @return 히스토리 목록
     */
    QList<HistoryEntry> getAllHistory() const;

    /**
     * @brief 기간별 히스토리 조회
     * @param startDate 시작 날짜
     * @param endDate 종료 날짜
     * @return 기간 내 히스토리 목록
     */
    QList<HistoryEntry> getHistoryByDateRange(const QDateTime &startDate, const QDateTime &endDate) const;

    /**
     * @brief ID로 히스토리 조회
     * @param id 히스토리 ID
     * @return 히스토리 항목 (존재하지 않으면 빈 항목)
     */
    HistoryEntry getHistoryById(const QString &id) const;

    /**
     * @brief 히스토리 검색 (제목, URL 부분 일치)
     * @param query 검색어
     * @return 검색 결과 히스토리 목록
     */
    QList<HistoryEntry> searchHistory(const QString &query) const;

    /**
     * @brief 날짜별 그룹화된 히스토리 반환
     * @return 날짜 그룹별 히스토리 맵
     */
    QMap<HistoryDateGroup, QList<HistoryEntry>> getGroupedHistory() const;

    /**
     * @brief 개별 히스토리 삭제
     * @param id 삭제할 히스토리 ID
     * @return 삭제 성공 여부
     */
    bool deleteHistory(const QString &id);

    /**
     * @brief 기간별 히스토리 삭제
     * @param startDate 시작 날짜
     * @param endDate 종료 날짜
     * @return 삭제된 항목 개수
     */
    int deleteHistoryByDateRange(const QDateTime &startDate, const QDateTime &endDate);

    /**
     * @brief 전체 히스토리 삭제
     * @return 삭제 성공 여부
     */
    bool deleteAllHistory();

    /**
     * @brief 히스토리 항목 개수 반환
     * @return 총 히스토리 개수
     */
    int getHistoryCount() const;

signals:
    /**
     * @brief 히스토리 추가 완료 시그널
     * @param entry 추가된 히스토리 항목
     */
    void historyAdded(const HistoryEntry &entry);

    /**
     * @brief 히스토리 삭제 완료 시그널
     * @param id 삭제된 히스토리 ID
     */
    void historyDeleted(const QString &id);

    /**
     * @brief 히스토리 업데이트 완료 시그널 (중복 방지 시 visitCount 증가)
     * @param entry 업데이트된 히스토리 항목
     */
    void historyUpdated(const HistoryEntry &entry);

    /**
     * @brief 전체 히스토리 삭제 완료 시그널
     */
    void allHistoryDeleted();

    /**
     * @brief 히스토리 로드 완료 시그널 (앱 초기화 시)
     */
    void historyLoaded();

private:
    /**
     * @brief 히스토리 데이터를 LS2 스토리지에서 로드
     */
    void loadHistoryFromStorage();

    /**
     * @brief 히스토리 데이터를 LS2 스토리지에 저장
     */
    void saveHistoryToStorage();

    /**
     * @brief 최근 1분 내 동일 URL 방문 체크 (중복 방지)
     * @param url 조회할 URL
     * @return 최근 방문 항목 (없으면 nullptr)
     */
    HistoryEntry* findRecentVisit(const QString &url);

    /**
     * @brief 히스토리 용량 제한 체크 (최대 5,000개)
     *
     * 5,000개 초과 시 오래된 항목부터 자동 삭제
     */
    void pruneOldHistory();

    /**
     * @brief UUID 생성
     * @return 고유 ID 문자열
     */
    QString generateUUID() const;

    /**
     * @brief 날짜별 그룹 계산
     * @param visitedAt 방문 시각
     * @return 날짜 그룹 카테고리
     */
    HistoryDateGroup calculateDateGroup(const QDateTime &visitedAt) const;

private:
    StorageService *storageService_;        ///< 스토리지 서비스 (LS2 API)
    QList<HistoryEntry> historyCache_;      ///< 히스토리 메모리 캐시
    bool isLoaded_;                         ///< 로드 완료 플래그

    static const int MAX_HISTORY_COUNT = 5000;       ///< 최대 히스토리 개수
    static const qint64 DUPLICATE_THRESHOLD_MS = 60000;  ///< 중복 체크 임계값 (1분)
};

} // namespace webosbrowser
