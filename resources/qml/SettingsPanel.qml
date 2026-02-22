/**
 * SettingsPanel.qml
 * 설정 패널 컴포넌트
 *
 * 오른쪽 슬라이드인 패널
 * 버전 정보, 리모컨 단축키 안내 표시
 * Qt 5.12.3 호환 (인라인 component 키워드 미사용)
 */

import QtQuick 2.12

Rectangle {
    id: root
    color: "#16213e"
    clip: true

    signal closeRequested()

    // 왼쪽 구분선
    Rectangle {
        anchors {
            top: parent.top
            bottom: parent.bottom
            left: parent.left
        }
        width: 2
        color: "#0f3460"
    }

    Column {
        anchors.fill: parent
        spacing: 0

        // ─────────────────────────────────────────────────────
        // 헤더
        // ─────────────────────────────────────────────────────
        Rectangle {
            width: parent.width
            height: 72
            color: "#0f3460"

            Text {
                anchors {
                    left: parent.left
                    verticalCenter: parent.verticalCenter
                    leftMargin: 20
                }
                text: "설정"
                color: "white"
                font.pixelSize: 26
                font.bold: true
            }

            Rectangle {
                anchors {
                    right: parent.right
                    verticalCenter: parent.verticalCenter
                    rightMargin: 16
                }
                width: 48
                height: 48
                radius: 8
                color: closeFocus.activeFocus ? "#e94560" : "transparent"
                border.color: closeFocus.activeFocus ? "#e94560" : "#4a90e2"
                border.width: 1

                Text {
                    anchors.centerIn: parent
                    text: "X"
                    color: "white"
                    font.pixelSize: 20
                    font.bold: true
                }

                FocusScope {
                    id: closeFocus
                    anchors.fill: parent
                    Keys.onReturnPressed: root.closeRequested()
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: root.closeRequested()
                }
            }
        }

        // ─────────────────────────────────────────────────────
        // 설정 내용
        // ─────────────────────────────────────────────────────
        Flickable {
            width: parent.width
            height: root.height - 72
            contentHeight: contentCol.height + 40
            clip: true

            Column {
                id: contentCol
                width: parent.width
                spacing: 0

                // 앱 정보 섹션
                Rectangle {
                    width: parent.width; height: 48; color: "#0d1a30"
                    Text {
                        anchors { left: parent.left; verticalCenter: parent.verticalCenter; leftMargin: 20 }
                        text: "앱 정보"; color: "#4a90e2"; font.pixelSize: 15; font.bold: true
                    }
                }

                Repeater {
                    model: [
                        { label: "앱 이름", value: "webOS Browser Native" },
                        { label: "버전", value: "1.0.0" },
                        { label: "플랫폼", value: "webOS 6.3.1 (ARMv7)" },
                        { label: "Qt 버전", value: "5.12.3" },
                        { label: "렌더링 방식", value: "Luna Service (외부)" },
                        { label: "해상도", value: "1920 x 1080" }
                    ]

                    delegate: Rectangle {
                        width: contentCol.width
                        height: 52
                        color: "transparent"

                        Text {
                            anchors { left: parent.left; verticalCenter: parent.verticalCenter; leftMargin: 20 }
                            text: modelData.label; color: "#9090a0"; font.pixelSize: 17
                        }
                        Text {
                            anchors { right: parent.right; verticalCenter: parent.verticalCenter; rightMargin: 20 }
                            text: modelData.value; color: "#d0d0e0"; font.pixelSize: 17
                            elide: Text.ElideRight
                            width: parent.width * 0.55
                            horizontalAlignment: Text.AlignRight
                        }
                        Rectangle {
                            anchors { bottom: parent.bottom; left: parent.left; right: parent.right; leftMargin: 20; rightMargin: 20 }
                            height: 1; color: "#0f2050"
                        }
                    }
                }

                Rectangle { width: parent.width; height: 1; color: "#0f3460" }

                // 리모컨 단축키 섹션
                Rectangle {
                    width: parent.width; height: 48; color: "#0d1a30"
                    Text {
                        anchors { left: parent.left; verticalCenter: parent.verticalCenter; leftMargin: 20 }
                        text: "리모컨 단축키"; color: "#4a90e2"; font.pixelSize: 15; font.bold: true
                    }
                }

                Repeater {
                    model: [
                        { label: "Red 버튼", value: "북마크 패널" },
                        { label: "Green 버튼", value: "히스토리 패널" },
                        { label: "Yellow 버튼", value: "홈으로 이동" },
                        { label: "Blue 버튼", value: "설정 패널" },
                        { label: "Back / Escape", value: "뒤로 / 패널 닫기" },
                        { label: "OK / Return", value: "선택 / 탐색" },
                        { label: "F5", value: "새로고침" }
                    ]

                    delegate: Rectangle {
                        width: contentCol.width
                        height: 52
                        color: "transparent"

                        Text {
                            anchors { left: parent.left; verticalCenter: parent.verticalCenter; leftMargin: 20 }
                            text: modelData.label; color: "#9090a0"; font.pixelSize: 17
                        }
                        Text {
                            anchors { right: parent.right; verticalCenter: parent.verticalCenter; rightMargin: 20 }
                            text: modelData.value; color: "#d0d0e0"; font.pixelSize: 17
                        }
                        Rectangle {
                            anchors { bottom: parent.bottom; left: parent.left; right: parent.right; leftMargin: 20; rightMargin: 20 }
                            height: 1; color: "#0f2050"
                        }
                    }
                }

                Rectangle { width: parent.width; height: 1; color: "#0f3460" }

                // 시스템 정보 섹션
                Rectangle {
                    width: parent.width; height: 48; color: "#0d1a30"
                    Text {
                        anchors { left: parent.left; verticalCenter: parent.verticalCenter; leftMargin: 20 }
                        text: "시스템 정보"; color: "#4a90e2"; font.pixelSize: 15; font.bold: true
                    }
                }

                Repeater {
                    model: [
                        { label: "Wayland 디스플레이", value: "wayland-0" },
                        { label: "XDG 런타임", value: "/tmp/xdg" },
                        { label: "Luna Send 경로", value: "/usr/bin/luna-send" },
                        { label: "브라우저 앱 ID", value: "com.webos.app.browser" }
                    ]

                    delegate: Rectangle {
                        width: contentCol.width
                        height: 52
                        color: "transparent"

                        Text {
                            anchors { left: parent.left; verticalCenter: parent.verticalCenter; leftMargin: 20 }
                            text: modelData.label; color: "#9090a0"; font.pixelSize: 17
                        }
                        Text {
                            anchors { right: parent.right; verticalCenter: parent.verticalCenter; rightMargin: 20 }
                            text: modelData.value; color: "#d0d0e0"; font.pixelSize: 14
                            elide: Text.ElideRight
                            width: parent.width * 0.6
                            horizontalAlignment: Text.AlignRight
                        }
                        Rectangle {
                            anchors { bottom: parent.bottom; left: parent.left; right: parent.right; leftMargin: 20; rightMargin: 20 }
                            height: 1; color: "#0f2050"
                        }
                    }
                }
            }

        }
    }
}
