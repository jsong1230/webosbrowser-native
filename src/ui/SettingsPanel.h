/**
 * @file SettingsPanel.h
 * @brief 설정 패널 UI
 */

#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QRadioButton>
#include <QButtonGroup>
#include <QLineEdit>
#include <QPushButton>
#include <QPropertyAnimation>
#include <QKeyEvent>

namespace webosbrowser {

class SettingsService;
class BookmarkService;
class HistoryService;

/**
 * @class SettingsPanel
 * @brief 설정 패널 UI (오버레이)
 *
 * 검색 엔진 선택, 홈페이지 설정, 테마 변경, 브라우징 데이터 삭제 UI 제공.
 * 리모컨 방향키로 항목 간 네비게이션, 슬라이드 인/아웃 애니메이션 지원.
 */
class SettingsPanel : public QWidget {
    Q_OBJECT

public:
    explicit SettingsPanel(SettingsService *settingsService,
                           BookmarkService *bookmarkService,
                           HistoryService *historyService,
                           QWidget *parent = nullptr);
    ~SettingsPanel() override;

    SettingsPanel(const SettingsPanel&) = delete;
    SettingsPanel& operator=(const SettingsPanel&) = delete;

    /**
     * @brief 패널 표시 (슬라이드 인 애니메이션)
     */
    void showPanel();

    /**
     * @brief 패널 숨김 (슬라이드 아웃 애니메이션)
     */
    void hidePanel();

signals:
    /**
     * @brief 패널 닫기 시그널
     */
    void panelClosed();

protected:
    /**
     * @brief 키 이벤트 핸들러 (리모컨 Back 키 처리)
     * @param event 키 이벤트
     */
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    /**
     * @brief 검색 엔진 라디오 버튼 토글 슬롯
     * @param checked 체크 상태
     */
    void onSearchEngineRadioToggled(bool checked);

    /**
     * @brief 홈페이지 입력 완료 슬롯
     */
    void onHomepageEditingFinished();

    /**
     * @brief 테마 라디오 버튼 토글 슬롯
     * @param checked 체크 상태
     */
    void onThemeRadioToggled(bool checked);

    /**
     * @brief 북마크 전체 삭제 버튼 클릭 슬롯
     */
    void onClearBookmarksClicked();

    /**
     * @brief 히스토리 전체 삭제 버튼 클릭 슬롯
     */
    void onClearHistoryClicked();

    /**
     * @brief 닫기 버튼 클릭 슬롯
     */
    void onCloseButtonClicked();

    /**
     * @brief 설정 로드 완료 슬롯 (UI 업데이트)
     */
    void onSettingsLoaded();

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
     * @brief 포커스 순서 설정
     */
    void setupFocusOrder();

    /**
     * @brief 현재 설정을 UI에 반영
     */
    void loadCurrentSettings();

    /**
     * @brief 확인 다이얼로그 표시
     * @param message 메시지
     * @return 사용자가 "예" 선택 시 true
     */
    bool showConfirmDialog(const QString &message);

    /**
     * @brief 토스트 메시지 표시
     * @param message 메시지
     */
    void showToast(const QString &message);

    SettingsService *settingsService_;
    BookmarkService *bookmarkService_;
    HistoryService *historyService_;

    // UI 컴포넌트
    QVBoxLayout *mainLayout_;
    QWidget *contentWidget_;
    QVBoxLayout *contentLayout_;
    QFormLayout *formLayout_;

    // 헤더 (제목 + 닫기 버튼)
    QHBoxLayout *headerLayout_;
    QLabel *titleLabel_;
    QPushButton *closeButton_;

    // 검색 엔진 섹션
    QLabel *searchEngineLabel_;
    QHBoxLayout *searchEngineLayout_;
    QRadioButton *googleRadio_;
    QRadioButton *naverRadio_;
    QRadioButton *bingRadio_;
    QRadioButton *duckduckgoRadio_;
    QButtonGroup *searchEngineGroup_;

    // 홈페이지 섹션
    QLabel *homepageLabel_;
    QLineEdit *homepageInput_;
    QLabel *homepageErrorLabel_;

    // 테마 섹션
    QLabel *themeLabel_;
    QHBoxLayout *themeLayout_;
    QRadioButton *darkThemeRadio_;
    QRadioButton *lightThemeRadio_;
    QButtonGroup *themeGroup_;

    // 브라우징 데이터 삭제 섹션
    QLabel *clearDataLabel_;
    QHBoxLayout *clearDataLayout_;
    QPushButton *clearBookmarksButton_;
    QPushButton *clearHistoryButton_;

    // 애니메이션
    QPropertyAnimation *slideAnimation_;
};

} // namespace webosbrowser
