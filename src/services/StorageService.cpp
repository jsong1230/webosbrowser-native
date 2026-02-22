/**
 * @file StorageService.cpp
 * @brief 스토리지 서비스 구현 - webOS LS2 API 래퍼
 */

#include "StorageService.h"
#include "utils/Logger.h"
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QStandardPaths>

namespace webosbrowser {

StorageService::StorageService(QObject *parent)
    : QObject(parent)
    , isInitialized_(false)
{
    // 데이터 디렉토리 경로 설정
    storageDir_ = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

    // 디렉토리가 없으면 생성 시도
    QDir dir;
    if (!dir.exists(storageDir_)) {
        if (!dir.mkpath(storageDir_)) {
            // 기본 경로 생성 실패 시 /tmp 폴백 (webOS 샌드박스 환경 대응)
            storageDir_ = "/tmp/webosbrowser_data";
            if (!dir.exists(storageDir_)) {
                dir.mkpath(storageDir_);
            }
        }
    }
    Logger::info(QString("스토리지 디렉토리: %1").arg(storageDir_));
}

StorageService::~StorageService() {
}

bool StorageService::initialize() {
    if (isInitialized_) {
        Logger::warning("StorageService가 이미 초기화되었습니다.");
        return true;
    }

    // Mock 구현: 스토리지 디렉토리 확인
    QDir dir(storageDir_);
    if (!dir.exists()) {
        Logger::error(QString("스토리지 디렉토리가 존재하지 않습니다: %1").arg(storageDir_));
        return false;
    }

    // 실제 webOS 환경에서는 여기서 LS2 API 초기화
    // LSError lserror;
    // LSErrorInit(&lserror);
    // if (!LSRegister("com.jsong.webosbrowser.native", &lsHandle_, &lserror)) {
    //     Logger::error(QString("LS2 등록 실패: %1").arg(lserror.message));
    //     return false;
    // }

    isInitialized_ = true;
    emit initialized();
    Logger::info("StorageService 초기화 완료 (Mock 모드)");

    return true;
}

bool StorageService::putData(DataKind kind, const QString &id, const QJsonObject &data) {
    if (!isInitialized_) {
        Logger::error("StorageService가 초기화되지 않았습니다.");
        return false;
    }

    // Mock 구현: JSON 파일에서 데이터 로드
    QJsonArray allData = loadFromFile(kind);

    // 기존 항목 찾기 (ID 일치)
    bool found = false;
    for (int i = 0; i < allData.size(); ++i) {
        QJsonObject item = allData[i].toObject();
        if (item["id"].toString() == id) {
            // 업데이트
            allData[i] = data;
            found = true;
            break;
        }
    }

    // 새 항목 추가
    if (!found) {
        allData.append(data);
    }

    // 파일에 저장
    bool success = saveToFile(kind, allData);
    if (success) {
        emit dataSaved(kind, id);
        Logger::debug(QString("데이터 저장 완료: Kind=%1, ID=%2").arg(kindToString(kind)).arg(id));
    } else {
        emit storageError(QString("데이터 저장 실패: Kind=%1, ID=%2").arg(kindToString(kind)).arg(id));
    }

    return success;
}

QJsonObject StorageService::getData(DataKind kind, const QString &id) const {
    if (!isInitialized_) {
        Logger::error("StorageService가 초기화되지 않았습니다.");
        return QJsonObject();
    }

    QJsonArray allData = loadFromFile(kind);

    for (const QJsonValue &value : allData) {
        QJsonObject item = value.toObject();
        if (item["id"].toString() == id) {
            return item;
        }
    }

    return QJsonObject(); // 없으면 빈 객체 반환
}

QJsonArray StorageService::findAllData(DataKind kind) const {
    if (!isInitialized_) {
        Logger::error("StorageService가 초기화되지 않았습니다.");
        return QJsonArray();
    }

    return loadFromFile(kind);
}

QJsonArray StorageService::findData(DataKind kind, const QJsonObject &query) const {
    if (!isInitialized_) {
        Logger::error("StorageService가 초기화되지 않았습니다.");
        return QJsonArray();
    }

    // Mock 구현: 간단한 필터링 (실제로는 LS2 쿼리 사용)
    QJsonArray allData = loadFromFile(kind);
    QJsonArray result;

    for (const QJsonValue &value : allData) {
        QJsonObject item = value.toObject();
        bool match = true;

        // 쿼리 조건 체크
        for (const QString &key : query.keys()) {
            if (item[key] != query[key]) {
                match = false;
                break;
            }
        }

        if (match) {
            result.append(item);
        }
    }

    return result;
}

bool StorageService::deleteData(DataKind kind, const QString &id) {
    if (!isInitialized_) {
        Logger::error("StorageService가 초기화되지 않았습니다.");
        return false;
    }

    QJsonArray allData = loadFromFile(kind);
    QJsonArray newData;

    bool found = false;
    for (const QJsonValue &value : allData) {
        QJsonObject item = value.toObject();
        if (item["id"].toString() == id) {
            found = true;
            continue; // 삭제할 항목은 제외
        }
        newData.append(item);
    }

    if (!found) {
        Logger::warning(QString("삭제할 데이터를 찾을 수 없습니다: Kind=%1, ID=%2").arg(kindToString(kind)).arg(id));
        return false;
    }

    bool success = saveToFile(kind, newData);
    if (success) {
        emit dataDeleted(kind, id);
        Logger::debug(QString("데이터 삭제 완료: Kind=%1, ID=%2").arg(kindToString(kind)).arg(id));
    }

    return success;
}

int StorageService::deleteAllData(DataKind kind) {
    if (!isInitialized_) {
        Logger::error("StorageService가 초기화되지 않았습니다.");
        return 0;
    }

    int count = countData(kind);

    QJsonArray emptyArray;
    bool success = saveToFile(kind, emptyArray);

    if (success) {
        Logger::info(QString("전체 데이터 삭제 완료: Kind=%1, Count=%2").arg(kindToString(kind)).arg(count));
    }

    return success ? count : 0;
}

int StorageService::countData(DataKind kind) const {
    if (!isInitialized_) {
        Logger::error("StorageService가 초기화되지 않았습니다.");
        return 0;
    }

    QJsonArray allData = loadFromFile(kind);
    return allData.size();
}

QString StorageService::kindToString(DataKind kind) const {
    switch (kind) {
        case DataKind::Bookmark:
            return "bookmark";
        case DataKind::History:
            return "history";
        case DataKind::Settings:
            return "settings";
        default:
            return "unknown";
    }
}

QJsonObject StorageService::callLS2(const QString &method, const QJsonObject &params) {
    // Mock 구현: 실제로는 LSCall() 사용
    Q_UNUSED(method)
    Q_UNUSED(params)

    Logger::warning("LS2 API 호출 (Mock): 실제 webOS 환경에서만 동작합니다.");
    return QJsonObject();
}

QString StorageService::getStorageFilePath(DataKind kind) const {
    QString fileName = QString("%1.json").arg(kindToString(kind));
    return QDir(storageDir_).filePath(fileName);
}

QJsonArray StorageService::loadFromFile(DataKind kind) const {
    QString filePath = getStorageFilePath(kind);
    QFile file(filePath);

    if (!file.exists()) {
        return QJsonArray(); // 파일이 없으면 빈 배열 반환
    }

    if (!file.open(QIODevice::ReadOnly)) {
        Logger::error(QString("파일 읽기 실패: %1").arg(filePath));
        return QJsonArray();
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isArray()) {
        Logger::error(QString("JSON 파싱 실패: %1").arg(filePath));
        return QJsonArray();
    }

    return doc.array();
}

bool StorageService::saveToFile(DataKind kind, const QJsonArray &data) {
    QString filePath = getStorageFilePath(kind);
    QFile file(filePath);

    if (!file.open(QIODevice::WriteOnly)) {
        Logger::error(QString("파일 쓰기 실패: %1").arg(filePath));
        return false;
    }

    QJsonDocument doc(data);
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();

    return true;
}

} // namespace webosbrowser
