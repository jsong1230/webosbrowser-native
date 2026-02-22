/**
 * UrlBar.qml
 * URL 입력 바 컴포넌트
 *
 * 리모컨으로 포커스 이동, Return/OK로 탐색 실행
 * HTTPS → 녹색 자물쇠, HTTP → 빨간색 자물쇠
 */

import QtQuick 2.12

Rectangle {
    id: root
    color: "#16213e"

    // 외부에서 읽기/쓰기 가능한 URL 텍스트
    property string urlText: webOSController.currentUrl
    property bool hasFocus: urlInput.activeFocus

    signal navigateRequested(string url)
    signal focusStateChanged(bool focused)

    // URL 보안 등급 계산
    property string securityLevel: {
        var url = urlText;
        if (url.startsWith("https://")) return "secure";
        if (url.startsWith("http://")) return "insecure";
        return "none";
    }

    // 외부에서 포커스 요청
    function requestFocus() {
        urlInput.forceActiveFocus();
        urlInput.selectAll();
    }

    // ─────────────────────────────────────────────────────────
    // 상단 구분선
    // ─────────────────────────────────────────────────────────
    Rectangle {
        anchors {
            bottom: parent.bottom
            left: parent.left
            right: parent.right
        }
        height: 1
        color: "#0f3460"
    }

    Row {
        anchors {
            fill: parent
            leftMargin: 16
            rightMargin: 16
            topMargin: 10
            bottomMargin: 10
        }
        spacing: 12

        // ─────────────────────────────────────────────────────
        // 보안 아이콘 (자물쇠)
        // ─────────────────────────────────────────────────────
        Rectangle {
            id: securityIcon
            width: 48
            height: parent.height
            color: "transparent"
            anchors.verticalCenter: undefined

            Text {
                anchors.centerIn: parent
                text: {
                    if (root.securityLevel === "secure") return "\uD83D\uDD12";   // 잠금 이모지
                    if (root.securityLevel === "insecure") return "\u26A0";        // 경고 이모지
                    return "\uD83C\uDF10";                                          // 지구 이모지
                }
                font.pixelSize: 22
                color: {
                    if (root.securityLevel === "secure") return "#2ecc71";
                    if (root.securityLevel === "insecure") return "#e74c3c";
                    return "#808090";
                }
            }
        }

        // ─────────────────────────────────────────────────────
        // URL 입력 필드
        // ─────────────────────────────────────────────────────
        Rectangle {
            id: inputContainer
            width: parent.width - securityIcon.width - goButton.width - parent.spacing * 2
            height: parent.height
            radius: 8
            color: urlInput.activeFocus ? "#1a2a4a" : "#0f1a30"
            border.color: urlInput.activeFocus ? "#4a90e2" : "#0f3460"
            border.width: urlInput.activeFocus ? 2 : 1

            Behavior on border.color {
                ColorAnimation { duration: 150 }
            }

            TextInput {
                id: urlInput
                anchors {
                    left: parent.left
                    right: parent.right
                    verticalCenter: parent.verticalCenter
                    leftMargin: 12
                    rightMargin: 12
                }
                text: root.urlText
                color: "white"
                font.pixelSize: 20
                selectionColor: "#4a90e2"
                selectedTextColor: "white"
                clip: true

                // 플레이스홀더
                Text {
                    anchors.verticalCenter: parent.verticalCenter
                    text: "URL 입력 또는 검색어..."
                    color: "#505060"
                    font.pixelSize: 20
                    visible: urlInput.text === "" && !urlInput.activeFocus
                }

                // Return/OK 키 → 탐색 실행
                Keys.onReturnPressed: {
                    var inputText = text.trim();
                    if (inputText !== "") {
                        root.urlText = inputText;
                        root.navigateRequested(inputText);
                        urlInput.focus = false;
                    }
                    event.accepted = true;
                }

                Keys.onEscapePressed: {
                    text = root.urlText;
                    urlInput.focus = false;
                    event.accepted = true;
                }

                // 포커스 시 전체 선택
                onActiveFocusChanged: {
                    if (activeFocus) {
                        selectAll();
                    }
                    root.focusStateChanged(activeFocus);
                }
            }
        }

        // ─────────────────────────────────────────────────────
        // 이동 버튼
        // ─────────────────────────────────────────────────────
        Rectangle {
            id: goButton
            width: 80
            height: parent.height
            radius: 8
            color: goButtonFocus.activeFocus ? "#e94560" : "#0f3460"
            border.color: goButtonFocus.activeFocus ? "#e94560" : "transparent"
            border.width: 2

            Behavior on color {
                ColorAnimation { duration: 150 }
            }

            Text {
                anchors.centerIn: parent
                text: "이동"
                color: "white"
                font.pixelSize: 18
                font.bold: true
            }

            FocusScope {
                id: goButtonFocus
                anchors.fill: parent

                KeyNavigation.left: urlInput

                Keys.onReturnPressed: {
                    var inputText = urlInput.text.trim();
                    if (inputText !== "") {
                        root.urlText = inputText;
                        root.navigateRequested(inputText);
                    }
                    event.accepted = true;
                }
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    var inputText = urlInput.text.trim();
                    if (inputText !== "") {
                        root.urlText = inputText;
                        root.navigateRequested(inputText);
                    }
                }
            }
        }
    }

    // 로딩 진행 표시 바 (하단)
    Rectangle {
        anchors {
            bottom: parent.bottom
            left: parent.left
        }
        height: 3
        color: "#4a90e2"
        visible: webOSController.isLoading

        SequentialAnimation on width {
            running: webOSController.isLoading
            loops: Animation.Infinite
            NumberAnimation { from: 0; to: root.width * 0.7; duration: 800; easing.type: Easing.InOutQuad }
            NumberAnimation { from: root.width * 0.7; to: root.width; duration: 400; easing.type: Easing.InOutQuad }
            NumberAnimation { from: root.width; to: 0; duration: 100 }
        }
    }
}
