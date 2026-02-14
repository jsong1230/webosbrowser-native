/**
 * @file StorageService.h
 * @brief webOS LS2 API를 사용한 스토리지 서비스
 */

#pragma once

#include <QObject>
#include <QString>
#include <QJsonObject>
#include <QJsonArray>
#include <QUuid>
#include <functional>

namespace webosbrowser {

/**
 * @brief webOS LS2 API 래퍼 클래스
 *
 * com.webos.service.db 서비스를 사용하여 데이터 영속 저장
 * 북마크 및 폴더 데이터를 저장하고 조회합니다.
 */
class StorageService : public QObject {
    Q_OBJECT

public:
    explicit StorageService(QObject *parent = nullptr);
    ~StorageService() override;

    StorageService(const StorageService&) = delete;
    StorageService& operator=(const StorageService&) = delete;

    /**
     * @brief 데이터베이스 초기화 (Kind 등록)
     * @param callback 초기화 완료 콜백 (성공 여부)
     */
    void initDatabase(std::function<void(bool)> callback);

    /**
     * @brief 데이터 저장 (put)
     * @param kind Kind 이름 (예: "com.jsong.webosbrowser:1.bookmarks")
     * @param data JSON 객체 배열
     * @param callback 완료 콜백 (성공 여부, 응답 데이터)
     */
    void putData(const QString& kind, const QJsonArray& data, std::function<void(bool, const QJsonObject&)> callback);

    /**
     * @brief 데이터 조회 (find)
     * @param kind Kind 이름
     * @param query 쿼리 객체 (선택적)
     * @param callback 완료 콜백 (성공 여부, 결과 배열)
     */
    void findData(const QString& kind, const QJsonObject& query, std::function<void(bool, const QJsonArray&)> callback);

    /**
     * @brief 단일 데이터 조회 (get)
     * @param kind Kind 이름
     * @param id 데이터 ID (_id)
     * @param callback 완료 콜백 (성공 여부, 결과 객체)
     */
    void getData(const QString& kind, const QString& id, std::function<void(bool, const QJsonObject&)> callback);

    /**
     * @brief 데이터 삭제 (del)
     * @param kind Kind 이름
     * @param ids 삭제할 ID 배열
     * @param callback 완료 콜백 (성공 여부)
     */
    void deleteData(const QString& kind, const QStringList& ids, std::function<void(bool)> callback);

    /**
     * @brief UUID 생성
     * @return UUID 문자열
     */
    static QString generateUuid() {
        return QUuid::createUuid().toString(QUuid::WithoutBraces);
    }

signals:
    /**
     * @brief 에러 발생 시그널
     * @param message 에러 메시지
     */
    void errorOccurred(const QString& message);

private:
    /**
     * @brief LS2 호출 (내부 구현, 향후 webOS API 연동)
     * @param service 서비스 URI (예: "luna://com.webos.service.db/")
     * @param method 메서드 이름 (예: "find")
     * @param payload JSON 페이로드
     * @param callback 완료 콜백
     */
    void callLS2(const QString& service, const QString& method, const QJsonObject& payload, std::function<void(bool, const QJsonObject&)> callback);

    bool m_initialized;      // 데이터베이스 초기화 상태
};

} // namespace webosbrowser
