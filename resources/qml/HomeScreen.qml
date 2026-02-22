/**
 * HomeScreen.qml
 * 홈 화면 컴포넌트
 *
 * 자주 방문하는 사이트 그리드 표시
 * 리모컨 화살표키로 네비게이션
 * Qt 5.12.3 호환
 */

import QtQuick 2.12
import QtQuick.Controls 2.12

Rectangle {
    id: root
    color: "#1a1a2e"

    signal siteSelected(string url)

    // 자주 방문하는 사이트 데이터
    property var quickSites: [
        { name: "Google", url: "https://www.google.com", letter: "G", color: "#4285f4" },
        { name: "YouTube", url: "https://www.youtube.com", letter: "Y", color: "#ff0000" },
        { name: "Naver", url: "https://www.naver.com", letter: "N", color: "#03c75a" },
        { name: "Wikipedia", url: "https://ko.wikipedia.org", letter: "W", color: "#636363" },
        { name: "Netflix", url: "https://www.netflix.com", letter: "N", color: "#e50914" },
        { name: "GitHub", url: "https://github.com", letter: "G", color: "#24292e" }
    ]

    // 현재 포커스된 카드 인덱스
    property int focusedIndex: 0

    // 열/행 수
    property int cols: 3

    Column {
        anchors.centerIn: parent
        spacing: 48

        // 타이틀
        Column {
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 12

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: "webOS 브라우저"
                color: "white"
                font.pixelSize: 44
                font.bold: true
            }

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: "URL 바에 주소를 입력하거나 아래에서 사이트를 선택하세요"
                color: "#707080"
                font.pixelSize: 20
            }
        }

        // 사이트 그리드
        Grid {
            id: siteGrid
            anchors.horizontalCenter: parent.horizontalCenter
            columns: root.cols
            spacing: 24

            // 키보드 네비게이션 (그리드 레벨)
            focus: root.visible

            Keys.onLeftPressed: {
                if (root.focusedIndex > 0) {
                    root.focusedIndex--;
                    updateFocus();
                }
                event.accepted = true;
            }

            Keys.onRightPressed: {
                if (root.focusedIndex < root.quickSites.length - 1) {
                    root.focusedIndex++;
                    updateFocus();
                }
                event.accepted = true;
            }

            Keys.onUpPressed: {
                if (root.focusedIndex >= root.cols) {
                    root.focusedIndex -= root.cols;
                    updateFocus();
                }
                event.accepted = true;
            }

            Keys.onDownPressed: {
                if (root.focusedIndex + root.cols < root.quickSites.length) {
                    root.focusedIndex += root.cols;
                    updateFocus();
                }
                event.accepted = true;
            }

            Keys.onReturnPressed: {
                root.siteSelected(root.quickSites[root.focusedIndex].url);
                event.accepted = true;
            }

            function updateFocus() {
                // 포커스 업데이트는 focusedIndex 바인딩으로 처리
            }

            Repeater {
                id: siteRepeater
                model: root.quickSites

                delegate: Rectangle {
                    id: siteCard
                    width: 210
                    height: 130
                    radius: 14
                    color: root.focusedIndex === index ? modelData.color : "#16213e"
                    border.color: root.focusedIndex === index ? modelData.color : "#0f3460"
                    border.width: root.focusedIndex === index ? 3 : 1

                    Behavior on color {
                        ColorAnimation { duration: 150 }
                    }

                    Behavior on border.color {
                        ColorAnimation { duration: 150 }
                    }

                    // 포커스 시 살짝 커지는 효과
                    scale: root.focusedIndex === index ? 1.05 : 1.0
                    Behavior on scale {
                        NumberAnimation { duration: 150; easing.type: Easing.OutBack }
                    }

                    Column {
                        anchors.centerIn: parent
                        spacing: 12

                        // 사이트 아이콘 (원형)
                        Rectangle {
                            width: 56
                            height: 56
                            radius: 28
                            color: root.focusedIndex === index ? "white" : modelData.color
                            anchors.horizontalCenter: parent.horizontalCenter

                            Text {
                                anchors.centerIn: parent
                                text: modelData.letter
                                color: root.focusedIndex === index ? modelData.color : "white"
                                font.pixelSize: 26
                                font.bold: true
                            }

                            Behavior on color {
                                ColorAnimation { duration: 150 }
                            }
                        }

                        // 사이트 이름
                        Text {
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: modelData.name
                            color: "white"
                            font.pixelSize: 19
                            font.bold: root.focusedIndex === index
                        }
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            root.focusedIndex = index;
                            root.siteSelected(modelData.url);
                        }
                        onEntered: root.focusedIndex = index
                    }
                }
            }
        }

        // 키 힌트
        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: "화살표키로 선택, OK(Return)로 열기"
            color: "#404050"
            font.pixelSize: 16
        }
    }
}
