/**
 * @file HistoryPanel.h
 * @brief 히스토리 패널 UI 헤더
 */

#pragma once

#include <QWidget>
#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QString>
#include "services/HistoryService.h"

namespace webosbrowser {

// Forward declarations
class HistoryService;

/**
 * @class HistoryPanel
 * @brief 히스토리 목록을 표시하고 관리하는 UI 패널
 *
 * 우측 슬라이드 오버레이 패널로 표시되며, 날짜별 그룹화된
 * 히스토리 목록을 제공합니다. 검색, 삭제, 실행 기능 포함.
 */
class HistoryPanel : public QWidget {
    Q_OBJECT

public:
    /**
     * @brief 생성자
     * @param historyService 히스토리 서비스
     * @param parent 부모 위젯 (기본값: nullptr)
     */
    explicit HistoryPanel(HistoryService *historyService, QWidget *parent = nullptr);

    /**
     * @brief 소멸자
     */
    ~HistoryPanel() override;

    // 복사 생성자 및 대입 연산자 삭제 (QObject 규칙)
    HistoryPanel(const HistoryPanel&) = delete;
    HistoryPanel& operator=(const HistoryPanel&) = delete;

    /**
     * @brief 패널 표시
     */
    void showPanel();

    /**
     * @brief 패널 숨기기
     */
    void hidePanel();

    /**
     * @brief 패널 표시 상태 토글
     */
    void togglePanel();

    /**
     * @brief 패널 표시 여부 반환
     * @return 표시 중이면 true
     */
    bool isVisible() const;

signals:
    /**
     * @brief 패널 닫기 요청 시그널
     */
    void closeRequested();

    /**
     * @brief 히스토리 선택 시그널 (페이지 열기)
     * @param url 선택한 URL
     * @param title 페이지 제목
     */
    void historySelected(const QString &url, const QString &title);

    /**
     * @brief 히스토리 삭제 완료 시그널
     * @param count 삭제된 항목 개수
     */
    void historyDeleted(int count);

private slots:
    /**
     * @brief 히스토리 목록 갱신
     */
    void refreshHistoryList();

    /**
     * @brief 검색어 입력 처리
     * @param query 검색어
     */
    void onSearchTextChanged(const QString &query);

    /**
     * @brief 히스토리 항목 더블클릭 처리 (페이지 열기)
     * @param item 클릭한 항목
     */
    void onHistoryItemDoubleClicked(QListWidgetItem *item);

    /**
     * @brief 히스토리 항목 컨텍스트 메뉴 표시
     * @param pos 메뉴 위치
     */
    void onHistoryItemContextMenu(const QPoint &pos);

    /**
     * @brief 개별 히스토리 삭제
     */
    void onDeleteHistoryItem();

    /**
     * @brief 전체 히스토리 삭제
     */
    void onDeleteAllHistory();

    /**
     * @brief 패널 닫기 버튼 클릭
     */
    void onCloseButtonClicked();

private:
    /**
     * @brief UI 초기화
     */
    void setupUI();

    /**
     * @brief 시그널/슬롯 연결
     */
    void setupConnections();

    /**
     * @brief 날짜별 그룹화된 히스토리 목록 렌더링
     */
    void renderGroupedHistory();

    /**
     * @brief 빈 히스토리 메시지 표시
     */
    void showEmptyMessage();

    /**
     * @brief 히스토리 항목을 QListWidgetItem으로 생성
     * @param entry 히스토리 항목
     * @return QListWidgetItem
     */
    QListWidgetItem* createHistoryItem(const HistoryEntry &entry);

    /**
     * @brief 날짜 그룹 헤더를 QListWidgetItem으로 생성
     * @param groupName 그룹 이름
     * @return QListWidgetItem
     */
    QListWidgetItem* createDateGroupHeader(const QString &groupName);

    /**
     * @brief 선택한 히스토리 항목의 ID 반환
     * @return 히스토리 ID (없으면 빈 문자열)
     */
    QString getSelectedHistoryId() const;

protected:
    /**
     * @brief 키 이벤트 처리 (리모컨 백 버튼)
     * @param event 키 이벤트
     */
    void keyPressEvent(QKeyEvent *event) override;

private:
    HistoryService *historyService_;    ///< 히스토리 서비스

    // UI 컴포넌트
    QVBoxLayout *mainLayout_;           ///< 메인 레이아웃
    QLabel *titleLabel_;                ///< 타이틀 라벨
    QLineEdit *searchEdit_;             ///< 검색 입력 필드
    QPushButton *closeButton_;          ///< 닫기 버튼
    QPushButton *deleteAllButton_;      ///< 모두 삭제 버튼
    QListWidget *historyListWidget_;    ///< 히스토리 리스트 위젯
    QLabel *emptyMessageLabel_;         ///< 빈 히스토리 메시지 라벨

    // 상태
    QString currentSearchQuery_;        ///< 현재 검색어
    bool isVisible_;                    ///< 패널 표시 상태

    // 스타일 상수
    static const int PANEL_WIDTH = 600;             ///< 패널 너비
    static const int ITEM_HEIGHT = 80;              ///< 히스토리 항목 높이
    static const int HEADER_HEIGHT = 60;            ///< 날짜 헤더 높이
};

} // namespace webosbrowser
