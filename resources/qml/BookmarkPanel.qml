/**
 * BookmarkPanel.qml
 * 북마크 패널 컴포넌트
 *
 * 오른쪽에서 슬라이드인. BookmarkService C++ 백엔드 연결.
 * 리모컨 화살표키로 항목 선택, Return/OK로 열기.
 */

import QtQuick 2.12
import QtQuick.Controls 2.12

Rectangle {
    id: root
    color: "#16213e"
    clip: true

    signal urlSelected(string url)
    signal closeRequested()

    // 북마크 데이터 모델
    property var bookmarks: []
    property int selectedIndex: 0

    // 패널 표시 시 북마크 로드
    onVisibleChanged: {
        if (visible) {
            loadBookmarks();
            selectedIndex = 0;
            if (bookmarks.length > 0) {
                bookmarkList.currentIndex = 0;
                bookmarkList.forceActiveFocus();
            }
        }
    }

    // x 위치 변화 감지로 슬라이드인 처리
    onXChanged: {
        if (x < parent.width && parent !== null) {
            loadBookmarks();
        }
    }

    function loadBookmarks() {
        // C++ BookmarkService에서 데이터 로드
        // BookmarkService는 비동기 콜백 기반이므로 여기서는 동기 대안 사용
        // 실제 구현 시 BookmarkService.getAllBookmarks()의 콜백을 QML 시그널로 연결
        bookmarks = []; // 실제 데이터로 대체
    }

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
        // 패널 헤더
        // ─────────────────────────────────────────────────────
        Rectangle {
            width: parent.width
            height: 72
            color: "#0f3460"

            // 헤더 제목 (왼쪽 정렬)
            Text {
                anchors {
                    left: parent.left
                    verticalCenter: parent.verticalCenter
                    leftMargin: 20
                }
                text: "북마크"
                color: "white"
                font.pixelSize: 24
                font.bold: true
            }

            // 닫기 버튼 (오른쪽 정렬)
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
        // 현재 페이지 북마크 추가 버튼
        // ─────────────────────────────────────────────────────
        Rectangle {
            width: parent.width
            height: 64
            color: addBtnFocus.activeFocus ? "#1a4a1a" : "#1a2a1a"
            border.color: addBtnFocus.activeFocus ? "#2ecc71" : "transparent"
            border.width: 1

            Behavior on color {
                ColorAnimation { duration: 150 }
            }

            visible: webOSController.currentUrl !== ""

            Row {
                anchors {
                    fill: parent
                    leftMargin: 20
                    rightMargin: 20
                }
                spacing: 12

                Text {
                    anchors.verticalCenter: parent.verticalCenter
                    text: "+"
                    color: "#2ecc71"
                    font.pixelSize: 28
                    font.bold: true
                }

                Column {
                    anchors.verticalCenter: parent.verticalCenter
                    spacing: 2

                    Text {
                        text: "현재 페이지 북마크 추가"
                        color: "#2ecc71"
                        font.pixelSize: 16
                        font.bold: true
                    }

                    Text {
                        text: webOSController.currentUrl
                        color: "#60c060"
                        font.pixelSize: 13
                        elide: Text.ElideRight
                        width: root.width - 80
                    }
                }
            }

            FocusScope {
                id: addBtnFocus
                anchors.fill: parent
                Keys.onReturnPressed: {
                    // 북마크 추가 로직 (BookmarkService 연동)
                    var url = webOSController.currentUrl;
                    var title = webOSController.pageTitle;
                    if (url !== "") {
                        // bookmarkService.addBookmark() 호출
                        // 현재는 시각적 피드백만 제공
                        addFeedback.visible = true;
                        addFeedbackTimer.restart();
                    }
                    event.accepted = true;
                }
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    var url = webOSController.currentUrl;
                    if (url !== "") {
                        addFeedback.visible = true;
                        addFeedbackTimer.restart();
                    }
                }
            }
        }

        // 추가 성공 피드백
        Rectangle {
            id: addFeedback
            width: parent.width
            height: 40
            color: "#1a4a1a"
            visible: false

            Text {
                anchors.centerIn: parent
                text: "북마크가 추가되었습니다"
                color: "#2ecc71"
                font.pixelSize: 16
            }

            Timer {
                id: addFeedbackTimer
                interval: 2000
                onTriggered: addFeedback.visible = false
            }
        }

        Rectangle {
            width: parent.width
            height: 1
            color: "#0f3460"
        }

        // ─────────────────────────────────────────────────────
        // 북마크 목록
        // ─────────────────────────────────────────────────────
        ListView {
            id: bookmarkList
            width: parent.width
            height: root.height - 72 - (webOSController.currentUrl !== "" ? 64 : 0) - 1
            clip: true
            model: root.bookmarks.length > 0 ? root.bookmarks : defaultBookmarks
            currentIndex: root.selectedIndex

            // 기본 북마크 (빈 경우)
            property var defaultBookmarks: [
                { title: "Google", url: "https://www.google.com" },
                { title: "YouTube", url: "https://www.youtube.com" },
                { title: "Naver", url: "https://www.naver.com" },
                { title: "Wikipedia (한국어)", url: "https://ko.wikipedia.org" }
            ]

            // 빈 상태 표시
            Text {
                anchors.centerIn: parent
                text: root.bookmarks.length === 0 ? "저장된 북마크가 없습니다\n\n현재 페이지를 북마크에 추가해보세요" : ""
                color: "#505060"
                font.pixelSize: 18
                horizontalAlignment: Text.AlignHCenter
                visible: root.bookmarks.length === 0 && bookmarkList.model === root.bookmarks
            }

            Keys.onUpPressed: {
                if (currentIndex > 0) {
                    currentIndex--;
                    root.selectedIndex = currentIndex;
                }
                event.accepted = true;
            }

            Keys.onDownPressed: {
                if (currentIndex < count - 1) {
                    currentIndex++;
                    root.selectedIndex = currentIndex;
                }
                event.accepted = true;
            }

            Keys.onReturnPressed: {
                var item = model[currentIndex];
                if (item && item.url) {
                    root.urlSelected(item.url);
                }
                event.accepted = true;
            }

            delegate: Rectangle {
                width: bookmarkList.width
                height: 72
                color: bookmarkList.currentIndex === index ? "#1a3060" : "transparent"
                border.color: bookmarkList.currentIndex === index ? "#4a90e2" : "transparent"
                border.width: bookmarkList.currentIndex === index ? 2 : 0

                Behavior on color {
                    ColorAnimation { duration: 120 }
                }

                Row {
                    anchors {
                        fill: parent
                        leftMargin: 20
                        rightMargin: 20
                    }
                    spacing: 12

                    // 파비콘 자리 (원형 아이콘)
                    Rectangle {
                        width: 40
                        height: 40
                        radius: 20
                        color: "#0f3460"
                        anchors.verticalCenter: parent.verticalCenter

                        Text {
                            anchors.centerIn: parent
                            text: modelData.title ? modelData.title.charAt(0).toUpperCase() : "?"
                            color: "#4a90e2"
                            font.pixelSize: 18
                            font.bold: true
                        }
                    }

                    Column {
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 4
                        width: parent.width - 60

                        Text {
                            text: modelData.title || "제목 없음"
                            color: "white"
                            font.pixelSize: 18
                            font.bold: true
                            elide: Text.ElideRight
                            width: parent.width
                        }

                        Text {
                            text: modelData.url || ""
                            color: "#608090"
                            font.pixelSize: 14
                            elide: Text.ElideMiddle
                            width: parent.width
                        }
                    }
                }

                // 구분선
                Rectangle {
                    anchors {
                        bottom: parent.bottom
                        left: parent.left
                        right: parent.right
                        leftMargin: 20
                        rightMargin: 20
                    }
                    height: 1
                    color: "#0f2050"
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        bookmarkList.currentIndex = index;
                        root.selectedIndex = index;
                        root.urlSelected(modelData.url);
                    }
                }
            }

            ScrollBar.vertical: ScrollBar {
                policy: ScrollBar.AsNeeded
            }
        }
    }
}
