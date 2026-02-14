/**
 * @file HomePage.cpp
 * @brief 즐겨찾기 홈 화면 구현
 */

#include "HomePage.h"
#include "BookmarkCard.h"
#include "../services/BookmarkService.h"
#include "../utils/Logger.h"
#include "../utils/KeyCodeConstants.h"
#include <QDebug>
#include <QFrame>

namespace webosbrowser {

HomePage::HomePage(BookmarkService *bookmarkService, QWidget *parent)
    : QWidget(parent)
    , bookmarkService_(bookmarkService)
    , mainLayout_(nullptr)
    , titleLabel_(nullptr)
    , contentScrollArea_(nullptr)
    , contentContainer_(nullptr)
    , gridLayout_(nullptr)
    , emptyStateWidget_(nullptr)
    , footerLabel_(nullptr)
    , currentFocusIndex_(0)
{
    qDebug() << "HomePage: 생성 중...";

    if (!bookmarkService_) {
        Logger::error("HomePage: BookmarkService가 nullptr입니다!");
        setupUI();
        return;
    }

    setupUI();
    setupConnections();
    applyStyles();

    qDebug() << "HomePage: 생성 완료";
}

HomePage::~HomePage() {
    qDebug() << "HomePage: 소멸";
}

void HomePage::setupUI() {
    // 메인 레이아웃
    mainLayout_ = new QVBoxLayout(this);
    mainLayout_->setContentsMargins(40, 40, 40, 40);
    mainLayout_->setSpacing(20);

    // 타이틀 라벨
    titleLabel_ = new QLabel("즐겨찾기", this);
    titleLabel_->setObjectName("titleLabel");
    titleLabel_->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    mainLayout_->addWidget(titleLabel_);

    // 스크롤 영역 (M4 대비)
    contentScrollArea_ = new QScrollArea(this);
    contentScrollArea_->setWidgetResizable(true);
    contentScrollArea_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    contentScrollArea_->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    contentScrollArea_->setFrameShape(QFrame::NoFrame);

    // 스크롤 컨테이너
    contentContainer_ = new QWidget();
    contentScrollArea_->setWidget(contentContainer_);

    // 그리드 레이아웃 (4열)
    gridLayout_ = new QGridLayout(contentContainer_);
    gridLayout_->setContentsMargins(0, 0, 0, 0);
    gridLayout_->setSpacing(16);  // 카드 간격 16px
    gridLayout_->setColumnStretch(0, 1);
    gridLayout_->setColumnStretch(1, 1);
    gridLayout_->setColumnStretch(2, 1);
    gridLayout_->setColumnStretch(3, 1);

    // 빈 상태 위젯 생성
    emptyStateWidget_ = createEmptyStateWidget();
    gridLayout_->addWidget(emptyStateWidget_, 0, 0, 1, 4);  // 4열 병합
    emptyStateWidget_->hide();  // 초기 숨김

    mainLayout_->addWidget(contentScrollArea_, 1);  // stretch=1

    // 푸터 라벨
    footerLabel_ = new QLabel("Red: 북마크 추가  |  Menu: 설정  |  Back: 뒤로가기", this);
    footerLabel_->setObjectName("footerLabel");
    footerLabel_->setAlignment(Qt::AlignCenter);
    mainLayout_->addWidget(footerLabel_);

    // 포커스 정책
    setFocusPolicy(Qt::StrongFocus);
}

QWidget* HomePage::createEmptyStateWidget() {
    QWidget *widget = new QWidget(contentContainer_);
    widget->setObjectName("emptyState");

    QVBoxLayout *layout = new QVBoxLayout(widget);
    layout->setAlignment(Qt::AlignCenter);
    layout->setSpacing(30);

    // 메시지
    QLabel *messageLabel = new QLabel(
        "북마크를 추가하여 자주 가는 사이트에\n빠르게 접근하세요",
        widget
    );
    messageLabel->setObjectName("emptyMessage");
    messageLabel->setAlignment(Qt::AlignCenter);
    messageLabel->setStyleSheet("font-size: 24px; color: #888888;");
    layout->addWidget(messageLabel);

    // 북마크 추가 버튼
    QPushButton *addButton = new QPushButton("북마크 추가 (Red 버튼)", widget);
    addButton->setObjectName("actionButton");
    addButton->setFocusPolicy(Qt::StrongFocus);
    addButton->setMinimumSize(300, 60);
    addButton->setStyleSheet(R"(
        QPushButton {
            font-size: 18px;
            background-color: #0078D7;
            color: white;
            border: none;
            border-radius: 5px;
        }
        QPushButton:focus {
            border: 3px solid #00AAFF;
        }
    )");
    connect(addButton, &QPushButton::clicked, this, [this]() {
        emit bookmarkAddRequested();
    });
    layout->addWidget(addButton);

    // "지금 둘러보기" 버튼
    QPushButton *browseButton = new QPushButton("지금 둘러보기", widget);
    browseButton->setObjectName("actionButton");
    browseButton->setFocusPolicy(Qt::StrongFocus);
    browseButton->setMinimumSize(300, 60);
    browseButton->setStyleSheet(R"(
        QPushButton {
            font-size: 18px;
            background-color: #5A5A5A;
            color: white;
            border: none;
            border-radius: 5px;
        }
        QPushButton:focus {
            border: 3px solid #00AAFF;
        }
    )");
    connect(browseButton, &QPushButton::clicked, this, [this]() {
        emit backRequested();
    });
    layout->addWidget(browseButton);

    return widget;
}

void HomePage::setupConnections() {
    if (!bookmarkService_) {
        return;
    }

    // BookmarkService 시그널 연결
    connect(bookmarkService_, &BookmarkService::bookmarkAdded,
            this, &HomePage::onBookmarkAdded);
    connect(bookmarkService_, &BookmarkService::bookmarkDeleted,
            this, &HomePage::onBookmarkDeleted);
    connect(bookmarkService_, &BookmarkService::bookmarkUpdated,
            this, &HomePage::onBookmarkUpdated);
}

void HomePage::applyStyles() {
    QString styleSheet = R"(
        HomePage {
            background-color: #1E1E1E;
        }

        QLabel#titleLabel {
            color: #FFFFFF;
            font-size: 32px;
            font-weight: bold;
            padding: 20px;
        }

        QLabel#footerLabel {
            color: #888888;
            font-size: 16px;
            padding: 10px;
            background-color: #2A2A2A;
        }

        QScrollArea {
            border: none;
            background-color: transparent;
        }
    )";

    setStyleSheet(styleSheet);
}

void HomePage::refreshBookmarks() {
    qDebug() << "HomePage: 북마크 새로고침";

    if (!bookmarkService_) {
        Logger::error("HomePage: BookmarkService가 nullptr입니다");
        displayEmptyState();
        return;
    }

    // BookmarkService에서 비동기 조회
    bookmarkService_->getAllBookmarks([this](bool success, const QVector<Bookmark> &bookmarks) {
        onBookmarksLoaded(success, bookmarks);
    });
}

void HomePage::onBookmarksLoaded(bool success, const QVector<Bookmark> &bookmarks) {
    if (!success) {
        Logger::warning("HomePage: 북마크 로드 실패");
        displayEmptyState();
        return;
    }

    qDebug() << "HomePage: 북마크 로드 성공 - 개수:" << bookmarks.size();

    // 데이터 캐시 업데이트
    bookmarks_ = bookmarks;

    // 최대 12개만 표시
    if (bookmarks_.size() > MAX_BOOKMARKS) {
        bookmarks_ = bookmarks_.mid(0, MAX_BOOKMARKS);
    }

    // 그리드 재구성
    if (bookmarks_.isEmpty()) {
        displayEmptyState();
    } else {
        displayBookmarks(bookmarks_);
    }
}

void HomePage::rebuildGrid() {
    qDebug() << "HomePage: 그리드 재구성";

    // 기존 카드 삭제
    for (BookmarkCard *card : cards_) {
        gridLayout_->removeWidget(card);
        card->deleteLater();
    }
    cards_.clear();

    // 빈 상태 위젯 숨김
    if (emptyStateWidget_) {
        emptyStateWidget_->hide();
    }

    // 북마크 없으면 빈 상태 표시
    if (bookmarks_.isEmpty()) {
        displayEmptyState();
        return;
    }

    // 새 카드 생성
    for (int i = 0; i < bookmarks_.size(); ++i) {
        const Bookmark &bookmark = bookmarks_[i];

        BookmarkCard *card = new BookmarkCard(
            bookmark.id, bookmark.title, bookmark.url, contentContainer_
        );

        // 시그널 연결
        connect(card, &BookmarkCard::clicked,
                this, &HomePage::onCardClicked);

        // 그리드에 추가 (4열)
        int row = i / GRID_COLUMNS;
        int col = i % GRID_COLUMNS;
        gridLayout_->addWidget(card, row, col);

        cards_.append(card);
    }

    // 첫 번째 카드에 포커스
    if (!cards_.isEmpty()) {
        currentFocusIndex_ = 0;
        cards_[0]->setFocus();
    }
}

void HomePage::displayBookmarks(const QVector<Bookmark> &bookmarks) {
    bookmarks_ = bookmarks;
    rebuildGrid();
}

void HomePage::displayEmptyState() {
    qDebug() << "HomePage: 빈 상태 표시";

    // 기존 카드 삭제
    for (BookmarkCard *card : cards_) {
        gridLayout_->removeWidget(card);
        card->deleteLater();
    }
    cards_.clear();

    // 빈 상태 위젯 표시
    if (emptyStateWidget_) {
        emptyStateWidget_->show();

        // 첫 번째 버튼에 포커스
        QPushButton *firstButton = emptyStateWidget_->findChild<QPushButton*>("actionButton");
        if (firstButton) {
            firstButton->setFocus();
        }
    }
}

void HomePage::onCardClicked(const QString &url) {
    qDebug() << "HomePage: 카드 클릭 - URL:" << url;
    emit bookmarkSelected(url);
}

void HomePage::onBookmarkAdded(const Bookmark &bookmark) {
    Q_UNUSED(bookmark);
    qDebug() << "HomePage: 북마크 추가됨 - 새로고침";
    refreshBookmarks();
}

void HomePage::onBookmarkDeleted(const QString &id) {
    Q_UNUSED(id);
    qDebug() << "HomePage: 북마크 삭제됨 - 새로고침";
    refreshBookmarks();
}

void HomePage::onBookmarkUpdated(const QString &id) {
    Q_UNUSED(id);
    qDebug() << "HomePage: 북마크 업데이트됨 - 새로고침";
    refreshBookmarks();
}

void HomePage::showEvent(QShowEvent *event) {
    QWidget::showEvent(event);
    qDebug() << "HomePage: 표시됨 - 북마크 새로고침";
    refreshBookmarks();
}

// Phase 4: 리모컨 네비게이션
void HomePage::keyPressEvent(QKeyEvent *event) {
    int keyCode = event->key();
    qDebug() << "HomePage: 키 입력 - keyCode:" << keyCode;

    // 방향키 처리
    if (keyCode == Qt::Key_Left) {
        moveFocusLeft();
        event->accept();
        return;
    } else if (keyCode == Qt::Key_Right) {
        moveFocusRight();
        event->accept();
        return;
    } else if (keyCode == Qt::Key_Up) {
        moveFocusUp();
        event->accept();
        return;
    } else if (keyCode == Qt::Key_Down) {
        moveFocusDown();
        event->accept();
        return;
    }

    // Enter 키 (북마크 선택)
    if (keyCode == Qt::Key_Return || keyCode == Qt::Key_Enter) {
        activateCurrentCard();
        event->accept();
        return;
    }

    // Back 키 (이전 페이지)
    if (keyCode == Qt::Key_Escape || keyCode == Qt::Key_Back) {
        qDebug() << "HomePage: Back 키 입력";
        emit backRequested();
        event->accept();
        return;
    }

    // Red 버튼 (북마크 추가)
    if (keyCode == KeyCode::RED) {
        qDebug() << "HomePage: Red 버튼 입력";
        emit bookmarkAddRequested();
        event->accept();
        return;
    }

    // Menu 버튼 (설정)
    if (keyCode == Qt::Key_Menu || keyCode == KeyCode::MENU) {
        qDebug() << "HomePage: Menu 버튼 입력";
        emit settingsRequested();
        event->accept();
        return;
    }

    // 그 외 키는 부모로 전파
    QWidget::keyPressEvent(event);
}

void HomePage::moveFocusLeft() {
    if (cards_.isEmpty()) return;

    int newIndex = currentFocusIndex_ - 1;
    if (newIndex < 0) {
        newIndex = 0;  // 최소값 제한
    }

    if (newIndex != currentFocusIndex_) {
        currentFocusIndex_ = newIndex;
        cards_[currentFocusIndex_]->setFocus();
        qDebug() << "HomePage: 포커스 이동 (왼쪽) - 인덱스:" << currentFocusIndex_;
    }
}

void HomePage::moveFocusRight() {
    if (cards_.isEmpty()) return;

    int newIndex = currentFocusIndex_ + 1;
    if (newIndex >= cards_.size()) {
        newIndex = cards_.size() - 1;  // 최대값 제한
    }

    if (newIndex != currentFocusIndex_) {
        currentFocusIndex_ = newIndex;
        cards_[currentFocusIndex_]->setFocus();
        qDebug() << "HomePage: 포커스 이동 (오른쪽) - 인덱스:" << currentFocusIndex_;
    }
}

void HomePage::moveFocusUp() {
    if (cards_.isEmpty()) return;

    int newIndex = currentFocusIndex_ - GRID_COLUMNS;
    if (newIndex < 0) {
        newIndex = 0;  // 최소값 제한
    }

    if (newIndex != currentFocusIndex_) {
        currentFocusIndex_ = newIndex;
        cards_[currentFocusIndex_]->setFocus();
        qDebug() << "HomePage: 포커스 이동 (위) - 인덱스:" << currentFocusIndex_;
    }
}

void HomePage::moveFocusDown() {
    if (cards_.isEmpty()) return;

    int newIndex = currentFocusIndex_ + GRID_COLUMNS;
    if (newIndex >= cards_.size()) {
        newIndex = cards_.size() - 1;  // 최대값 제한
    }

    if (newIndex != currentFocusIndex_) {
        currentFocusIndex_ = newIndex;
        cards_[currentFocusIndex_]->setFocus();
        qDebug() << "HomePage: 포커스 이동 (아래) - 인덱스:" << currentFocusIndex_;
    }
}

void HomePage::activateCurrentCard() {
    if (cards_.isEmpty() || currentFocusIndex_ < 0 || currentFocusIndex_ >= cards_.size()) {
        return;
    }

    // 현재 포커스된 카드의 URL 전달
    QString url = cards_[currentFocusIndex_]->url();
    qDebug() << "HomePage: 북마크 활성화 - URL:" << url;
    emit bookmarkSelected(url);
}

} // namespace webosbrowser
