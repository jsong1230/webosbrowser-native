/**
 * @file HomePage.h
 * @brief 즐겨찾기 홈 화면 컴포넌트
 */

#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QScrollArea>
#include <QLabel>
#include <QPushButton>
#include <QVector>
#include <QKeyEvent>
#include <QShowEvent>
#include "../models/Bookmark.h"

namespace webosbrowser {

class BookmarkService;
class BookmarkCard;

/**
 * @class HomePage
 * @brief 즐겨찾기 홈 화면 (about:favorites)
 *
 * 북마크를 4열 그리드로 표시하며 리모컨 방향키로 탐색합니다.
 * 북마크 0개 시 빈 상태 UI를 표시합니다.
 */
class HomePage : public QWidget {
    Q_OBJECT

public:
    explicit HomePage(BookmarkService *bookmarkService, QWidget *parent = nullptr);
    ~HomePage() override;

    HomePage(const HomePage&) = delete;
    HomePage& operator=(const HomePage&) = delete;

    /**
     * @brief 북마크 목록 새로고침
     *
     * BookmarkService에서 최신 데이터를 조회하여 그리드를 재구성합니다.
     */
    void refreshBookmarks();

signals:
    /**
     * @brief 북마크 선택 시그널
     * @param url 선택된 북마크 URL
     */
    void bookmarkSelected(const QString &url);

    /**
     * @brief 북마크 추가 요청 시그널 (Red 버튼)
     */
    void bookmarkAddRequested();

    /**
     * @brief 설정 요청 시그널 (Menu 버튼)
     */
    void settingsRequested();

    /**
     * @brief 뒤로 가기 요청 시그널 (Back 버튼)
     */
    void backRequested();

protected:
    /**
     * @brief 키 입력 이벤트 (리모컨 단축키)
     * @param event 키 이벤트
     *
     * 방향키: 그리드 포커스 이동
     * Enter: 북마크 선택
     * Back/Escape: 이전 페이지
     * Red(0x193): 북마크 추가
     * Menu(Qt::Key_Menu): 설정
     */
    void keyPressEvent(QKeyEvent *event) override;

    /**
     * @brief 위젯 표시 이벤트
     * @param event 표시 이벤트
     *
     * HomePage가 표시될 때마다 자동으로 refreshBookmarks() 호출
     */
    void showEvent(QShowEvent *event) override;

private slots:
    /**
     * @brief 북마크 추가됨 슬롯
     * @param bookmark 추가된 북마크
     */
    void onBookmarkAdded(const Bookmark &bookmark);

    /**
     * @brief 북마크 삭제됨 슬롯
     * @param id 북마크 ID
     */
    void onBookmarkDeleted(const QString &id);

    /**
     * @brief 북마크 업데이트됨 슬롯
     * @param id 북마크 ID
     */
    void onBookmarkUpdated(const QString &id);

    /**
     * @brief 북마크 데이터 로드 완료 슬롯
     * @param success 성공 여부
     * @param bookmarks 북마크 리스트
     */
    void onBookmarksLoaded(bool success, const QVector<Bookmark> &bookmarks);

    /**
     * @brief BookmarkCard 클릭 슬롯
     * @param url 북마크 URL
     */
    void onCardClicked(const QString &url);

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
     * @brief 스타일시트 적용
     */
    void applyStyles();

    /**
     * @brief 빈 상태 위젯 생성
     * @return 빈 상태 위젯 포인터
     */
    QWidget* createEmptyStateWidget();

    /**
     * @brief 그리드 뷰 재구성
     *
     * 기존 카드를 모두 삭제하고 bookmarks_를 기반으로 새 카드 생성
     */
    void rebuildGrid();

    /**
     * @brief 북마크 표시 (그리드 뷰)
     * @param bookmarks 표시할 북마크 목록 (최대 12개)
     */
    void displayBookmarks(const QVector<Bookmark> &bookmarks);

    /**
     * @brief 빈 상태 UI 표시
     */
    void displayEmptyState();

    /**
     * @brief 그리드 포커스 이동 (왼쪽)
     */
    void moveFocusLeft();

    /**
     * @brief 그리드 포커스 이동 (오른쪽)
     */
    void moveFocusRight();

    /**
     * @brief 그리드 포커스 이동 (위)
     */
    void moveFocusUp();

    /**
     * @brief 그리드 포커스 이동 (아래)
     */
    void moveFocusDown();

    /**
     * @brief 현재 포커스된 카드 활성화 (Enter 키)
     */
    void activateCurrentCard();

    // 서비스
    BookmarkService *bookmarkService_;

    // UI 컴포넌트
    QVBoxLayout *mainLayout_;             // 메인 레이아웃
    QLabel *titleLabel_;                  // 타이틀 "즐겨찾기"
    QScrollArea *contentScrollArea_;      // 스크롤 영역 (M4 대비)
    QWidget *contentContainer_;           // 스크롤 컨테이너
    QGridLayout *gridLayout_;             // 4열 그리드
    QWidget *emptyStateWidget_;           // 빈 상태 UI
    QLabel *footerLabel_;                 // 푸터 (단축키 안내)

    // 북마크 카드 목록
    QVector<BookmarkCard*> cards_;

    // 데이터 캐시
    QVector<Bookmark> bookmarks_;

    // 포커스 관리
    int currentFocusIndex_;

    // 상수
    static constexpr int MAX_BOOKMARKS = 12;  // 최대 표시 개수
    static constexpr int GRID_COLUMNS = 4;    // 그리드 열 개수
};

} // namespace webosbrowser
