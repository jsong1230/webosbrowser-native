cmake_minimum_required(VERSION 3.16)
project(webosbrowser-tests VERSION 0.1.0 LANGUAGES CXX)

# C++ 표준 설정
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Qt 자동 처리 활성화
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Qt 패키지 찾기
find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets Test)

# Google Test - FetchContent로 자동 다운로드
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(googletest)

# 테스트 포함 디렉토리
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
)

# 단위 테스트 - TabManager (WebView 의존성 제거)
set(UNIT_TESTS
    ../tests/unit/TabManagerBasicTest.cpp
)

# TabManager 필요 소스
set(TEST_SOURCES
    ../src/browser/TabManager.cpp
    ../src/utils/Logger.cpp
    MockWebView.cpp
)

# 테스트 실행 파일
add_executable(webosbrowser_tests
    ${UNIT_TESTS}
    ${TEST_SOURCES}
)

# Qt 라이브러리 링크
target_link_libraries(webosbrowser_tests
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::Test
    gtest
    gtest_main
)

# 테스트 등록
enable_testing()
add_test(NAME TabManagerTest COMMAND webosbrowser_tests --gtest_color=yes)

# 출력 디렉토리
set_target_properties(webosbrowser_tests PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
