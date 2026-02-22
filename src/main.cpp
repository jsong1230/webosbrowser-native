/**
 * @file main.cpp
 * @brief webOS Browser Native App 진입점 (QML 기반)
 *
 * QGuiApplication + QQmlApplicationEngine 구조로 구현.
 * 서비스 클래스들을 QML Context Property로 등록하여 QML에서 사용 가능하게 합니다.
 */

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QDebug>

#include "browser/WebOSController.h"
#include "services/BookmarkService.h"
#include "services/HistoryService.h"
#include "services/SettingsService.h"
#include "services/StorageService.h"
#include "services/SearchEngine.h"

using namespace webosbrowser;

int main(int argc, char *argv[])
{
    // webOS Wayland 환경 설정 (webOS 타겟 빌드에서만 적용)
#ifdef USE_WEBOS_TARGET
    qputenv("QT_QPA_PLATFORM", "wayland");
    qputenv("XDG_RUNTIME_DIR", "/tmp/xdg");
    qputenv("WAYLAND_DISPLAY", "wayland-0");
    qputenv("QT_WAYLAND_DISABLE_WINDOWDECORATION", "1");
    qDebug() << "[main] webOS Wayland 환경 설정 완료";
#else
    qDebug() << "[main] macOS 개발 환경 (Wayland 설정 생략)";
#endif

    // Qt 6에서 High DPI 자동 활성화 (Qt5에서는 수동 설정)
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

    QGuiApplication app(argc, argv);

    // 애플리케이션 메타데이터 설정
    QGuiApplication::setApplicationName("webOS Browser");
    QGuiApplication::setApplicationVersion("1.0.0");
    QGuiApplication::setOrganizationName("jsong");
    QGuiApplication::setOrganizationDomain("com.jsong.webosbrowser.native");

    qDebug() << "[main] webOS Browser Native App (QML) 시작";

    // ─────────────────────────────────────────────────────────
    // 서비스 초기화 (의존성 주입 체인)
    // ─────────────────────────────────────────────────────────
    StorageService storageService;
    storageService.initialize();

    BookmarkService bookmarkService(&storageService);
    HistoryService historyService(&storageService);
    SettingsService settingsService(&storageService);
    settingsService.loadSettings();

    WebOSController webosController;

    // ─────────────────────────────────────────────────────────
    // QML 엔진 설정
    // ─────────────────────────────────────────────────────────
    QQmlApplicationEngine engine;
    QQmlContext* context = engine.rootContext();

    // C++ 서비스를 QML Context Property로 등록
    context->setContextProperty("webOSController", &webosController);
    context->setContextProperty("bookmarkService", &bookmarkService);
    context->setContextProperty("historyService", &historyService);
    context->setContextProperty("settingsService", &settingsService);

    qDebug() << "[main] QML Context Property 등록 완료";

    // QML import 경로 추가 (컴포넌트 파일들 위치)
    engine.addImportPath(QStringLiteral("qrc:/"));

    // QML 메인 파일 로드
    engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));

    if (engine.rootObjects().isEmpty()) {
        qCritical() << "[main] QML 로드 실패! 종료합니다.";
        return -1;
    }

    qDebug() << "[main] QML 로드 성공, 이벤트 루프 시작";
    return app.exec();
}
