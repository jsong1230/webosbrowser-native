# 테스트 CMakeLists.txt

# Google Test - FetchContent로 자동 다운로드
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
FetchContent_MakeAvailable(googletest)

# 또는 시스템 GTest 사용 시도
find_package(GTest QUIET)

# 테스트 포함 디렉토리
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
    ${GTEST_INCLUDE_DIRS}
)

# Qt 자동 처리
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# 단위 테스트
set(UNIT_TESTS
    unit/URLValidatorTest.cpp
    unit/URLBarTest.cpp
    unit/TabManagerTest.cpp
)

# 통합 테스트
set(INTEGRATION_TESTS
    integration/BrowserWindowIntegrationTest.cpp
    integration/BrowserWindowTabManagerIntegrationTest.cpp
)

# 모든 테스트 목록
set(ALL_TESTS ${UNIT_TESTS} ${INTEGRATION_TESTS})

# 테스트 실행 파일 생성
add_executable(webosbrowser_tests
    ${ALL_TESTS}
    # 소스 파일들 (테스트에서 필요한 클래스들)
    ../src/utils/URLValidator.cpp
    ../src/ui/URLBar.cpp
    ../src/browser/BrowserWindow.cpp
    ../src/browser/WebView.cpp
    ../src/browser/TabManager.cpp
    ../src/ui/NavigationBar.cpp
    ../src/ui/BookmarkPanel.cpp
    ../src/services/BookmarkService.cpp
    ../src/services/HistoryService.cpp
    ../src/services/StorageService.cpp
    ../src/utils/Logger.cpp
)

# Qt 라이브러리 링크
target_link_libraries(webosbrowser_tests
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    gtest
    gtest_main
)

# 테스트 등록
add_test(NAME webosbrowser_tests COMMAND webosbrowser_tests)

# 출력 디렉토리
set_target_properties(webosbrowser_tests PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/tests
)
