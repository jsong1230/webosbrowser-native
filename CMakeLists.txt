cmake_minimum_required(VERSION 3.16)
project(webosbrowser-native VERSION 0.1.0 LANGUAGES CXX)

# C++ 표준 설정
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Qt 자동 처리 활성화
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Qt 패키지 찾기
find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets)

# 헤더 파일 경로
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# 소스 파일 목록
set(SOURCES
    src/main.cpp
    src/browser/BrowserWindow.cpp
    src/browser/WebView.cpp
    src/browser/TabManager.cpp
    src/ui/URLBar.cpp
    src/ui/NavigationBar.cpp
    src/ui/LoadingIndicator.cpp
    src/ui/BookmarkPanel.cpp
    src/ui/HistoryPanel.cpp
    src/services/BookmarkService.cpp
    src/services/HistoryService.cpp
    src/services/StorageService.cpp
    src/services/SearchEngine.cpp
    src/utils/Logger.cpp
    src/utils/URLValidator.cpp
    src/utils/DateFormatter.cpp
)

# 헤더 파일 목록
set(HEADERS
    src/browser/BrowserWindow.h
    src/browser/WebView.h
    src/browser/TabManager.h
    src/ui/URLBar.h
    src/ui/NavigationBar.h
    src/ui/LoadingIndicator.h
    src/ui/BookmarkPanel.h
    src/ui/HistoryPanel.h
    src/models/Bookmark.h
    src/services/BookmarkService.h
    src/services/HistoryService.h
    src/services/StorageService.h
    src/services/SearchEngine.h
    src/utils/Logger.h
    src/utils/URLValidator.h
    src/utils/DateFormatter.h
)

# 실행 파일 생성
add_executable(webosbrowser ${SOURCES} ${HEADERS})

# Qt 라이브러리 링크
target_link_libraries(webosbrowser
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
)

# 컴파일 옵션
target_compile_options(webosbrowser PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

# 출력 디렉토리 설정
set_target_properties(webosbrowser PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# 테스트 활성화
enable_testing()

# Google Test (선택사항, 나중에 추가)
# add_subdirectory(tests)

# 설치 규칙 (webOS 패키징용)
install(TARGETS webosbrowser
    RUNTIME DESTINATION bin
)

install(DIRECTORY resources/
    DESTINATION share/webosbrowser/resources
)

install(FILES webos-meta/appinfo.json
    DESTINATION share/webosbrowser
)
