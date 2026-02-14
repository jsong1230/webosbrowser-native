/**
 * @file HistoryPanel.cpp
 * @brief 히스토리 패널 UI 구현
 */

#include "HistoryPanel.h"
#include "services/HistoryService.h"
#include "utils/Logger.h"
#include "utils/DateFormatter.h"
#include <QHBoxLayout>
#include <QMenu>
#include <QMessageBox>
#include <QKeyEvent>
#include <QScrollBar>

namespace webosbrowser {

HistoryPanel::HistoryPanel(HistoryService *historyService, QWidget *parent)
    : QWidget(parent)
    , historyService_(historyService)
    , isVisible_(false)
{
    if (!historyService_) {
        Logger::error("HistoryPanel: HistoryService가 null입니다.");
    }

    setupUI();
    setupConnections();

    // 초기에는 숨김 상태
    hide();
}

HistoryPanel::~HistoryPanel() {
}

void HistoryPanel::setupUI() {
    // 메인 레이아웃
    mainLayout_ = new QVBoxLayout(this);
    mainLayout_->setContentsMargins(20, 20, 20, 20);
    mainLayout_->setSpacing(15);

    // 헤더 영역 (타이틀 + 닫기 버튼)
    QHBoxLayout *headerLayout = new QHBoxLayout();

    titleLabel_ = new QLabel("히스토리", this);
    QFont titleFont = titleLabel_->font();
    titleFont.setPointSize(24);
    titleFont.setBold(true);
    titleLabel_->setFont(titleFont);

    closeButton_ = new QPushButton("닫기", this);
    closeButton_->setFixedSize(100, 40);

    headerLayout->addWidget(titleLabel_);
    headerLayout->addStretch();
    headerLayout->addWidget(closeButton_);

    mainLayout_->addLayout(headerLayout);

    // 검색 입력 필드
    searchEdit_ = new QLineEdit(this);
    searchEdit_->setPlaceholderText("제목 또는 URL 검색...");
    searchEdit_->setFixedHeight(40);
    QFont searchFont = searchEdit_->font();
    searchFont.setPointSize(16);
    searchEdit_->setFont(searchFont);

    mainLayout_->addWidget(searchEdit_);

    // 액션 버튼 (모두 삭제)
    QHBoxLayout *actionLayout = new QHBoxLayout();

    deleteAllButton_ = new QPushButton("모두 삭제", this);
    deleteAllButton_->setFixedSize(120, 40);

    actionLayout->addStretch();
    actionLayout->addWidget(deleteAllButton_);

    mainLayout_->addLayout(actionLayout);

    // 히스토리 리스트 위젯
    historyListWidget_ = new QListWidget(this);
    historyListWidget_->setContextMenuPolicy(Qt::CustomContextMenu);
    historyListWidget_->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    historyListWidget_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // 폰트 크기 설정 (대화면 최적화)
    QFont listFont = historyListWidget_->font();
    listFont.setPointSize(16);
    historyListWidget_->setFont(listFont);

    mainLayout_->addWidget(historyListWidget_);

    // 빈 히스토리 메시지 라벨
    emptyMessageLabel_ = new QLabel("방문 기록이 없습니다.", this);
    emptyMessageLabel_->setAlignment(Qt::AlignCenter);
    QFont emptyFont = emptyMessageLabel_->font();
    emptyFont.setPointSize(18);
    emptyMessageLabel_->setFont(emptyFont);
    emptyMessageLabel_->hide();

    mainLayout_->addWidget(emptyMessageLabel_);

    // 패널 크기 및 스타일 설정
    setFixedWidth(PANEL_WIDTH);

    // 스타일시트 적용
    setStyleSheet(R"(
        QWidget {
            background-color: #1e1e1e;
            color: #ffffff;
        }
        QLineEdit {
            background-color: #2a2a2a;
            border: 2px solid #3a3a3a;
            border-radius: 8px;
            padding: 8px;
            color: #ffffff;
        }
        QLineEdit:focus {
            border-color: #4a9eff;
        }
        QPushButton {
            background-color: #3a3a3a;
            border: none;
            border-radius: 8px;
            padding: 10px;
            color: #ffffff;
        }
        QPushButton:hover {
            background-color: #4a4a4a;
        }
        QPushButton:pressed {
            background-color: #2a2a2a;
        }
        QListWidget {
            background-color: #1e1e1e;
            border: none;
            outline: none;
        }
        QListWidget::item {
            background-color: #2a2a2a;
            border-radius: 8px;
            margin: 4px 0;
            padding: 10px;
        }
        QListWidget::item:hover {
            background-color: #3a3a3a;
        }
        QListWidget::item:selected {
            background-color: #4a9eff;
            border: 3px solid #6ab9ff;
        }
    )");
}

void HistoryPanel::setupConnections() {
    if (!historyService_) {
        return;
    }

    // 버튼 클릭
    connect(closeButton_, &QPushButton::clicked, this, &HistoryPanel::onCloseButtonClicked);
    connect(deleteAllButton_, &QPushButton::clicked, this, &HistoryPanel::onDeleteAllHistory);

    // 검색 입력
    connect(searchEdit_, &QLineEdit::textChanged, this, &HistoryPanel::onSearchTextChanged);

    // 히스토리 리스트 이벤트
    connect(historyListWidget_, &QListWidget::itemDoubleClicked,
            this, &HistoryPanel::onHistoryItemDoubleClicked);
    connect(historyListWidget_, &QListWidget::customContextMenuRequested,
            this, &HistoryPanel::onHistoryItemContextMenu);

    // 히스토리 서비스 시그널
    connect(historyService_, &HistoryService::historyAdded,
            this, &HistoryPanel::refreshHistoryList);
    connect(historyService_, &HistoryService::historyDeleted,
            this, &HistoryPanel::refreshHistoryList);
    connect(historyService_, &HistoryService::allHistoryDeleted,
            this, &HistoryPanel::refreshHistoryList);
}

void HistoryPanel::showPanel() {
    isVisible_ = true;
    show();
    refreshHistoryList();

    // 검색 필드에 포커스
    searchEdit_->setFocus();
    searchEdit_->clear();

    Logger::info("HistoryPanel 표시");
}

void HistoryPanel::hidePanel() {
    isVisible_ = false;
    hide();
    emit closeRequested();
    Logger::info("HistoryPanel 숨김");
}

void HistoryPanel::togglePanel() {
    if (isVisible_) {
        hidePanel();
    } else {
        showPanel();
    }
}

void HistoryPanel::refreshHistoryList() {
    if (!historyService_) {
        return;
    }

    historyListWidget_->clear();

    QList<HistoryEntry> historyList;

    // 검색어가 있으면 검색 결과, 없으면 전체 목록
    if (!currentSearchQuery_.isEmpty()) {
        historyList = historyService_->searchHistory(currentSearchQuery_);
    } else {
        // 날짜별 그룹화된 히스토리 표시
        renderGroupedHistory();
        return;
    }

    // 검색 결과 표시
    if (historyList.isEmpty()) {
        showEmptyMessage();
        return;
    }

    for (const HistoryEntry &entry : historyList) {
        QListWidgetItem *item = createHistoryItem(entry);
        historyListWidget_->addItem(item);
    }

    emptyMessageLabel_->hide();
    historyListWidget_->show();
}

void HistoryPanel::renderGroupedHistory() {
    if (!historyService_) {
        return;
    }

    QMap<HistoryDateGroup, QList<HistoryEntry>> grouped = historyService_->getGroupedHistory();

    if (grouped.isEmpty() || historyService_->getHistoryCount() == 0) {
        showEmptyMessage();
        return;
    }

    // 날짜 그룹 순서
    QList<HistoryDateGroup> groupOrder = {
        HistoryDateGroup::Today,
        HistoryDateGroup::Yesterday,
        HistoryDateGroup::Last7Days,
        HistoryDateGroup::ThisMonth,
        HistoryDateGroup::Older
    };

    for (HistoryDateGroup group : groupOrder) {
        if (grouped.contains(group) && !grouped[group].isEmpty()) {
            // 날짜 그룹 헤더 추가
            QString groupName = DateFormatter::getDateGroupName(static_cast<int>(group));
            QListWidgetItem *headerItem = createDateGroupHeader(groupName);
            historyListWidget_->addItem(headerItem);

            // 히스토리 항목 추가
            for (const HistoryEntry &entry : grouped[group]) {
                QListWidgetItem *item = createHistoryItem(entry);
                historyListWidget_->addItem(item);
            }
        }
    }

    emptyMessageLabel_->hide();
    historyListWidget_->show();
}

void HistoryPanel::showEmptyMessage() {
    historyListWidget_->hide();
    emptyMessageLabel_->show();
}

QListWidgetItem* HistoryPanel::createHistoryItem(const HistoryEntry &entry) {
    QListWidgetItem *item = new QListWidgetItem();

    // 항목 텍스트: 제목 + URL + 방문 시각
    QString timeStr = DateFormatter::toTimeString(entry.visitedAt);
    QString itemText = QString("%1\n%2  |  %3")
                       .arg(entry.title, entry.url, timeStr);

    item->setText(itemText);
    item->setData(Qt::UserRole, entry.id); // ID를 UserRole에 저장
    item->setSizeHint(QSize(PANEL_WIDTH - 40, ITEM_HEIGHT));

    return item;
}

QListWidgetItem* HistoryPanel::createDateGroupHeader(const QString &groupName) {
    QListWidgetItem *item = new QListWidgetItem();

    item->setText(QString("📅 %1").arg(groupName));
    item->setData(Qt::UserRole, "header"); // 헤더 표시
    item->setSizeHint(QSize(PANEL_WIDTH - 40, HEADER_HEIGHT));
    item->setFlags(Qt::ItemIsEnabled); // 선택 불가

    // 헤더 스타일
    QFont font = item->font();
    font.setPointSize(20);
    font.setBold(true);
    item->setFont(font);

    return item;
}

QString HistoryPanel::getSelectedHistoryId() const {
    QListWidgetItem *currentItem = historyListWidget_->currentItem();
    if (!currentItem) {
        return QString();
    }

    QString data = currentItem->data(Qt::UserRole).toString();
    if (data == "header") {
        return QString(); // 헤더는 선택 불가
    }

    return data;
}

void HistoryPanel::onSearchTextChanged(const QString &query) {
    currentSearchQuery_ = query;
    refreshHistoryList();
}

void HistoryPanel::onHistoryItemDoubleClicked(QListWidgetItem *item) {
    QString historyId = item->data(Qt::UserRole).toString();
    if (historyId.isEmpty() || historyId == "header") {
        return;
    }

    if (!historyService_) {
        return;
    }

    HistoryEntry entry = historyService_->getHistoryById(historyId);
    if (entry.id.isEmpty()) {
        Logger::warning("HistoryPanel: 히스토리를 찾을 수 없습니다.");
        return;
    }

    emit historySelected(entry.url, entry.title);
    hidePanel();

    Logger::info(QString("히스토리 선택: %1").arg(entry.url));
}

void HistoryPanel::onHistoryItemContextMenu(const QPoint &pos) {
    QListWidgetItem *item = historyListWidget_->itemAt(pos);
    if (!item) {
        return;
    }

    QString historyId = item->data(Qt::UserRole).toString();
    if (historyId.isEmpty() || historyId == "header") {
        return;
    }

    // 컨텍스트 메뉴 생성
    QMenu menu(this);
    QAction *deleteAction = menu.addAction("삭제");
    QAction *openAction = menu.addAction("페이지 열기");

    QAction *selected = menu.exec(historyListWidget_->mapToGlobal(pos));

    if (selected == deleteAction) {
        onDeleteHistoryItem();
    } else if (selected == openAction) {
        onHistoryItemDoubleClicked(item);
    }
}

void HistoryPanel::onDeleteHistoryItem() {
    QString historyId = getSelectedHistoryId();
    if (historyId.isEmpty()) {
        return;
    }

    if (!historyService_) {
        return;
    }

    // 삭제 확인 다이얼로그
    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "히스토리 삭제",
        "이 히스토리를 삭제하시겠습니까?",
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply == QMessageBox::Yes) {
        bool success = historyService_->deleteHistory(historyId);
        if (success) {
            emit historyDeleted(1);
            Logger::info(QString("히스토리 삭제: ID=%1").arg(historyId));
        }
    }
}

void HistoryPanel::onDeleteAllHistory() {
    if (!historyService_) {
        return;
    }

    // 전체 삭제 확인 다이얼로그
    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "전체 히스토리 삭제",
        "모든 방문 기록을 삭제하시겠습니까?",
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply == QMessageBox::Yes) {
        int count = historyService_->getHistoryCount();
        bool success = historyService_->deleteAllHistory();
        if (success) {
            emit historyDeleted(count);
            Logger::info(QString("전체 히스토리 삭제: %1개").arg(count));
        }
    }
}

void HistoryPanel::onCloseButtonClicked() {
    hidePanel();
}

void HistoryPanel::keyPressEvent(QKeyEvent *event) {
    // 백 버튼 (Escape 또는 Backspace)으로 패널 닫기
    if (event->key() == Qt::Key_Escape || event->key() == Qt::Key_Backspace) {
        hidePanel();
        event->accept();
        return;
    }

    QWidget::keyPressEvent(event);
}

bool HistoryPanel::isVisible() const {
    return isVisible_;
}

} // namespace webosbrowser
