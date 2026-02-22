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

# Homebrew 경로 추가 (macOS)
list(APPEND CMAKE_PREFIX_PATH /opt/homebrew)

# ─────────────────────────────────────────────────────────
# Qt 탐지 전략: Qt6 WebEngine → Qt5 WebEngine → Luna Service
# 중요: Qt5/Qt6를 혼용하면 QT_DEFAULT_MAJOR_VERSION 충돌 발생.
#        하나의 버전만 사용한다.
# ─────────────────────────────────────────────────────────

# 1단계: Qt6 + WebEngine 시도
find_package(Qt6 QUIET COMPONENTS Core Gui Widgets WebEngineWidgets)

if(Qt6_FOUND AND Qt6WebEngineWidgets_FOUND)
    message(STATUS "✅ Qt6 + WebEngineWidgets 사용 → 실제 브라우저 렌더링")
    set(QT_NAMESPACE Qt6)
    set(WEBENGINE_FOUND TRUE)
    set(USE_QT6 TRUE)

elseif(Qt6_FOUND)
    # Qt6 있지만 WebEngine 없음
    message(STATUS "⚠️  Qt6 있지만 WebEngineWidgets 없음 → Luna Service 모드")
    set(QT_NAMESPACE Qt6)
    set(WEBENGINE_FOUND FALSE)
    set(USE_QT6 TRUE)

else()
    # 2단계: Qt5 + WebEngine 시도
    message(STATUS "Qt6 없음 → Qt5 시도")
    find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets)
    find_package(Qt5WebEngineWidgets QUIET)

    set(QT_NAMESPACE Qt5)
    set(USE_QT6 FALSE)

    if(Qt5WebEngineWidgets_FOUND)
        message(STATUS "✅ Qt5 + WebEngineWidgets 사용")
        set(WEBENGINE_FOUND TRUE)
    else()
        message(STATUS "⚠️  Qt5 WebEngine 없음 → Luna Service 모드")
        set(WEBENGINE_FOUND FALSE)
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
# 소스 파일 목록
# ─────────────────────────────────────────────────────────
set(SOURCES
    src/main.cpp
    src/browser/BrowserWindow.cpp
    src/browser/TabManager.cpp
    src/ui/URLBar.cpp
    src/ui/NavigationBar.cpp
    src/ui/LoadingIndicator.cpp
    src/ui/ErrorPage.cpp
    src/ui/BookmarkPanel.cpp
    src/ui/HistoryPanel.cpp
    src/ui/SecurityIndicator.cpp
    src/ui/SettingsPanel.cpp
    src/ui/HomePage.cpp
    src/ui/BookmarkCard.cpp
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
    src/browser/BrowserWindow.h
    src/browser/WebView.h
    src/browser/TabManager.h
    src/ui/URLBar.h
    src/ui/NavigationBar.h
    src/ui/LoadingIndicator.h
    src/ui/ErrorPage.h
    src/ui/BookmarkPanel.h
    src/ui/HistoryPanel.h
    src/ui/SecurityIndicator.h
    src/ui/SettingsPanel.h
    src/ui/HomePage.h
    src/ui/BookmarkCard.h
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

# WebEngine 유무에 따라 WebView 구현 선택
if(WEBENGINE_FOUND)
    message(STATUS "WebView: QWebEngineView 기반 (실제 렌더링)")
    list(APPEND SOURCES src/browser/WebView.cpp)
    add_definitions(-DUSE_WEBENGINE)
else()
    message(STATUS "WebView: Luna Service 기반 (외부 브라우저 호출)")
    list(APPEND SOURCES src/browser/WebView_lunasvc.cpp)
    add_definitions(-DUSE_LUNA_SERVICE)
endif()

# ─────────────────────────────────────────────────────────
# 실행 파일 생성
# ─────────────────────────────────────────────────────────
add_executable(webosbrowser ${SOURCES} ${HEADERS} ${RESOURCES})

# Qt 라이브러리 링크
if(USE_QT6)
    target_link_libraries(webosbrowser Qt6::Core Qt6::Gui Qt6::Widgets)
    if(WEBENGINE_FOUND)
        target_link_libraries(webosbrowser Qt6::WebEngineWidgets)
    endif()
else()
    target_link_libraries(webosbrowser Qt5::Core Qt5::Gui Qt5::Widgets)
    if(WEBENGINE_FOUND)
        target_link_libraries(webosbrowser Qt5::WebEngineWidgets)
    endif()
endif()

# 컴파일 옵션
target_compile_options(webosbrowser PRIVATE
    -Wall
    -Wextra
)

# 출력 디렉토리
set_target_properties(webosbrowser PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# 테스트
enable_testing()

# 설치 규칙 (webOS 패키징용)
install(TARGETS webosbrowser RUNTIME DESTINATION bin)
install(DIRECTORY resources/ DESTINATION share/webosbrowser/resources)
install(FILES webos-meta/appinfo.json DESTINATION share/webosbrowser)
