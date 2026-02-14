/**
 * @file KeyCodeConstants.h
 * @brief webOS 리모컨 키 코드 상수 정의
 *
 * LG webOS 프로젝터 HU715QW 리모컨의 keyCode 매핑.
 * 기기 및 펌웨어 버전에 따라 달라질 수 있으므로
 * 실제 기기에서 QKeyEvent::key() 값을 로그로 확인 후 수정.
 *
 * Qt::Key 값과 webOS keyCode가 동일한 경우가 많지만,
 * 일부 특수 키는 기기별로 다를 수 있습니다.
 */

#pragma once

namespace webosbrowser {
namespace KeyCode {

// ============================================================================
// 채널 버튼 (Channel Buttons)
// ============================================================================

/**
 * @brief 채널 올리기 버튼
 * Qt::Key_ChannelUp (427)
 */
constexpr int CHANNEL_UP = 427;

/**
 * @brief 채널 내리기 버튼
 * Qt::Key_ChannelDown (428)
 */
constexpr int CHANNEL_DOWN = 428;

// ============================================================================
// 컬러 버튼 (Color Buttons)
// ============================================================================

/**
 * @brief Red 버튼 (북마크 패널)
 * Qt::Key_Red (403 또는 404)
 */
constexpr int RED = 403;

/**
 * @brief Green 버튼 (히스토리 패널)
 * Qt::Key_Green (405)
 * 주의: YELLOW와 keyCode가 중복될 수 있음 (기기 의존)
 */
constexpr int GREEN = 405;

/**
 * @brief Yellow 버튼 (다운로드 패널)
 * Qt::Key_Yellow (406)
 * 주의: GREEN과 keyCode가 중복될 수 있음 (기기 의존)
 */
constexpr int YELLOW = 406;

/**
 * @brief Blue 버튼 (새 탭 생성)
 * Qt::Key_Blue (407)
 */
constexpr int BLUE = 407;

// ============================================================================
// 숫자 버튼 (Number Buttons)
// ============================================================================

/**
 * @brief 숫자 1 버튼 (첫 번째 탭 선택)
 * Qt::Key_1 (49)
 */
constexpr int NUM_1 = 49;

/**
 * @brief 숫자 2 버튼 (두 번째 탭 선택)
 * Qt::Key_2 (50)
 */
constexpr int NUM_2 = 50;

/**
 * @brief 숫자 3 버튼 (세 번째 탭 선택)
 * Qt::Key_3 (51)
 */
constexpr int NUM_3 = 51;

/**
 * @brief 숫자 4 버튼 (네 번째 탭 선택)
 * Qt::Key_4 (52)
 */
constexpr int NUM_4 = 52;

/**
 * @brief 숫자 5 버튼 (다섯 번째 탭 선택)
 * Qt::Key_5 (53)
 */
constexpr int NUM_5 = 53;

// ============================================================================
// 설정 버튼 (Settings/Menu Buttons)
// ============================================================================

/**
 * @brief Menu 버튼 (설정 패널)
 * Qt::Key_Menu (457)
 */
constexpr int MENU = 457;

/**
 * @brief Settings 버튼 (대체 키코드)
 * 일부 기기에서는 Menu 대신 Settings 키 사용 (18)
 */
constexpr int SETTINGS = 18;

// ============================================================================
// 재생 컨트롤 버튼 (Playback Control Buttons) - M3 이후
// ============================================================================

/**
 * @brief Play 버튼 (자동 스크롤 시작)
 * Qt::Key_MediaPlay (415)
 */
constexpr int PLAY = 415;

/**
 * @brief Pause 버튼 (자동 스크롤 멈춤)
 * Qt::Key_MediaPause (19)
 */
constexpr int PAUSE = 19;

/**
 * @brief FastForward 버튼 (페이지 끝으로)
 * Qt::Key_MediaNext (417)
 */
constexpr int FAST_FORWARD = 417;

/**
 * @brief Rewind 버튼 (페이지 맨 위로)
 * Qt::Key_MediaPrevious (412)
 */
constexpr int REWIND = 412;

// ============================================================================
// 기타 버튼 (Other Buttons)
// ============================================================================

/**
 * @brief Back 버튼 (뒤로가기 / 패널 닫기)
 * Qt::Key_Escape (27)
 * webOS에서는 Back 버튼이 Escape 키로 매핑됨
 */
constexpr int BACK = 27;

} // namespace KeyCode
} // namespace webosbrowser
