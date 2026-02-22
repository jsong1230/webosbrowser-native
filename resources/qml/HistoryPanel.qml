/**
 * HistoryPanel.qml
 * 히스토리 패널 컴포넌트
 *
 * 오른쪽에서 슬라이드인. HistoryService C++ 백엔드 연결.
 * 최근 방문 순서로 목록 표시.
 */

import QtQuick 2.12
import QtQuick.Controls 2.12

Rectangle {
    id: root
    color: "#16213e"
    clip: true

    signal urlSelected(string url)
    signal closeRequested()

    property int selectedIndex: 0

    // 히스토리 데이터 (C++ HistoryService에서 로드)
    property var historyItems: []

    // 슬라이드인 감지
    onXChanged: {
        if (x < parent.width && parent !== null) {
            loadHistory();
            selectedIndex = 0;
            if (historyList.count > 0) {
                historyList.currentIndex = 0;
                historyList.forceActiveFocus();
            }
        }
    }

    function loadHistory() {
        // C++ HistoryService.getAllHistory() 결과를 QML 모델로 변환
        // 실제 구현 시 HistoryService의 시그널/슬롯 연동 필요
        // 현재는 빈 배열 (서비스 연동 후 채워짐)
        historyItems = [];
    }

    function formatTime(dateStr) {
        if (!dateStr) return "";
        // 간단한 시간 포맷팅
        var date = new Date(dateStr);
        var now = new Date();
        var diffMs = now - date;
        var diffMins = Math.floor(diffMs / 60000);
        var diffHours = Math.floor(diffMs / 3600000);
        var diffDays = Math.floor(diffMs / 86400000);

        if (diffMins < 1) return "방금 전";
        if (diffMins < 60) return diffMins + "분 전";
        if (diffHours < 24) return diffHours + "시간 전";
        if (diffDays < 7) return diffDays + "일 전";
        return date.toLocaleDateString("ko-KR");
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

            Row {
                anchors {
                    fill: parent
                    leftMargin: 20
                    rightMargin: 16
                }
                spacing: 12

                Text {
                    anchors.verticalCenter: parent.verticalCenter
                    text: "\uD83D\uDCC5"  // 달력 이모지
                    font.pixelSize: 24
                }

                Text {
                    anchors.verticalCenter: parent.verticalCenter
                    text: "방문 기록"
                    color: "white"
                    font.pixelSize: 24
                    font.bold: true
                }

                // 전체 삭제 버튼
                Rectangle {
                    id: clearAllBtn
                    anchors.verticalCenter: parent.verticalCenter
                    width: 100
                    height: 40
                    radius: 8
                    color: clearFocus.activeFocus ? "#8b0000" : "#3a1a1a"
                    border.color: clearFocus.activeFocus ? "#e94560" : "#4a2020"
                    border.width: 1
                    visible: historyList.count > 0

                    Text {
                        anchors.centerIn: parent
                        text: "전체 삭제"
                        color: "#e94560"
                        font.pixelSize: 14
                    }

                    FocusScope {
                        id: clearFocus
                        anchors.fill: parent
                        Keys.onReturnPressed: {
                            // historyService.deleteAllHistory() 호출
                            root.historyItems = [];
                            event.accepted = true;
                        }
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: root.historyItems = []
                    }
                }

                Item { width: 1; height: 1 }

                // 닫기 버튼
                Rectangle {
                    anchors.verticalCenter: parent.verticalCenter
                    width: 48
                    height: 48
                    radius: 8
                    color: closeFocus.activeFocus ? "#e94560" : "transparent"
                    border.color: closeFocus.activeFocus ? "#e94560" : "#4a90e2"
                    border.width: 1

                    Text {
                        anchors.centerIn: parent
                        text: "✕"
                        color: "white"
                        font.pixelSize: 20
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
        }

        Rectangle {
            width: parent.width
            height: 1
            color: "#0f3460"
        }

        // ─────────────────────────────────────────────────────
        // 히스토리 목록
        // ─────────────────────────────────────────────────────
        ListView {
            id: historyList
            width: parent.width
            height: root.height - 73
            clip: true
            model: root.historyItems.length > 0 ? root.historyItems : defaultHistory
            currentIndex: root.selectedIndex

            // 개발/데모용 기본 히스토리
            property var defaultHistory: [
                { title: "Google", url: "https://www.google.com", visitedAt: "" },
                { title: "YouTube", url: "https://www.youtube.com", visitedAt: "" },
                { title: "Naver - 기본 검색", url: "https://www.naver.com", visitedAt: "" }
            ]

            // 빈 상태 메시지
            Item {
                anchors.centerIn: parent
                visible: root.historyItems.length === 0 && historyList.model === root.historyItems
                width: parent.width - 40
                height: 200

                Column {
                    anchors.centerIn: parent
                    spacing: 16

                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: "\uD83D\uDCC5"
                        font.pixelSize: 48
                    }

                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: "방문 기록이 없습니다"
                        color: "#505060"
                        font.pixelSize: 20
                    }

                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: "URL을 탐색하면 자동으로 기록됩니다"
                        color: "#404050"
                        font.pixelSize: 16
                    }
                }
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
                width: historyList.width
                height: 72
                color: historyList.currentIndex === index ? "#1a2060" : "transparent"
                border.color: historyList.currentIndex === index ? "#4a90e2" : "transparent"
                border.width: historyList.currentIndex === index ? 2 : 0

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

                    // 방문 아이콘
                    Rectangle {
                        width: 40
                        height: 40
                        radius: 6
                        color: "#0f3460"
                        anchors.verticalCenter: parent.verticalCenter

                        Text {
                            anchors.centerIn: parent
                            text: modelData.title ? modelData.title.charAt(0).toUpperCase() : "?"
                            color: "#6090c0"
                            font.pixelSize: 18
                            font.bold: true
                        }
                    }

                    Column {
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 4
                        width: parent.width - 100

                        Text {
                            text: modelData.title || "제목 없음"
                            color: "white"
                            font.pixelSize: 18
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

                    // 방문 시각
                    Text {
                        anchors.verticalCenter: parent.verticalCenter
                        text: root.formatTime(modelData.visitedAt)
                        color: "#404060"
                        font.pixelSize: 13
                        width: 60
                        horizontalAlignment: Text.AlignRight
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
                        historyList.currentIndex = index;
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
