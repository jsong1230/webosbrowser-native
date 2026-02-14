/**
 * @file SecurityClassifierTest.cpp
 * @brief SecurityClassifier 단위 테스트
 * @note Qt Test Framework 사용
 */

#include <QtTest/QtTest>
#include <QUrl>
#include <QString>
#include "../../src/utils/SecurityClassifier.h"

using namespace webosbrowser;

/**
 * @class SecurityClassifierTest
 * @brief SecurityClassifier 단위 테스트
 */
class SecurityClassifierTest : public QObject {
    Q_OBJECT

private slots:
    // HTTPS 테스트
    void testHttpsUrl() {
        QUrl url("https://www.google.com");
        SecurityLevel level = SecurityClassifier::classify(url);
        QCOMPARE(level, SecurityLevel::Secure);
    }

    void testHttpsWithPort() {
        QUrl url("https://example.com:443/path");
        SecurityLevel level = SecurityClassifier::classify(url);
        QCOMPARE(level, SecurityLevel::Secure);
    }

    // HTTP 공인 IP 테스트
    void testHttpPublicDomain() {
        QUrl url("http://example.com");
        SecurityLevel level = SecurityClassifier::classify(url);
        QCOMPARE(level, SecurityLevel::Insecure);
    }

    void testHttpPublicIp() {
        QUrl url("http://8.8.8.8");
        SecurityLevel level = SecurityClassifier::classify(url);
        QCOMPARE(level, SecurityLevel::Insecure);
    }

    // HTTP 로컬 테스트
    void testHttpLocalhost() {
        QUrl url("http://localhost");
        SecurityLevel level = SecurityClassifier::classify(url);
        QCOMPARE(level, SecurityLevel::Local);
    }

    void testHttpLocalhost127() {
        QUrl url("http://127.0.0.1");
        SecurityLevel level = SecurityClassifier::classify(url);
        QCOMPARE(level, SecurityLevel::Local);
    }

    void testHttpLocalhostIPv6() {
        QUrl url("http://[::1]");
        SecurityLevel level = SecurityClassifier::classify(url);
        QCOMPARE(level, SecurityLevel::Local);
    }

    void testHttpPrivateIp10() {
        QUrl url("http://10.0.0.1");
        SecurityLevel level = SecurityClassifier::classify(url);
        QCOMPARE(level, SecurityLevel::Local);
    }

    void testHttpPrivateIp172() {
        QUrl url("http://172.16.0.1");
        SecurityLevel level = SecurityClassifier::classify(url);
        QCOMPARE(level, SecurityLevel::Local);
    }

    void testHttpPrivateIp192() {
        QUrl url("http://192.168.1.1");
        SecurityLevel level = SecurityClassifier::classify(url);
        QCOMPARE(level, SecurityLevel::Local);
    }

    // File URL 테스트
    void testFileUrl() {
        QUrl url("file:///home/user/document.html");
        SecurityLevel level = SecurityClassifier::classify(url);
        QCOMPARE(level, SecurityLevel::Local);
    }

    // Unknown 테스트
    void testAboutBlank() {
        QUrl url("about:blank");
        SecurityLevel level = SecurityClassifier::classify(url);
        QCOMPARE(level, SecurityLevel::Unknown);
    }

    void testEmptyUrl() {
        QUrl url;
        SecurityLevel level = SecurityClassifier::classify(url);
        QCOMPARE(level, SecurityLevel::Unknown);
    }

    // 문자열 변환 테스트
    void testSecurityLevelToString() {
        QCOMPARE(SecurityClassifier::securityLevelToString(SecurityLevel::Secure), QString("Secure"));
        QCOMPARE(SecurityClassifier::securityLevelToString(SecurityLevel::Insecure), QString("Insecure"));
        QCOMPARE(SecurityClassifier::securityLevelToString(SecurityLevel::Local), QString("Local"));
        QCOMPARE(SecurityClassifier::securityLevelToString(SecurityLevel::Unknown), QString("Unknown"));
    }

    // 툴팁 변환 테스트
    void testSecurityLevelToTooltip() {
        QCOMPARE(SecurityClassifier::securityLevelToTooltip(SecurityLevel::Secure), QString("보안 연결"));
        QCOMPARE(SecurityClassifier::securityLevelToTooltip(SecurityLevel::Insecure), QString("안전하지 않음"));
        QCOMPARE(SecurityClassifier::securityLevelToTooltip(SecurityLevel::Local), QString("로컬 연결"));
        QCOMPARE(SecurityClassifier::securityLevelToTooltip(SecurityLevel::Unknown), QString(""));
    }

    // 아이콘 경로 테스트
    void testSecurityLevelToIconPath() {
        QVERIFY(!SecurityClassifier::securityLevelToIconPath(SecurityLevel::Secure).isEmpty());
        QVERIFY(SecurityClassifier::securityLevelToIconPath(SecurityLevel::Secure).contains("lock_secure"));
        QVERIFY(SecurityClassifier::securityLevelToIconPath(SecurityLevel::Insecure).contains("lock_insecure"));
        QVERIFY(SecurityClassifier::securityLevelToIconPath(SecurityLevel::Local).contains("info"));
        QVERIFY(SecurityClassifier::securityLevelToIconPath(SecurityLevel::Unknown).isEmpty());
    }

    // isLocalAddress 직접 테스트
    void testIsLocalAddressLocalhost() {
        QVERIFY(SecurityClassifier::isLocalAddress("localhost"));
        QVERIFY(SecurityClassifier::isLocalAddress("127.0.0.1"));
        QVERIFY(SecurityClassifier::isLocalAddress("::1"));
    }

    void testIsLocalAddressPrivateIp() {
        QVERIFY(SecurityClassifier::isLocalAddress("10.0.0.0"));
        QVERIFY(SecurityClassifier::isLocalAddress("10.255.255.255"));
        QVERIFY(SecurityClassifier::isLocalAddress("172.16.0.0"));
        QVERIFY(SecurityClassifier::isLocalAddress("172.31.255.255"));
        QVERIFY(SecurityClassifier::isLocalAddress("192.168.0.0"));
        QVERIFY(SecurityClassifier::isLocalAddress("192.168.255.255"));
    }

    void testIsLocalAddressPublicIp() {
        QVERIFY(!SecurityClassifier::isLocalAddress("8.8.8.8"));
        QVERIFY(!SecurityClassifier::isLocalAddress("1.2.3.4"));
        QVERIFY(!SecurityClassifier::isLocalAddress("example.com"));
    }

    // 엣지 케이스
    void testCaseSensitivity() {
        QUrl url1("HTTPS://EXAMPLE.COM");
        QUrl url2("http://LOCALHOST");
        QCOMPARE(SecurityClassifier::classify(url1), SecurityLevel::Secure);
        QCOMPARE(SecurityClassifier::classify(url2), SecurityLevel::Local);
    }

    void testUrlWithPathAndQuery() {
        QUrl url("https://example.com/path?query=value#fragment");
        SecurityLevel level = SecurityClassifier::classify(url);
        QCOMPARE(level, SecurityLevel::Secure);
    }
};

// 테스트 메인
QTEST_MAIN(SecurityClassifierTest)
#include "SecurityClassifierTest.moc"
