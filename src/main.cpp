/**
 * @file main.cpp
 * @brief webOS Browser Native App 진입점
 */

#include <QApplication>
#include <QDebug>
#include "browser/BrowserWindow.h"

namespace webosbrowser {

/**
 * @brief 애플리케이션 진입점
 * @param argc 명령줄 인자 개수
 * @param argv 명령줄 인자 배열
 * @return 애플리케이션 종료 코드
 */
int main(int argc, char *argv[]) {
    // Qt 애플리케이션 초기화
    QApplication app(argc, argv);

    // 애플리케이션 정보 설정
    QApplication::setApplicationName("webOS Browser");
    QApplication::setApplicationVersion("0.1.0");
    QApplication::setOrganizationName("jsong");
    QApplication::setOrganizationDomain("com.jsong.webosbrowser.native");

    qDebug() << "webOS Browser Native App 시작...";

    // 메인 윈도우 생성 및 표시
    BrowserWindow window;
    window.show();

    // 이벤트 루프 실행
    return app.exec();
}

} // namespace webosbrowser

/**
 * @brief C 스타일 main 함수 (진입점)
 */
int main(int argc, char *argv[]) {
    return webosbrowser::main(argc, argv);
}
