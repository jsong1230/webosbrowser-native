cmake_minimum_required(VERSION 3.16)
project(webosbrowser-native VERSION 1.0.0 LANGUAGES CXX)

# C++ 표준 설정
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Qt 자동 처리 활성화
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# ─────────────────────────────────────────────────────────
# 빌드 타겟 분기
#   -DWEBOS_TARGET=ON  → webOS ARMv7 (Qt5 전용, Wayland, QML)
#   (기본)             → macOS 개발 (Qt6 우선, WebEngine, QML)
# ─────────────────────────────────────────────────────────

option(WEBOS_TARGET "webOS 프로젝터 타겟 빌드" OFF)

if(WEBOS_TARGET)
    # ─────────────────────────────────────────────────────
    # webOS 타겟: Qt5만 사용, Widgets 없음, QML 기반
    # ─────────────────────────────────────────────────────
    message(STATUS "=== webOS 타겟 빌드 모드 ===")
    message(STATUS "Qt5 전용, QML 기반, Luna Service 모드")

    find_package(Qt5 REQUIRED COMPONENTS Core Gui Qml Quick Network)

    set(QT_NAMESPACE Qt5)
    set(USE_QT6 FALSE)
    set(WEBENGINE_FOUND FALSE)

    # webOS 전용 컴파일 정의
    add_definitions(-DUSE_WEBOS_TARGET)
    add_definitions(-DUSE_LUNA_SERVICE)

    message(STATUS "Qt5 버전: ${Qt5Core_VERSION}")

else()
    # ─────────────────────────────────────────────────────
    # macOS 개발 환경: Qt6 우선, Qt5 fallback
    # ─────────────────────────────────────────────────────
    message(STATUS "=== macOS 개발 환경 빌드 모드 ===")

    # Homebrew 경로 추가 (macOS)
    list(APPEND CMAKE_PREFIX_PATH /opt/homebrew)

    # 1단계: Qt6 + WebEngine 시도
    find_package(Qt6 QUIET COMPONENTS Core Gui Qml Quick Network WebEngineWidgets)

    if(Qt6_FOUND AND Qt6WebEngineWidgets_FOUND)
        message(STATUS "Qt6 + WebEngineWidgets 사용 → 실제 브라우저 렌더링")
        set(QT_NAMESPACE Qt6)
        set(WEBENGINE_FOUND TRUE)
        set(USE_QT6 TRUE)

    elseif(Qt6_FOUND)
        message(STATUS "Qt6 있지만 WebEngineWidgets 없음 → QML + Luna Service 모드")
        find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick Network)
        set(QT_NAMESPACE Qt6)
        set(WEBENGINE_FOUND FALSE)
        set(USE_QT6 TRUE)

    else()
        # 2단계: Qt5 fallback
        message(STATUS "Qt6 없음 → Qt5 시도")
        find_package(Qt5 REQUIRED COMPONENTS Core Gui Qml Quick Network)
        find_package(Qt5WebEngineWidgets QUIET)

        set(QT_NAMESPACE Qt5)
        set(USE_QT6 FALSE)

        if(Qt5WebEngineWidgets_FOUND)
            message(STATUS "Qt5 + WebEngineWidgets 사용")
            set(WEBENGINE_FOUND TRUE)
        else()
            message(STATUS "Qt5 WebEngine 없음 → QML + Luna Service 모드")
            set(WEBENGINE_FOUND FALSE)
        endif()
    endif()
endif()

# ─────────────────────────────────────────────────────────
# 헤더 경로
# ─────────────────────────────────────────────────────────
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# ─────────────────────────────────────────────────────────
# 소스 파일 목록 (QML 기반 - QWidget UI 제거됨)
# ─────────────────────────────────────────────────────────
set(SOURCES
    src/main.cpp
    src/browser/WebOSController.cpp
    src/services/BookmarkService.cpp
    src/services/HistoryService.cpp
    src/services/StorageService.cpp
    src/services/SearchEngine.cpp
    src/services/SettingsService.cpp
    src/utils/Logger.cpp
    src/utils/URLValidator.cpp
    src/utils/DateFormatter.cpp
    src/utils/SecurityClassifier.cpp
)

set(HEADERS
    src/browser/WebOSController.h
    src/models/Bookmark.h
    src/services/BookmarkService.h
    src/services/HistoryService.h
    src/services/StorageService.h
    src/services/SearchEngine.h
    src/services/SettingsService.h
    src/utils/Logger.h
    src/utils/URLValidator.h
    src/utils/DateFormatter.h
    src/utils/SecurityClassifier.h
    src/utils/KeyCodeConstants.h
)

set(RESOURCES
    resources/resources.qrc
)

# ─────────────────────────────────────────────────────────
# WebEngine 유무에 따라 WebView 구현 선택 (macOS 전용)
# webOS 타겟에서는 WebOSController만 사용
# ─────────────────────────────────────────────────────────
if(NOT WEBOS_TARGET AND WEBENGINE_FOUND)
    message(STATUS "WebEngine 모드: Qt WebEngineView 포함")
    add_definitions(-DUSE_WEBENGINE)
    # WebEngine 통합은 향후 별도 컨트롤러로 구현 예정
endif()

# ─────────────────────────────────────────────────────────
# 실행 파일 생성
# ─────────────────────────────────────────────────────────
if(USE_QT6)
    # Qt6: qt_add_executable 사용 (권장)
    qt_add_executable(webosbrowser ${SOURCES} ${HEADERS})
    # Qt6: qt_add_resources로 QRC 추가
    # PREFIX /qml로 설정하면 qrc:/qml/main.qml 경로로 접근 가능
    qt_add_resources(webosbrowser "qml_resources"
        PREFIX "/qml"
        BASE "resources/qml"
        FILES
            resources/qml/main.qml
            resources/qml/UrlBar.qml
            resources/qml/NavBar.qml
            resources/qml/BookmarkPanel.qml
            resources/qml/HistoryPanel.qml
            resources/qml/HomeScreen.qml
            resources/qml/SettingsPanel.qml
    )
    qt_add_resources(webosbrowser "style_resources"
        PREFIX "/styles"
        BASE "resources/styles"
        FILES
            resources/styles/dark.qss
            resources/styles/light.qss
    )
else()
    # Qt5: 기존 방식
    add_executable(webosbrowser ${SOURCES} ${HEADERS} ${RESOURCES})
endif()

# ─────────────────────────────────────────────────────────
# Qt 라이브러리 링크
# ─────────────────────────────────────────────────────────
if(USE_QT6)
    # Qt6: PRIVATE 키워드 방식 필수 (qt_add_executable과 혼용 시)
    target_link_libraries(webosbrowser PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Qml
        Qt6::Quick
        Qt6::Network
    )
    if(WEBENGINE_FOUND)
        target_link_libraries(webosbrowser PRIVATE Qt6::WebEngineWidgets)
    endif()
else()
    # Qt5 링크 (webOS 타겟 포함)
    target_link_libraries(webosbrowser
        Qt5::Core
        Qt5::Gui
        Qt5::Qml
        Qt5::Quick
        Qt5::Network
    )
    if(NOT WEBOS_TARGET AND WEBENGINE_FOUND)
        target_link_libraries(webosbrowser Qt5::WebEngineWidgets)
    endif()
endif()

# ─────────────────────────────────────────────────────────
# 컴파일 옵션
# ─────────────────────────────────────────────────────────
target_compile_options(webosbrowser PRIVATE
    -Wall
    -Wextra
)

if(WEBOS_TARGET)
    # ARM 최적화 옵션
    target_compile_options(webosbrowser PRIVATE
        -march=armv7-a
        -mfpu=neon
        -mfloat-abi=hard
    )
endif()

# ─────────────────────────────────────────────────────────
# 출력 디렉토리
# ─────────────────────────────────────────────────────────
set_target_properties(webosbrowser PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# ─────────────────────────────────────────────────────────
# 테스트
# ─────────────────────────────────────────────────────────
enable_testing()

# ─────────────────────────────────────────────────────────
# 설치 규칙 (webOS 패키징용)
# ─────────────────────────────────────────────────────────
install(TARGETS webosbrowser RUNTIME DESTINATION bin)
install(DIRECTORY resources/ DESTINATION share/webosbrowser/resources)
install(FILES webos-meta/appinfo.json DESTINATION share/webosbrowser)

# ─────────────────────────────────────────────────────────
# 빌드 정보 요약
# ─────────────────────────────────────────────────────────
message(STATUS "")
message(STATUS "=== 빌드 설정 요약 ===")
message(STATUS "타겟: ${CMAKE_SYSTEM_NAME} (${CMAKE_SYSTEM_PROCESSOR})")
message(STATUS "Qt 네임스페이스: ${QT_NAMESPACE}")
message(STATUS "WebEngine: ${WEBENGINE_FOUND}")
message(STATUS "webOS 타겟: ${WEBOS_TARGET}")
message(STATUS "빌드 타입: ${CMAKE_BUILD_TYPE}")
message(STATUS "=======================")
