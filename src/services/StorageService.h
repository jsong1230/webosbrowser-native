/**
 * @file StorageService.h
 * @brief 스토리지 서비스 헤더 - webOS LS2 API 래퍼
 *
 * webOS LS2 API를 사용하여 영속 데이터를 저장/조회하는 서비스.
 * 북마크, 히스토리, 설정 등의 데이터를 Kind 기반으로 관리.
 */

#pragma once

#include <QObject>
#include <QString>
#include <QJsonObject>
#include <QJsonArray>
#include <QVariantMap>

namespace webosbrowser {

/**
 * @enum DataKind
 * @brief LS2 DB8 Kind 타입
 */
enum class DataKind {
    Bookmark,   ///< 북마크 데이터
    History,    ///< 히스토리 데이터
    Settings    ///< 설정 데이터
};

/**
 * @class StorageService
 * @brief webOS LS2 API 래퍼 클래스
 *
 * webOS의 LS2 (Luna Service 2) API를 사용하여 DB8 (내장 NoSQL DB)에
 * 데이터를 저장하고 조회합니다. 북마크, 히스토리, 설정 등을 Kind별로 관리.
 *
 * 참고: 현재는 단순 JSON 파일 기반 구현 (Mock). 실제 webOS 환경에서는
 * luna-service2 라이브러리를 사용하여 DB8 API를 호출해야 합니다.
 */
class StorageService : public QObject {
    Q_OBJECT

public:
    /**
     * @brief 생성자
     * @param parent 부모 객체 (기본값: nullptr)
     */
    explicit StorageService(QObject *parent = nullptr);

    /**
     * @brief 소멸자
     */
    ~StorageService() override;

    // 복사 생성자 및 대입 연산자 삭제 (QObject 규칙)
    StorageService(const StorageService&) = delete;
    StorageService& operator=(const StorageService&) = delete;

    /**
     * @brief LS2 스토리지 초기화
     *
     * Kind 스키마를 생성하고 DB8 연결을 초기화합니다.
     * 앱 시작 시 한 번만 호출되어야 합니다.
     *
     * @return 초기화 성공 여부
     */
    bool initialize();

    /**
     * @brief 데이터 저장 (Put)
     * @param kind Kind 타입 (Bookmark, History, Settings)
     * @param id 데이터 ID (고유 식별자)
     * @param data JSON 데이터
     * @return 저장 성공 여부
     */
    bool putData(DataKind kind, const QString &id, const QJsonObject &data);

    /**
     * @brief 데이터 조회 (Get by ID)
     * @param kind Kind 타입
     * @param id 데이터 ID
     * @return JSON 데이터 (존재하지 않으면 빈 객체)
     */
    QJsonObject getData(DataKind kind, const QString &id) const;

    /**
     * @brief 모든 데이터 조회 (Find)
     * @param kind Kind 타입
     * @return JSON 배열 (모든 항목)
     */
    QJsonArray findAllData(DataKind kind) const;

    /**
     * @brief 쿼리 기반 데이터 조회 (Find with Query)
     * @param kind Kind 타입
     * @param query 쿼리 조건 (JSON 객체)
     * @return JSON 배열 (쿼리 결과)
     */
    QJsonArray findData(DataKind kind, const QJsonObject &query) const;

    /**
     * @brief 데이터 삭제 (Delete by ID)
     * @param kind Kind 타입
     * @param id 데이터 ID
     * @return 삭제 성공 여부
     */
    bool deleteData(DataKind kind, const QString &id);

    /**
     * @brief 모든 데이터 삭제 (Delete All)
     * @param kind Kind 타입
     * @return 삭제된 항목 개수
     */
    int deleteAllData(DataKind kind);

    /**
     * @brief 데이터 개수 조회 (Count)
     * @param kind Kind 타입
     * @return 항목 개수
     */
    int countData(DataKind kind) const;

signals:
    /**
     * @brief 스토리지 초기화 완료 시그널
     */
    void initialized();

    /**
     * @brief 데이터 저장 완료 시그널
     * @param kind Kind 타입
     * @param id 데이터 ID
     */
    void dataSaved(DataKind kind, const QString &id);

    /**
     * @brief 데이터 삭제 완료 시그널
     * @param kind Kind 타입
     * @param id 데이터 ID
     */
    void dataDeleted(DataKind kind, const QString &id);

    /**
     * @brief 스토리지 에러 시그널
     * @param errorMessage 에러 메시지
     */
    void storageError(const QString &errorMessage);

private:
    /**
     * @brief Kind 타입을 문자열로 변환
     * @param kind Kind 타입
     * @return Kind 문자열 (예: "bookmark", "history")
     */
    QString kindToString(DataKind kind) const;

    /**
     * @brief LS2 API 호출 (Mock 구현)
     *
     * 실제 webOS 환경에서는 luna-service2 라이브러리의
     * LSCall() 함수를 사용하여 DB8 API를 호출해야 합니다.
     *
     * @param method LS2 메서드 (예: "luna://com.webos.service.db/put")
     * @param params 파라미터 (JSON)
     * @return 응답 JSON
     */
    QJsonObject callLS2(const QString &method, const QJsonObject &params);

    /**
     * @brief JSON 파일 경로 반환 (Mock 구현용)
     * @param kind Kind 타입
     * @return 파일 경로
     */
    QString getStorageFilePath(DataKind kind) const;

    /**
     * @brief JSON 파일에서 데이터 로드 (Mock 구현)
     * @param kind Kind 타입
     * @return JSON 배열
     */
    QJsonArray loadFromFile(DataKind kind) const;

    /**
     * @brief JSON 파일에 데이터 저장 (Mock 구현)
     * @param kind Kind 타입
     * @param data JSON 배열
     * @return 저장 성공 여부
     */
    bool saveToFile(DataKind kind, const QJsonArray &data);

private:
    bool isInitialized_;        ///< 초기화 플래그
    QString storageDir_;        ///< 스토리지 디렉토리 경로 (Mock)

    // LS2 Handle (실제 webOS 환경에서 사용)
    // LSHandle *lsHandle_;
};

} // namespace webosbrowser
