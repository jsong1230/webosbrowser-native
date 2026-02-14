/**
 * @file KeyCodeMappingTest.cpp
 * @brief 리모컨 키 코드 매핑 단위 테스트 (F-13)
 * @note Qt Test Framework 사용
 */

#include <QtTest/QtTest>
#include <QKeyEvent>
#include "../../src/utils/KeyCodeConstants.h"

using namespace webosbrowser;

/**
 * @class KeyCodeMappingTest
 * @brief KeyCodeConstants 단위 테스트
 */
class KeyCodeMappingTest : public QObject {
    Q_OBJECT

private slots:
    // 채널 버튼 테스트
    void testChannelUpCode() {
        QCOMPARE(KeyCode::CHANNEL_UP, 427);
    }

    void testChannelDownCode() {
        QCOMPARE(KeyCode::CHANNEL_DOWN, 428);
    }

    // 컬러 버튼 테스트
    void testRedCode() {
        QCOMPARE(KeyCode::RED, 403);
    }

    void testGreenCode() {
        QCOMPARE(KeyCode::GREEN, 405);
    }

    void testYellowCode() {
        QCOMPARE(KeyCode::YELLOW, 406);
    }

    void testBlueCode() {
        QCOMPARE(KeyCode::BLUE, 407);
    }

    // 숫자 버튼 테스트
    void testNum1Code() {
        QCOMPARE(KeyCode::NUM_1, 49);
    }

    void testNum2Code() {
        QCOMPARE(KeyCode::NUM_2, 50);
    }

    void testNum3Code() {
        QCOMPARE(KeyCode::NUM_3, 51);
    }

    void testNum4Code() {
        QCOMPARE(KeyCode::NUM_4, 52);
    }

    void testNum5Code() {
        QCOMPARE(KeyCode::NUM_5, 53);
    }

    // 설정 버튼 테스트
    void testMenuCode() {
        QCOMPARE(KeyCode::MENU, 457);
    }

    void testSettingsCode() {
        QCOMPARE(KeyCode::SETTINGS, 18);
    }

    // 재생 컨트롤 테스트 (M3 이후)
    void testPlayCode() {
        QCOMPARE(KeyCode::PLAY, 415);
    }

    void testPauseCode() {
        QCOMPARE(KeyCode::PAUSE, 19);
    }

    void testFastForwardCode() {
        QCOMPARE(KeyCode::FAST_FORWARD, 417);
    }

    void testRewindCode() {
        QCOMPARE(KeyCode::REWIND, 412);
    }

    // Back 버튼 테스트
    void testBackCode() {
        QCOMPARE(KeyCode::BACK, 27);
    }

    // 범위 체크
    void testNumberButtonRange() {
        QVERIFY(KeyCode::NUM_1 >= 49 && KeyCode::NUM_1 <= 53);
        QVERIFY(KeyCode::NUM_2 >= 49 && KeyCode::NUM_2 <= 53);
        QVERIFY(KeyCode::NUM_3 >= 49 && KeyCode::NUM_3 <= 53);
        QVERIFY(KeyCode::NUM_4 >= 49 && KeyCode::NUM_4 <= 53);
        QVERIFY(KeyCode::NUM_5 >= 49 && KeyCode::NUM_5 <= 53);
    }

    void testColorButtonRange() {
        QVERIFY(KeyCode::RED >= 403 && KeyCode::RED <= 407);
        QVERIFY(KeyCode::GREEN >= 403 && KeyCode::GREEN <= 407);
        QVERIFY(KeyCode::YELLOW >= 403 && KeyCode::YELLOW <= 407);
        QVERIFY(KeyCode::BLUE >= 403 && KeyCode::BLUE <= 407);
    }

    // 중복 체크 (같은 키 코드가 없어야 함)
    void testNoDuplicateKeyCodes() {
        // 채널 버튼
        QVERIFY(KeyCode::CHANNEL_UP != KeyCode::CHANNEL_DOWN);

        // 컬러 버튼 (RED, GREEN, YELLOW, BLUE는 모두 다름)
        QVERIFY(KeyCode::RED != KeyCode::GREEN);
        QVERIFY(KeyCode::RED != KeyCode::YELLOW);
        QVERIFY(KeyCode::RED != KeyCode::BLUE);
        QVERIFY(KeyCode::GREEN != KeyCode::YELLOW);
        QVERIFY(KeyCode::GREEN != KeyCode::BLUE);
        QVERIFY(KeyCode::YELLOW != KeyCode::BLUE);

        // 숫자 버튼 (1~5는 모두 다름)
        QVERIFY(KeyCode::NUM_1 != KeyCode::NUM_2);
        QVERIFY(KeyCode::NUM_1 != KeyCode::NUM_3);
        QVERIFY(KeyCode::NUM_1 != KeyCode::NUM_4);
        QVERIFY(KeyCode::NUM_1 != KeyCode::NUM_5);
        QVERIFY(KeyCode::NUM_2 != KeyCode::NUM_3);
        QVERIFY(KeyCode::NUM_2 != KeyCode::NUM_4);
        QVERIFY(KeyCode::NUM_2 != KeyCode::NUM_5);
        QVERIFY(KeyCode::NUM_3 != KeyCode::NUM_4);
        QVERIFY(KeyCode::NUM_3 != KeyCode::NUM_5);
        QVERIFY(KeyCode::NUM_4 != KeyCode::NUM_5);
    }

    // 키 분류 헬퍼 함수 테스트
    void testKeyClassification() {
        // 채널 버튼
        QVERIFY(KeyCode::CHANNEL_UP == 427 || KeyCode::CHANNEL_UP == 428);
        QVERIFY(KeyCode::CHANNEL_DOWN == 427 || KeyCode::CHANNEL_DOWN == 428);
        QVERIFY(KeyCode::CHANNEL_UP != KeyCode::CHANNEL_DOWN);

        // 컬러 버튼 (범위 403-407)
        QVERIFY(KeyCode::RED >= 403 && KeyCode::RED <= 407);
        QVERIFY(KeyCode::GREEN >= 403 && KeyCode::GREEN <= 407);
        QVERIFY(KeyCode::YELLOW >= 403 && KeyCode::YELLOW <= 407);
        QVERIFY(KeyCode::BLUE >= 403 && KeyCode::BLUE <= 407);

        // 숫자 버튼 (ASCII '1'~'5' = 49~53)
        QVERIFY(KeyCode::NUM_1 >= 49 && KeyCode::NUM_1 <= 53);
        QVERIFY(KeyCode::NUM_5 >= 49 && KeyCode::NUM_5 <= 53);

        // Back 버튼 (ESC = 27)
        QVERIFY(KeyCode::BACK == 27);
    }

    // 키 이벤트 시뮬레이션 검증
    void testKeyEventMapping() {
        // QKeyEvent 객체 생성 (실제 리모컨 입력 시뮬레이션)
        // Yellow 버튼 (406)
        QKeyEvent yellowEvent(QEvent::KeyPress, KeyCode::YELLOW, Qt::NoModifier);
        QCOMPARE(yellowEvent.key(), 406);

        // Channel Up (427)
        QKeyEvent channelUpEvent(QEvent::KeyPress, KeyCode::CHANNEL_UP, Qt::NoModifier);
        QCOMPARE(channelUpEvent.key(), 427);

        // Number 1 (49)
        QKeyEvent num1Event(QEvent::KeyPress, KeyCode::NUM_1, Qt::NoModifier);
        QCOMPARE(num1Event.key(), 49);
    }

    // 리모컨 키 조합 검증
    void testKeyHandlingLogic() {
        // 탭 전환 (채널 버튼)
        int tabIndex = 0;
        if (KeyCode::CHANNEL_UP == 427) {
            tabIndex = (tabIndex - 1 + 5) % 5;  // 순환 이전 탭
            QCOMPARE(tabIndex, 4);  // 0 → 4
        }

        // 탭 선택 (숫자 버튼)
        int numKey = KeyCode::NUM_3;  // 51 ('3')
        if (numKey >= KeyCode::NUM_1 && numKey <= KeyCode::NUM_5) {
            int selectedTab = numKey - KeyCode::NUM_1;
            QCOMPARE(selectedTab, 2);  // 탭 2 (0-indexed)
        }
    }
};

// 테스트 메인
QTEST_MAIN(KeyCodeMappingTest)
#include "KeyCodeMappingTest.moc"
