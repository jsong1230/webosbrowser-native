/**
 * NavBar.qml
 * 하단 네비게이션 바 컴포넌트
 *
 * 뒤로/앞으로/새로고침/홈 버튼
 * 리모컨 포커스 네비게이션 지원
 * Qt 5.12.3 호환 (인라인 component 미사용)
 */

import QtQuick 2.12

Rectangle {
    id: root
    color: "#16213e"

    signal backClicked()
    signal forwardClicked()
    signal reloadClicked()
    signal homeClicked()
    signal urlBarFocusRequested()

    // 상단 구분선
    Rectangle {
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }
        height: 1
        color: "#0f3460"
    }

    Row {
        anchors.centerIn: parent
        spacing: 16

        // ─────────────────────────────────────────────────────
        // 뒤로 가기 버튼
        // ─────────────────────────────────────────────────────
        FocusScope {
            id: backBtn
            width: 100
            height: 60
            focus: true

            Keys.onReturnPressed: {
                if (webOSController.canGoBack) {
                    root.backClicked();
                }
                event.accepted = true;
            }
            Keys.onRightPressed: { forwardBtn.forceActiveFocus(); event.accepted = true; }
            Keys.onUpPressed: { root.urlBarFocusRequested(); event.accepted = true; }

            Rectangle {
                anchors.fill: parent
                radius: 8
                color: {
                    if (!webOSController.canGoBack) return "#0a0a1a";
                    if (parent.activeFocus) return "#0f3460";
                    return "transparent";
                }
                border.color: parent.activeFocus ? "#4a90e2" : "transparent"
                border.width: parent.activeFocus ? 2 : 0

                Behavior on color { ColorAnimation { duration: 120 } }

                Column {
                    anchors.centerIn: parent
                    spacing: 4
                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: "<"
                        color: webOSController.canGoBack ? "white" : "#404055"
                        font.pixelSize: 24
                        font.bold: true
                    }
                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: "뒤로"
                        color: webOSController.canGoBack ? "#a0a0c0" : "#404055"
                        font.pixelSize: 13
                    }
                }
            }
        }

        // ─────────────────────────────────────────────────────
        // 앞으로 가기 버튼
        // ─────────────────────────────────────────────────────
        FocusScope {
            id: forwardBtn
            width: 100
            height: 60

            Keys.onReturnPressed: {
                if (webOSController.canGoForward) {
                    root.forwardClicked();
                }
                event.accepted = true;
            }
            Keys.onLeftPressed: { backBtn.forceActiveFocus(); event.accepted = true; }
            Keys.onRightPressed: { reloadBtn.forceActiveFocus(); event.accepted = true; }

            Rectangle {
                anchors.fill: parent
                radius: 8
                color: {
                    if (!webOSController.canGoForward) return "#0a0a1a";
                    if (parent.activeFocus) return "#0f3460";
                    return "transparent";
                }
                border.color: parent.activeFocus ? "#4a90e2" : "transparent"
                border.width: parent.activeFocus ? 2 : 0

                Behavior on color { ColorAnimation { duration: 120 } }

                Column {
                    anchors.centerIn: parent
                    spacing: 4
                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: ">"
                        color: webOSController.canGoForward ? "white" : "#404055"
                        font.pixelSize: 24
                        font.bold: true
                    }
                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: "앞으로"
                        color: webOSController.canGoForward ? "#a0a0c0" : "#404055"
                        font.pixelSize: 13
                    }
                }
            }
        }

        // 구분선
        Rectangle {
            width: 1
            height: 40
            color: "#0f3460"
            anchors.verticalCenter: parent.verticalCenter
        }

        // ─────────────────────────────────────────────────────
        // 새로고침/중지 버튼
        // ─────────────────────────────────────────────────────
        FocusScope {
            id: reloadBtn
            width: 110
            height: 60

            Keys.onReturnPressed: {
                if (webOSController.isLoading) {
                    webOSController.stop();
                } else {
                    root.reloadClicked();
                }
                event.accepted = true;
            }
            Keys.onLeftPressed: { forwardBtn.forceActiveFocus(); event.accepted = true; }
            Keys.onRightPressed: { homeBtn.forceActiveFocus(); event.accepted = true; }

            Rectangle {
                anchors.fill: parent
                radius: 8
                color: parent.activeFocus ? "#0f3460" : "transparent"
                border.color: parent.activeFocus ? "#4a90e2" : "transparent"
                border.width: parent.activeFocus ? 2 : 0

                Behavior on color { ColorAnimation { duration: 120 } }

                Column {
                    anchors.centerIn: parent
                    spacing: 4
                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: webOSController.isLoading ? "■" : "↺"
                        color: "white"
                        font.pixelSize: 22
                    }
                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: webOSController.isLoading ? "중지" : "새로고침"
                        color: "#a0a0c0"
                        font.pixelSize: 13
                    }
                }
            }
        }

        // ─────────────────────────────────────────────────────
        // 홈 버튼
        // ─────────────────────────────────────────────────────
        FocusScope {
            id: homeBtn
            width: 100
            height: 60

            Keys.onReturnPressed: {
                root.homeClicked();
                event.accepted = true;
            }
            Keys.onLeftPressed: { reloadBtn.forceActiveFocus(); event.accepted = true; }

            Rectangle {
                anchors.fill: parent
                radius: 8
                color: parent.activeFocus ? "#0f3460" : "transparent"
                border.color: parent.activeFocus ? "#4a90e2" : "transparent"
                border.width: parent.activeFocus ? 2 : 0

                Behavior on color { ColorAnimation { duration: 120 } }

                Column {
                    anchors.centerIn: parent
                    spacing: 4
                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: "H"
                        color: "white"
                        font.pixelSize: 22
                        font.bold: true
                    }
                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: "홈"
                        color: "#a0a0c0"
                        font.pixelSize: 13
                    }
                }
            }
        }
    }

    // 현재 URL 표시 (우측)
    Text {
        anchors {
            right: parent.right
            verticalCenter: parent.verticalCenter
            rightMargin: 24
        }
        width: 600
        text: webOSController.currentUrl !== "" ? webOSController.currentUrl : "홈"
        color: "#505060"
        font.pixelSize: 14
        elide: Text.ElideMiddle
        horizontalAlignment: Text.AlignRight
    }
}
