/**
 * @file BookmarkPanel.h
 * @brief 북마크 관리 패널 UI
 */

#pragma once

#include <QWidget>
#include <QListWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QDialog>
#include <QComboBox>
#include "../models/Bookmark.h"

namespace webosbrowser {

class BookmarkService;

/**
 * @brief 북마크 목록 및 관리 패널
 *
 * 북마크 목록 조회, 추가, 편집, 삭제, 폴더 관리 UI 제공
 * 리모컨 키 이벤트 처리 (방향키, 선택 버튼, 백 버튼)
 */
class BookmarkPanel : public QWidget {
    Q_OBJECT

public:
    explicit BookmarkPanel(BookmarkService* bookmarkService, QWidget *parent = nullptr);
    ~BookmarkPanel() override;

    BookmarkPanel(const BookmarkPanel&) = delete;
    BookmarkPanel& operator=(const BookmarkPanel&) = delete;

    /**
     * @brief 패널 표시
     */
    void show();

    /**
     * @brief 패널 숨김
     */
    void hide();

    /**
     * @brief 현재 페이지 정보 설정 (북마크 추가용)
     * @param url 현재 URL
     * @param title 현재 페이지 제목
     */
    void setCurrentPage(const QString& url, const QString& title);

signals:
    /**
     * @brief 북마크 선택 시그널 (페이지 열기)
     * @param url 북마크 URL
     * @param title 북마크 제목
     */
    void bookmarkSelected(const QString& url, const QString& title);

    /**
     * @brief 패널 닫기 시그널
     */
    void panelClosed();

protected:
    /**
     * @brief 키 이벤트 핸들러 (리모컨 입력 처리)
     * @param event 키 이벤트
     */
    void keyPressEvent(QKeyEvent* event) override;

private slots:
    /**
     * @brief 북마크 추가 버튼 클릭
     */
    void onAddBookmarkClicked();

    /**
     * @brief 북마크 편집 버튼 클릭
     */
    void onEditBookmarkClicked();

    /**
     * @brief 북마크 삭제 버튼 클릭
     */
    void onDeleteBookmarkClicked();

    /**
     * @brief 새 폴더 버튼 클릭
     */
    void onNewFolderClicked();

    /**
     * @brief 북마크 항목 더블클릭 (페이지 열기)
     * @param item 리스트 항목
     */
    void onBookmarkItemDoubleClicked(QListWidgetItem* item);

    /**
     * @brief 북마크 목록 갱신
     */
    void refreshBookmarks();

    /**
     * @brief 검색어 변경
     * @param text 검색어
     */
    void onSearchTextChanged(const QString& text);

private:
    BookmarkService* m_bookmarkService;  // 북마크 서비스

    // UI 컴포넌트
    QVBoxLayout* m_mainLayout;
    QHBoxLayout* m_headerLayout;
    QHBoxLayout* m_actionLayout;
    QLabel* m_titleLabel;
    QPushButton* m_closeButton;
    QLineEdit* m_searchInput;
    QPushButton* m_addButton;
    QPushButton* m_editButton;
    QPushButton* m_deleteButton;
    QPushButton* m_newFolderButton;
    QListWidget* m_bookmarkList;

    // 현재 페이지 정보
    QString m_currentUrl;
    QString m_currentTitle;

    // 현재 폴더 ID (빈 문자열이면 루트)
    QString m_currentFolderId;

    /**
     * @brief UI 초기화
     */
    void setupUI();

    /**
     * @brief 북마크 추가 다이얼로그 표시
     * @param editMode 편집 모드 (true: 편집, false: 추가)
     * @param bookmark 편집할 북마크 (편집 모드일 때)
     */
    void showBookmarkDialog(bool editMode = false, const Bookmark& bookmark = Bookmark());

    /**
     * @brief 폴더 추가 다이얼로그 표시
     */
    void showFolderDialog();

    /**
     * @brief 삭제 확인 다이얼로그 표시
     * @param message 확인 메시지
     * @param callback 확인 콜백
     */
    void showConfirmDialog(const QString& message, std::function<void()> callback);

    /**
     * @brief 토스트 메시지 표시
     * @param message 메시지
     */
    void showToast(const QString& message);

    /**
     * @brief 선택된 북마크 가져오기
     * @return 북마크 객체 (선택 없으면 기본 생성자)
     */
    Bookmark getSelectedBookmark() const;
};

/**
 * @brief 북마크 추가/편집 다이얼로그
 */
class BookmarkDialog : public QDialog {
    Q_OBJECT

public:
    explicit BookmarkDialog(bool editMode, const Bookmark& bookmark, const QVector<BookmarkFolder>& folders, QWidget* parent = nullptr);

    /**
     * @brief 입력된 북마크 데이터 가져오기
     * @return 북마크 객체
     */
    Bookmark getBookmark() const;

private:
    bool m_editMode;
    Bookmark m_bookmark;

    QLineEdit* m_titleInput;
    QLineEdit* m_urlInput;
    QComboBox* m_folderCombo;
    QLineEdit* m_descriptionInput;
    QPushButton* m_saveButton;
    QPushButton* m_cancelButton;

    void setupUI(const QVector<BookmarkFolder>& folders);
};

/**
 * @brief 폴더 추가/편집 다이얼로그
 */
class FolderDialog : public QDialog {
    Q_OBJECT

public:
    explicit FolderDialog(bool editMode, const BookmarkFolder& folder, QWidget* parent = nullptr);

    /**
     * @brief 입력된 폴더 이름 가져오기
     * @return 폴더 이름
     */
    QString getFolderName() const;

private:
    bool m_editMode;
    BookmarkFolder m_folder;

    QLineEdit* m_nameInput;
    QPushButton* m_saveButton;
    QPushButton* m_cancelButton;

    void setupUI();
};

} // namespace webosbrowser
