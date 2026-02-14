/**
 * @file BookmarkPanel.cpp
 * @brief 북마크 관리 패널 UI 구현
 */

#include "BookmarkPanel.h"
#include "../services/BookmarkService.h"
#include "../utils/Logger.h"
#include <QMessageBox>
#include <QKeyEvent>
#include <QComboBox>
#include <QFormLayout>
#include <QTimer>

namespace webosbrowser {

BookmarkPanel::BookmarkPanel(BookmarkService* bookmarkService, QWidget *parent)
    : QWidget(parent),
      m_bookmarkService(bookmarkService),
      m_currentFolderId("") {
    setupUI();
    setFocusPolicy(Qt::StrongFocus);
    hide(); // 기본적으로 숨김 상태
}

BookmarkPanel::~BookmarkPanel() {
}

void BookmarkPanel::setupUI() {
    // 메인 레이아웃
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(20, 20, 20, 20);
    m_mainLayout->setSpacing(12);

    // 헤더 레이아웃
    m_headerLayout = new QHBoxLayout();
    m_titleLabel = new QLabel("북마크", this);
    m_titleLabel->setStyleSheet("font-size: 28px; font-weight: bold; color: #FFFFFF;");

    m_closeButton = new QPushButton("닫기", this);
    m_closeButton->setFixedSize(100, 40);
    connect(m_closeButton, &QPushButton::clicked, this, [this]() {
        hide();
        emit panelClosed();
    });

    m_headerLayout->addWidget(m_titleLabel);
    m_headerLayout->addStretch();
    m_headerLayout->addWidget(m_closeButton);

    // 검색 입력
    m_searchInput = new QLineEdit(this);
    m_searchInput->setPlaceholderText("북마크 검색...");
    m_searchInput->setStyleSheet("font-size: 18px; padding: 8px; background-color: rgba(255, 255, 255, 0.1); color: #FFFFFF; border: 1px solid #555;");
    connect(m_searchInput, &QLineEdit::textChanged, this, &BookmarkPanel::onSearchTextChanged);

    // 액션 버튼 레이아웃
    m_actionLayout = new QHBoxLayout();

    m_addButton = new QPushButton("추가", this);
    m_addButton->setFixedSize(100, 40);
    connect(m_addButton, &QPushButton::clicked, this, &BookmarkPanel::onAddBookmarkClicked);

    m_editButton = new QPushButton("편집", this);
    m_editButton->setFixedSize(100, 40);
    connect(m_editButton, &QPushButton::clicked, this, &BookmarkPanel::onEditBookmarkClicked);

    m_deleteButton = new QPushButton("삭제", this);
    m_deleteButton->setFixedSize(100, 40);
    connect(m_deleteButton, &QPushButton::clicked, this, &BookmarkPanel::onDeleteBookmarkClicked);

    m_newFolderButton = new QPushButton("새 폴더", this);
    m_newFolderButton->setFixedSize(100, 40);
    connect(m_newFolderButton, &QPushButton::clicked, this, &BookmarkPanel::onNewFolderClicked);

    m_actionLayout->addWidget(m_addButton);
    m_actionLayout->addWidget(m_editButton);
    m_actionLayout->addWidget(m_deleteButton);
    m_actionLayout->addWidget(m_newFolderButton);
    m_actionLayout->addStretch();

    // 북마크 리스트
    m_bookmarkList = new QListWidget(this);
    m_bookmarkList->setStyleSheet(
        "QListWidget {"
        "    background-color: rgba(255, 255, 255, 0.05);"
        "    color: #FFFFFF;"
        "    font-size: 20px;"
        "    border: 1px solid #555;"
        "    border-radius: 8px;"
        "}"
        "QListWidget::item {"
        "    padding: 16px;"
        "    border-bottom: 1px solid rgba(255, 255, 255, 0.1);"
        "}"
        "QListWidget::item:selected {"
        "    background-color: rgba(255, 255, 255, 0.2);"
        "    border: 3px solid #0078D7;"
        "}"
        "QListWidget::item:focus {"
        "    outline: 3px solid #0078D7;"
        "}"
    );
    connect(m_bookmarkList, &QListWidget::itemDoubleClicked, this, &BookmarkPanel::onBookmarkItemDoubleClicked);

    // 레이아웃 조립
    m_mainLayout->addLayout(m_headerLayout);
    m_mainLayout->addWidget(m_searchInput);
    m_mainLayout->addLayout(m_actionLayout);
    m_mainLayout->addWidget(m_bookmarkList);

    setLayout(m_mainLayout);

    // 패널 스타일
    setStyleSheet(
        "BookmarkPanel {"
        "    background-color: rgba(30, 30, 30, 0.95);"
        "    border-left: 2px solid #555;"
        "}"
    );
}

void BookmarkPanel::show() {
    QWidget::show();
    setFocus();
    refreshBookmarks();
    Logger::info("[BookmarkPanel] 패널 표시");
}

void BookmarkPanel::hide() {
    QWidget::hide();
    Logger::info("[BookmarkPanel] 패널 숨김");
}

void BookmarkPanel::setCurrentPage(const QString& url, const QString& title) {
    m_currentUrl = url;
    m_currentTitle = title;
    Logger::info(QString("[BookmarkPanel] 현재 페이지 설정: url=%1, title=%2").arg(url, title));
}

void BookmarkPanel::keyPressEvent(QKeyEvent* event) {
    switch (event->key()) {
    case Qt::Key_Escape:  // 백 버튼
    case Qt::Key_Backspace:
        hide();
        emit panelClosed();
        break;
    case Qt::Key_Return:  // 선택 버튼
    case Qt::Key_Enter:
        if (m_bookmarkList->currentItem()) {
            onBookmarkItemDoubleClicked(m_bookmarkList->currentItem());
        }
        break;
    default:
        QWidget::keyPressEvent(event);
        break;
    }
}

void BookmarkPanel::onAddBookmarkClicked() {
    Logger::info("[BookmarkPanel] 북마크 추가 버튼 클릭");
    showBookmarkDialog(false);
}

void BookmarkPanel::onEditBookmarkClicked() {
    Bookmark bookmark = getSelectedBookmark();
    if (bookmark.id.isEmpty()) {
        showToast("편집할 북마크를 선택하세요");
        return;
    }
    Logger::info(QString("[BookmarkPanel] 북마크 편집 버튼 클릭: id=%1").arg(bookmark.id));
    showBookmarkDialog(true, bookmark);
}

void BookmarkPanel::onDeleteBookmarkClicked() {
    Bookmark bookmark = getSelectedBookmark();
    if (bookmark.id.isEmpty()) {
        showToast("삭제할 북마크를 선택하세요");
        return;
    }

    Logger::info(QString("[BookmarkPanel] 북마크 삭제 버튼 클릭: id=%1").arg(bookmark.id));

    showConfirmDialog("북마크를 삭제하시겠습니까?", [this, bookmark]() {
        m_bookmarkService->deleteBookmark(bookmark.id, [this](bool success) {
            if (success) {
                showToast("북마크가 삭제되었습니다");
                refreshBookmarks();
            } else {
                showToast("북마크 삭제 실패");
            }
        });
    });
}

void BookmarkPanel::onNewFolderClicked() {
    Logger::info("[BookmarkPanel] 새 폴더 버튼 클릭");
    showFolderDialog();
}

void BookmarkPanel::onBookmarkItemDoubleClicked(QListWidgetItem* item) {
    if (!item) return;

    QString bookmarkId = item->data(Qt::UserRole).toString();
    if (bookmarkId.isEmpty()) return;

    m_bookmarkService->getBookmarkById(bookmarkId, [this](bool success, const Bookmark& bookmark) {
        if (success) {
            Logger::info(QString("[BookmarkPanel] 북마크 선택: url=%1").arg(bookmark.url));

            // 방문 횟수 증가
            m_bookmarkService->incrementVisitCount(bookmark.id, [](bool) {});

            emit bookmarkSelected(bookmark.url, bookmark.title);
            hide();
        } else {
            showToast("북마크를 찾을 수 없습니다");
        }
    });
}

void BookmarkPanel::refreshBookmarks() {
    Logger::info("[BookmarkPanel] 북마크 목록 갱신");

    m_bookmarkList->clear();

    m_bookmarkService->getBookmarksByFolder(m_currentFolderId, [this](bool success, const QVector<Bookmark>& bookmarks) {
        if (!success) {
            Logger::error("[BookmarkPanel] 북마크 목록 조회 실패");
            return;
        }

        if (bookmarks.isEmpty()) {
            QListWidgetItem* item = new QListWidgetItem("저장된 북마크가 없습니다");
            item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
            item->setForeground(QColor(150, 150, 150));
            m_bookmarkList->addItem(item);
            return;
        }

        for (const Bookmark& bookmark : bookmarks) {
            QString displayText = QString("%1\n%2").arg(bookmark.title, bookmark.url);
            QListWidgetItem* item = new QListWidgetItem(displayText);
            item->setData(Qt::UserRole, bookmark.id);
            m_bookmarkList->addItem(item);
        }

        Logger::info(QString("[BookmarkPanel] 북마크 목록 갱신 완료: %1개").arg(bookmarks.size()));
    });
}

void BookmarkPanel::onSearchTextChanged(const QString& text) {
    if (text.isEmpty()) {
        refreshBookmarks();
        return;
    }

    Logger::info(QString("[BookmarkPanel] 검색: query=%1").arg(text));

    m_bookmarkList->clear();

    m_bookmarkService->searchBookmarks(text, [this](bool success, const QVector<Bookmark>& bookmarks) {
        if (!success) {
            Logger::error("[BookmarkPanel] 검색 실패");
            return;
        }

        if (bookmarks.isEmpty()) {
            QListWidgetItem* item = new QListWidgetItem("검색 결과가 없습니다");
            item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
            item->setForeground(QColor(150, 150, 150));
            m_bookmarkList->addItem(item);
            return;
        }

        for (const Bookmark& bookmark : bookmarks) {
            QString displayText = QString("%1\n%2").arg(bookmark.title, bookmark.url);
            QListWidgetItem* item = new QListWidgetItem(displayText);
            item->setData(Qt::UserRole, bookmark.id);
            m_bookmarkList->addItem(item);
        }

        Logger::info(QString("[BookmarkPanel] 검색 결과: %1개").arg(bookmarks.size()));
    });
}

void BookmarkPanel::showBookmarkDialog(bool editMode, const Bookmark& bookmark) {
    m_bookmarkService->getAllFolders([this, editMode, bookmark](bool success, const QVector<BookmarkFolder>& folders) {
        if (!success) {
            Logger::error("[BookmarkPanel] 폴더 목록 조회 실패");
            return;
        }

        // 편집 모드가 아니고 현재 페이지 정보가 있으면 자동 입력
        Bookmark newBookmark = bookmark;
        if (!editMode && !m_currentUrl.isEmpty()) {
            newBookmark.url = m_currentUrl;
            newBookmark.title = m_currentTitle.isEmpty() ? m_currentUrl : m_currentTitle;
            newBookmark.folderId = m_currentFolderId;
        }

        BookmarkDialog* dialog = new BookmarkDialog(editMode, newBookmark, folders, this);

        if (dialog->exec() == QDialog::Accepted) {
            Bookmark result = dialog->getBookmark();

            if (editMode) {
                // 편집
                m_bookmarkService->updateBookmark(bookmark.id, result, [this](bool success) {
                    if (success) {
                        showToast("북마크가 수정되었습니다");
                        refreshBookmarks();
                    } else {
                        showToast("북마크 수정 실패");
                    }
                });
            } else {
                // 추가
                m_bookmarkService->addBookmark(result, [this](bool success, const Bookmark&, const QString& errorMsg) {
                    if (success) {
                        showToast("북마크가 저장되었습니다");
                        refreshBookmarks();
                    } else {
                        showToast(errorMsg.isEmpty() ? "북마크 추가 실패" : errorMsg);
                    }
                });
            }
        }

        delete dialog;
    });
}

void BookmarkPanel::showFolderDialog() {
    FolderDialog* dialog = new FolderDialog(false, BookmarkFolder(), this);

    if (dialog->exec() == QDialog::Accepted) {
        QString folderName = dialog->getFolderName();

        BookmarkFolder folder;
        folder.name = folderName;
        folder.parentId = m_currentFolderId;

        m_bookmarkService->addFolder(folder, [this](bool success, const BookmarkFolder&) {
            if (success) {
                showToast("폴더가 생성되었습니다");
                refreshBookmarks();
            } else {
                showToast("폴더 생성 실패");
            }
        });
    }

    delete dialog;
}

void BookmarkPanel::showConfirmDialog(const QString& message, std::function<void()> callback) {
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "확인", message,
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply == QMessageBox::Yes) {
        callback();
    }
}

void BookmarkPanel::showToast(const QString& message) {
    // 간단한 토스트 메시지 (QLabel 사용)
    QLabel* toast = new QLabel(message, this);
    toast->setStyleSheet(
        "QLabel {"
        "    background-color: rgba(0, 0, 0, 0.8);"
        "    color: #FFFFFF;"
        "    font-size: 18px;"
        "    padding: 12px 24px;"
        "    border-radius: 8px;"
        "}"
    );
    toast->setAlignment(Qt::AlignCenter);
    toast->setFixedSize(400, 60);
    toast->move((width() - toast->width()) / 2, height() - 100);
    toast->show();

    // 3초 후 자동 삭제
    QTimer::singleShot(3000, toast, &QLabel::deleteLater);

    Logger::info(QString("[BookmarkPanel] 토스트: %1").arg(message));
}

Bookmark BookmarkPanel::getSelectedBookmark() const {
    QListWidgetItem* item = m_bookmarkList->currentItem();
    if (!item) return Bookmark();

    QString bookmarkId = item->data(Qt::UserRole).toString();
    if (bookmarkId.isEmpty()) return Bookmark();

    // 동기 조회 (캐시에서)
    Bookmark result;
    m_bookmarkService->getBookmarkById(bookmarkId, [&result](bool success, const Bookmark& bookmark) {
        if (success) {
            result = bookmark;
        }
    });

    return result;
}

// BookmarkDialog 구현

BookmarkDialog::BookmarkDialog(bool editMode, const Bookmark& bookmark, const QVector<BookmarkFolder>& folders, QWidget* parent)
    : QDialog(parent),
      m_editMode(editMode),
      m_bookmark(bookmark) {
    setupUI(folders);
}

void BookmarkDialog::setupUI(const QVector<BookmarkFolder>& folders) {
    setWindowTitle(m_editMode ? "북마크 편집" : "북마크 추가");
    setModal(true);
    setMinimumWidth(500);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    QFormLayout* formLayout = new QFormLayout();

    // 제목 입력
    m_titleInput = new QLineEdit(m_bookmark.title, this);
    m_titleInput->setPlaceholderText("북마크 제목");
    m_titleInput->setStyleSheet("font-size: 18px; padding: 8px;");
    formLayout->addRow("제목:", m_titleInput);

    // URL 입력
    m_urlInput = new QLineEdit(m_bookmark.url, this);
    m_urlInput->setPlaceholderText("URL");
    m_urlInput->setStyleSheet("font-size: 18px; padding: 8px;");
    m_urlInput->setReadOnly(m_editMode);  // 편집 시 URL 변경 불가
    formLayout->addRow("URL:", m_urlInput);

    // 폴더 선택
    m_folderCombo = new QComboBox(this);
    m_folderCombo->setStyleSheet("font-size: 18px; padding: 8px;");
    m_folderCombo->addItem("루트 폴더", "");
    for (const BookmarkFolder& folder : folders) {
        m_folderCombo->addItem(folder.name, folder.id);
    }
    // 현재 폴더 선택
    int folderIndex = m_folderCombo->findData(m_bookmark.folderId);
    if (folderIndex >= 0) {
        m_folderCombo->setCurrentIndex(folderIndex);
    }
    formLayout->addRow("폴더:", m_folderCombo);

    // 설명 입력 (선택적)
    m_descriptionInput = new QLineEdit(m_bookmark.description, this);
    m_descriptionInput->setPlaceholderText("설명 (선택사항)");
    m_descriptionInput->setStyleSheet("font-size: 18px; padding: 8px;");
    formLayout->addRow("설명:", m_descriptionInput);

    mainLayout->addLayout(formLayout);

    // 버튼
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    m_saveButton = new QPushButton("저장", this);
    m_saveButton->setFixedSize(100, 40);
    connect(m_saveButton, &QPushButton::clicked, this, &QDialog::accept);

    m_cancelButton = new QPushButton("취소", this);
    m_cancelButton->setFixedSize(100, 40);
    connect(m_cancelButton, &QPushButton::clicked, this, &QDialog::reject);

    buttonLayout->addStretch();
    buttonLayout->addWidget(m_saveButton);
    buttonLayout->addWidget(m_cancelButton);

    mainLayout->addLayout(buttonLayout);
    setLayout(mainLayout);
}

Bookmark BookmarkDialog::getBookmark() const {
    Bookmark result = m_bookmark;
    result.title = m_titleInput->text();
    result.url = m_urlInput->text();
    result.folderId = m_folderCombo->currentData().toString();
    result.description = m_descriptionInput->text();
    return result;
}

// FolderDialog 구현

FolderDialog::FolderDialog(bool editMode, const BookmarkFolder& folder, QWidget* parent)
    : QDialog(parent),
      m_editMode(editMode),
      m_folder(folder) {
    setupUI();
}

void FolderDialog::setupUI() {
    setWindowTitle(m_editMode ? "폴더 편집" : "새 폴더");
    setModal(true);
    setMinimumWidth(400);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    QFormLayout* formLayout = new QFormLayout();

    // 폴더 이름 입력
    m_nameInput = new QLineEdit(m_folder.name, this);
    m_nameInput->setPlaceholderText("폴더 이름");
    m_nameInput->setStyleSheet("font-size: 18px; padding: 8px;");
    formLayout->addRow("이름:", m_nameInput);

    mainLayout->addLayout(formLayout);

    // 버튼
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    m_saveButton = new QPushButton("저장", this);
    m_saveButton->setFixedSize(100, 40);
    connect(m_saveButton, &QPushButton::clicked, this, &QDialog::accept);

    m_cancelButton = new QPushButton("취소", this);
    m_cancelButton->setFixedSize(100, 40);
    connect(m_cancelButton, &QPushButton::clicked, this, &QDialog::reject);

    buttonLayout->addStretch();
    buttonLayout->addWidget(m_saveButton);
    buttonLayout->addWidget(m_cancelButton);

    mainLayout->addLayout(buttonLayout);
    setLayout(mainLayout);
}

QString FolderDialog::getFolderName() const {
    return m_nameInput->text();
}

} // namespace webosbrowser
