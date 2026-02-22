/**
 * main.qml
 * webOS 프로젝터 브라우저 메인 화면
 *
 * 전체화면 1920x1080, 어두운 테마 (#1a1a2e)
 * 리모컨 전용 UI (마우스 없음)
 * Qt 5.12 호환 및 Qt 6 Connections 문법 사용
 */

import QtQuick 2.12
import QtQuick.Window 2.12

Window {
    id: root
    width: 1920
    height: 1080
    visible: true
    title: "webOS Browser"
    color: "#1a1a2e"

    // 에러/상태 메시지 감시 (Qt6 function 문법)
    Connections {
        target: webOSController

        function onErrorMessageChanged() {
            var msg = webOSController.errorMessage;
            if (msg !== "") {
                mainItem.statusMessage = msg;
                mainItem.statusIsError = true;
                statusTimer.restart();
            }
        }

        function onPageOpened(url) {
            mainItem.statusMessage = "브라우저에서 열림";
            mainItem.statusIsError = false;
            statusTimer.restart();
        }

        function onCurrentUrlChanged(url) {
            urlBar.urlText = url;
        }
    }

    Timer {
        id: statusTimer
        interval: 4000
        onTriggered: mainItem.statusMessage = ""
    }

    // ─────────────────────────────────────────────────────────
    // 메인 Item (Keys 처리는 Item에서 가능)
    // ─────────────────────────────────────────────────────────
    Item {
        id: mainItem
        anchors.fill: parent
        focus: true

        // 상태 관리
        property string activePanel: ""
        property bool urlBarFocused: false
        property string statusMessage: ""
        property bool statusIsError: false

        Keys.onPressed: {
            // 패널이 열려있으면 Back으로 패널 닫기
            if (mainItem.activePanel !== "") {
                if (event.key === Qt.Key_Back || event.key === Qt.Key_Escape) {
                    mainItem.activePanel = "";
                    event.accepted = true;
                    return;
                }
            }

            if (event.key === 403) {
                mainItem.activePanel = (mainItem.activePanel === "bookmark") ? "" : "bookmark";
                event.accepted = true;
            } else if (event.key === 404) {
                mainItem.activePanel = (mainItem.activePanel === "history") ? "" : "history";
                event.accepted = true;
            } else if (event.key === 405) {
                mainItem.activePanel = "";
                webOSController.navigateTo(settingsService.homepage());
                event.accepted = true;
            } else if (event.key === 406) {
                mainItem.activePanel = (mainItem.activePanel === "settings") ? "" : "settings";
                event.accepted = true;
            } else if (event.key === Qt.Key_Back || event.key === Qt.Key_Escape) {
                if (mainItem.urlBarFocused) {
                    mainItem.urlBarFocused = false;
                    navBar.forceActiveFocus();
                } else {
                    webOSController.goBack();
                }
                event.accepted = true;
            } else if (event.key === Qt.Key_F5) {
                webOSController.reload();
                event.accepted = true;
            }
        }

        // ─────────────────────────────────────────────────────
        // 메인 레이아웃
        // ─────────────────────────────────────────────────────
        Column {
            id: mainLayout
            anchors.fill: parent
            spacing: 0

            // 상단 URL 바
            UrlBar {
                id: urlBar
                width: parent.width
                height: 80
                onNavigateRequested: {
                    mainItem.activePanel = "";
                    webOSController.navigateTo(url);
                }
                onFocusStateChanged: {
                    mainItem.urlBarFocused = focused;
                }
            }

            // 가운데 컨텐츠 영역
            Rectangle {
                id: contentArea
                width: parent.width
                height: root.height - urlBar.height - navBar.height
                color: "#1a1a2e"

                // 홈 화면 (URL이 없을 때)
                HomeScreen {
                    id: homeScreen
                    anchors.fill: parent
                    visible: webOSController.currentUrl === ""
                    onSiteSelected: webOSController.navigateTo(url)
                }

                // URL 탐색 상태 화면 (URL 있을 때)
                Item {
                    anchors.fill: parent
                    visible: webOSController.currentUrl !== ""

                    Column {
                        anchors.centerIn: parent
                        spacing: 32

                        // 로딩 인디케이터
                        Rectangle {
                            width: 80
                            height: 80
                            radius: 40
                            color: "#0f3460"
                            anchors.horizontalCenter: parent.horizontalCenter
                            visible: webOSController.isLoading

                            RotationAnimator on rotation {
                                running: webOSController.isLoading
                                from: 0
                                to: 360
                                duration: 1200
                                loops: Animation.Infinite
                            }

                            Rectangle {
                                width: 18
                                height: 18
                                radius: 9
                                color: "#e94560"
                                anchors {
                                    top: parent.top
                                    horizontalCenter: parent.horizontalCenter
                                    topMargin: 5
                                }
                            }
                        }

                        Text {
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: webOSController.isLoading
                                  ? "시스템 브라우저에서 열리는 중..."
                                  : "외부 브라우저에서 열림"
                            color: "#a0a0c0"
                            font.pixelSize: 24
                        }

                        // 현재 URL 표시
                        Rectangle {
                            width: 960
                            height: 64
                            radius: 10
                            color: "#16213e"
                            border.color: "#0f3460"
                            border.width: 2
                            anchors.horizontalCenter: parent.horizontalCenter

                            Text {
                                anchors {
                                    left: parent.left
                                    right: parent.right
                                    verticalCenter: parent.verticalCenter
                                    leftMargin: 20
                                    rightMargin: 20
                                }
                                text: webOSController.currentUrl
                                color: "#e0e0ff"
                                font.pixelSize: 20
                                elide: Text.ElideMiddle
                            }
                        }

                        Text {
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: "Back=뒤로   Red=북마크   Green=히스토리   Yellow=홈   Blue=설정"
                            color: "#505060"
                            font.pixelSize: 17
                        }
                    }
                }

                // 상태/에러 메시지 오버레이
                Rectangle {
                    anchors {
                        bottom: parent.bottom
                        left: parent.left
                        right: parent.right
                        margins: 20
                        bottomMargin: 12
                    }
                    height: 56
                    radius: 8
                    color: mainItem.statusIsError ? "#6b0000" : "#003322"
                    visible: mainItem.statusMessage !== ""
                    opacity: 0.94

                    Behavior on opacity {
                        NumberAnimation { duration: 300 }
                    }

                    Row {
                        anchors {
                            fill: parent
                            leftMargin: 20
                            rightMargin: 20
                        }
                        spacing: 10

                        Text {
                            anchors.verticalCenter: parent.verticalCenter
                            text: mainItem.statusIsError ? "!" : "v"
                            color: mainItem.statusIsError ? "#ff6b6b" : "#2ecc71"
                            font.pixelSize: 22
                            font.bold: true
                        }

                        Text {
                            anchors.verticalCenter: parent.verticalCenter
                            text: mainItem.statusMessage
                            color: "white"
                            font.pixelSize: 18
                            elide: Text.ElideRight
                            width: parent.width - 40
                        }
                    }
                }
            }

            // 하단 네비게이션 바
            NavBar {
                id: navBar
                width: parent.width
                height: 72
                onBackClicked: webOSController.goBack()
                onForwardClicked: webOSController.goForward()
                onReloadClicked: webOSController.reload()
                onHomeClicked: {
                    mainItem.activePanel = "";
                    webOSController.navigateTo(settingsService.homepage());
                }
                onUrlBarFocusRequested: urlBar.requestFocus()
            }
        }

        // ─────────────────────────────────────────────────────
        // 패널 딤 오버레이
        // ─────────────────────────────────────────────────────
        Rectangle {
            anchors.fill: parent
            color: "#000000"
            opacity: mainItem.activePanel !== "" ? 0.55 : 0.0
            visible: opacity > 0.01

            Behavior on opacity {
                NumberAnimation { duration: 250 }
            }

            MouseArea {
                anchors.fill: parent
                enabled: mainItem.activePanel !== ""
                onClicked: mainItem.activePanel = ""
            }
        }

        // ─────────────────────────────────────────────────────
        // 북마크 패널 (오른쪽 슬라이드인)
        // ─────────────────────────────────────────────────────
        BookmarkPanel {
            id: bookmarkPanel
            y: 0
            height: root.height
            width: 500
            x: mainItem.activePanel === "bookmark" ? root.width - width : root.width

            Behavior on x {
                NumberAnimation { duration: 280; easing.type: Easing.OutCubic }
            }

            onUrlSelected: {
                mainItem.activePanel = "";
                webOSController.navigateTo(url);
            }
            onCloseRequested: mainItem.activePanel = ""
        }

        // ─────────────────────────────────────────────────────
        // 히스토리 패널 (오른쪽 슬라이드인)
        // ─────────────────────────────────────────────────────
        HistoryPanel {
            id: historyPanel
            y: 0
            height: root.height
            width: 500
            x: mainItem.activePanel === "history" ? root.width - width : root.width

            Behavior on x {
                NumberAnimation { duration: 280; easing.type: Easing.OutCubic }
            }

            onUrlSelected: {
                mainItem.activePanel = "";
                webOSController.navigateTo(url);
            }
            onCloseRequested: mainItem.activePanel = ""
        }

        // ─────────────────────────────────────────────────────
        // 설정 패널 (오른쪽 슬라이드인)
        // ─────────────────────────────────────────────────────
        SettingsPanel {
            id: settingsPanelQml
            y: 0
            height: root.height
            width: 500
            x: mainItem.activePanel === "settings" ? root.width - width : root.width

            Behavior on x {
                NumberAnimation { duration: 280; easing.type: Easing.OutCubic }
            }

            onCloseRequested: mainItem.activePanel = ""
        }

        // ─────────────────────────────────────────────────────
        // 리모컨 버튼 힌트 (하단 좌측)
        // ─────────────────────────────────────────────────────
        Row {
            anchors {
                bottom: parent.bottom
                left: parent.left
                leftMargin: 20
                bottomMargin: 10
            }
            spacing: 16
            visible: mainItem.activePanel === ""

            Repeater {
                model: [
                    { color: "#e94560", letter: "R", text: "북마크" },
                    { color: "#2ecc71", letter: "G", text: "히스토리" },
                    { color: "#f39c12", letter: "Y", text: "홈" },
                    { color: "#3498db", letter: "B", text: "설정" }
                ]

                Row {
                    spacing: 6

                    Rectangle {
                        width: 22
                        height: 22
                        radius: 4
                        color: modelData.color
                        anchors.verticalCenter: parent.verticalCenter

                        Text {
                            anchors.centerIn: parent
                            text: modelData.letter
                            color: "white"
                            font.pixelSize: 12
                            font.bold: true
                        }
                    }

                    Text {
                        anchors.verticalCenter: parent.verticalCenter
                        text: modelData.text
                        color: "#606070"
                        font.pixelSize: 14
                    }
                }
            }
        }
    }
}
