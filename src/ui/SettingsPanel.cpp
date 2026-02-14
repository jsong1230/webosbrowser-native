/**
 * @file SettingsPanel.cpp
 * @brief 설정 패널 UI 구현
 */

#include "SettingsPanel.h"
#include "../services/SettingsService.h"
#include "../services/BookmarkService.h"
#include "../services/HistoryService.h"
#include "../services/SearchEngine.h"
#include "../utils/URLValidator.h"
#include <QMessageBox>
#include <QEasingCurve>
#include <QDebug>

namespace webosbrowser {

SettingsPanel::SettingsPanel(SettingsService *settingsService,
                             BookmarkService *bookmarkService,
                             HistoryService *historyService,
                             QWidget *parent)
    : QWidget(parent)
    , settingsService_(settingsService)
    , bookmarkService_(bookmarkService)
    , historyService_(historyService)
    , mainLayout_(nullptr)
    , contentWidget_(nullptr)
    , contentLayout_(nullptr)
    , formLayout_(nullptr)
    , headerLayout_(nullptr)
    , titleLabel_(nullptr)
    , closeButton_(nullptr)
    , searchEngineLabel_(nullptr)
    , searchEngineLayout_(nullptr)
    , googleRadio_(nullptr)
    , naverRadio_(nullptr)
    , bingRadio_(nullptr)
    , duckduckgoRadio_(nullptr)
    , searchEngineGroup_(nullptr)
    , homepageLabel_(nullptr)
    , homepageInput_(nullptr)
    , homepageErrorLabel_(nullptr)
    , themeLabel_(nullptr)
    , themeLayout_(nullptr)
    , darkThemeRadio_(nullptr)
    , lightThemeRadio_(nullptr)
    , themeGroup_(nullptr)
    , clearDataLabel_(nullptr)
    , clearDataLayout_(nullptr)
    , clearBookmarksButton_(nullptr)
    , clearHistoryButton_(nullptr)
    , slideAnimation_(nullptr)
{
    setupUI();
    setupConnections();
    setupFocusOrder();

    // 초기 상태: 숨김
    hide();
}

SettingsPanel::~SettingsPanel() = default;

void SettingsPanel::setupUI() {
    // 반투명 배경 (오버레이)
    setStyleSheet("QWidget#SettingsPanel { background-color: rgba(0, 0, 0, 0.7); }");
    setObjectName("SettingsPanel");

    // 메인 레이아웃
    mainLayout_ = new QVBoxLayout(this);
    mainLayout_->setContentsMargins(0, 0, 0, 0);

    // 컨텐츠 위젯 (실제 설정 패널)
    contentWidget_ = new QWidget(this);
    contentWidget_->setObjectName("SettingsContentWidget");
    contentWidget_->setStyleSheet(R"(
        QWidget#SettingsContentWidget {
            background-color: #2D2D2D;
            border: 1px solid #444444;
            border-radius: 8px;
        }
    )");
    contentWidget_->setFixedWidth(500);

    contentLayout_ = new QVBoxLayout(contentWidget_);
    contentLayout_->setContentsMargins(20, 20, 20, 20);
    contentLayout_->setSpacing(15);

    // 헤더 (제목 + 닫기 버튼)
    headerLayout_ = new QHBoxLayout();
    titleLabel_ = new QLabel("설정 (Settings)", contentWidget_);
    titleLabel_->setStyleSheet("QLabel { color: #FFFFFF; font-size: 18px; font-weight: bold; }");
    closeButton_ = new QPushButton("X", contentWidget_);
    closeButton_->setFixedSize(30, 30);
    closeButton_->setStyleSheet(R"(
        QPushButton {
            background-color: #CC0000;
            color: #FFFFFF;
            border: none;
            border-radius: 4px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #FF0000;
        }
        QPushButton:focus {
            border: 2px solid #0078D7;
        }
    )");
    headerLayout_->addWidget(titleLabel_);
    headerLayout_->addStretch();
    headerLayout_->addWidget(closeButton_);
    contentLayout_->addLayout(headerLayout_);

    // 구분선
    QWidget *separator1 = new QWidget(contentWidget_);
    separator1->setFixedHeight(1);
    separator1->setStyleSheet("background-color: #444444;");
    contentLayout_->addWidget(separator1);

    // 폼 레이아웃
    formLayout_ = new QFormLayout();
    formLayout_->setSpacing(10);
    formLayout_->setLabelAlignment(Qt::AlignLeft);

    // 검색 엔진 섹션
    searchEngineLabel_ = new QLabel("검색 엔진:", contentWidget_);
    searchEngineLabel_->setStyleSheet("QLabel { color: #FFFFFF; font-size: 14px; }");

    searchEngineLayout_ = new QHBoxLayout();
    googleRadio_ = new QRadioButton("Google", contentWidget_);
    naverRadio_ = new QRadioButton("Naver", contentWidget_);
    bingRadio_ = new QRadioButton("Bing", contentWidget_);
    duckduckgoRadio_ = new QRadioButton("DuckDuckGo", contentWidget_);

    googleRadio_->setStyleSheet("QRadioButton { color: #FFFFFF; }");
    naverRadio_->setStyleSheet("QRadioButton { color: #FFFFFF; }");
    bingRadio_->setStyleSheet("QRadioButton { color: #FFFFFF; }");
    duckduckgoRadio_->setStyleSheet("QRadioButton { color: #FFFFFF; }");

    searchEngineGroup_ = new QButtonGroup(this);
    searchEngineGroup_->addButton(googleRadio_, 0);
    searchEngineGroup_->addButton(naverRadio_, 1);
    searchEngineGroup_->addButton(bingRadio_, 2);
    searchEngineGroup_->addButton(duckduckgoRadio_, 3);

    searchEngineLayout_->addWidget(googleRadio_);
    searchEngineLayout_->addWidget(naverRadio_);
    searchEngineLayout_->addWidget(bingRadio_);
    searchEngineLayout_->addWidget(duckduckgoRadio_);
    searchEngineLayout_->addStretch();

    formLayout_->addRow(searchEngineLabel_, searchEngineLayout_);

    // 홈페이지 섹션
    homepageLabel_ = new QLabel("홈페이지 URL:", contentWidget_);
    homepageLabel_->setStyleSheet("QLabel { color: #FFFFFF; font-size: 14px; }");

    QVBoxLayout *homepageVLayout = new QVBoxLayout();
    homepageInput_ = new QLineEdit(contentWidget_);
    homepageInput_->setPlaceholderText("예: https://www.google.com");
    homepageInput_->setStyleSheet(R"(
        QLineEdit {
            background-color: #1E1E1E;
            color: #FFFFFF;
            border: 1px solid #555555;
            border-radius: 4px;
            padding: 6px;
        }
        QLineEdit:focus {
            border: 2px solid #0078D7;
        }
    )");
    homepageErrorLabel_ = new QLabel(contentWidget_);
    homepageErrorLabel_->setStyleSheet("QLabel { color: #FF6666; font-size: 12px; }");
    homepageErrorLabel_->setVisible(false);
    homepageVLayout->addWidget(homepageInput_);
    homepageVLayout->addWidget(homepageErrorLabel_);

    formLayout_->addRow(homepageLabel_, homepageVLayout);

    // 테마 섹션
    themeLabel_ = new QLabel("테마:", contentWidget_);
    themeLabel_->setStyleSheet("QLabel { color: #FFFFFF; font-size: 14px; }");

    themeLayout_ = new QHBoxLayout();
    darkThemeRadio_ = new QRadioButton("다크 모드", contentWidget_);
    lightThemeRadio_ = new QRadioButton("라이트 모드", contentWidget_);

    darkThemeRadio_->setStyleSheet("QRadioButton { color: #FFFFFF; }");
    lightThemeRadio_->setStyleSheet("QRadioButton { color: #FFFFFF; }");

    themeGroup_ = new QButtonGroup(this);
    themeGroup_->addButton(darkThemeRadio_, 0);
    themeGroup_->addButton(lightThemeRadio_, 1);

    themeLayout_->addWidget(darkThemeRadio_);
    themeLayout_->addWidget(lightThemeRadio_);
    themeLayout_->addStretch();

    formLayout_->addRow(themeLabel_, themeLayout_);

    contentLayout_->addLayout(formLayout_);

    // 구분선
    QWidget *separator2 = new QWidget(contentWidget_);
    separator2->setFixedHeight(1);
    separator2->setStyleSheet("background-color: #444444;");
    contentLayout_->addWidget(separator2);

    // 브라우징 데이터 삭제 섹션
    clearDataLabel_ = new QLabel("브라우징 데이터 삭제:", contentWidget_);
    clearDataLabel_->setStyleSheet("QLabel { color: #FFFFFF; font-size: 14px; font-weight: bold; }");
    contentLayout_->addWidget(clearDataLabel_);

    clearDataLayout_ = new QHBoxLayout();
    clearBookmarksButton_ = new QPushButton("북마크 전체 삭제", contentWidget_);
    clearHistoryButton_ = new QPushButton("히스토리 전체 삭제", contentWidget_);

    clearBookmarksButton_->setStyleSheet(R"(
        QPushButton {
            background-color: #990000;
            color: #FFFFFF;
            border: none;
            border-radius: 4px;
            padding: 8px;
            font-size: 12px;
        }
        QPushButton:hover {
            background-color: #CC0000;
        }
        QPushButton:focus {
            border: 2px solid #0078D7;
        }
    )");

    clearHistoryButton_->setStyleSheet(clearBookmarksButton_->styleSheet());

    clearDataLayout_->addWidget(clearBookmarksButton_);
    clearDataLayout_->addWidget(clearHistoryButton_);
    clearDataLayout_->addStretch();

    contentLayout_->addLayout(clearDataLayout_);

    contentLayout_->addStretch();

    mainLayout_->addWidget(contentWidget_);

    // 애니메이션 설정
    slideAnimation_ = new QPropertyAnimation(contentWidget_, "geometry", this);
    slideAnimation_->setDuration(300);
    slideAnimation_->setEasingCurve(QEasingCurve::OutCubic);
}

void SettingsPanel::setupConnections() {
    if (settingsService_) {
        connect(settingsService_, &SettingsService::settingsLoaded,
                this, &SettingsPanel::onSettingsLoaded);
    }

    // 검색 엔진 라디오 버튼
    connect(googleRadio_, &QRadioButton::toggled,
            this, &SettingsPanel::onSearchEngineRadioToggled);
    connect(naverRadio_, &QRadioButton::toggled,
            this, &SettingsPanel::onSearchEngineRadioToggled);
    connect(bingRadio_, &QRadioButton::toggled,
            this, &SettingsPanel::onSearchEngineRadioToggled);
    connect(duckduckgoRadio_, &QRadioButton::toggled,
            this, &SettingsPanel::onSearchEngineRadioToggled);

    // 홈페이지 입력
    connect(homepageInput_, &QLineEdit::editingFinished,
            this, &SettingsPanel::onHomepageEditingFinished);

    // 테마 라디오 버튼
    connect(darkThemeRadio_, &QRadioButton::toggled,
            this, &SettingsPanel::onThemeRadioToggled);
    connect(lightThemeRadio_, &QRadioButton::toggled,
            this, &SettingsPanel::onThemeRadioToggled);

    // 브라우징 데이터 삭제 버튼
    connect(clearBookmarksButton_, &QPushButton::clicked,
            this, &SettingsPanel::onClearBookmarksClicked);
    connect(clearHistoryButton_, &QPushButton::clicked,
            this, &SettingsPanel::onClearHistoryClicked);

    // 닫기 버튼
    connect(closeButton_, &QPushButton::clicked,
            this, &SettingsPanel::onCloseButtonClicked);
}

void SettingsPanel::setupFocusOrder() {
    // 포커스 정책 설정
    googleRadio_->setFocusPolicy(Qt::StrongFocus);
    naverRadio_->setFocusPolicy(Qt::StrongFocus);
    bingRadio_->setFocusPolicy(Qt::StrongFocus);
    duckduckgoRadio_->setFocusPolicy(Qt::StrongFocus);
    homepageInput_->setFocusPolicy(Qt::StrongFocus);
    darkThemeRadio_->setFocusPolicy(Qt::StrongFocus);
    lightThemeRadio_->setFocusPolicy(Qt::StrongFocus);
    clearBookmarksButton_->setFocusPolicy(Qt::StrongFocus);
    clearHistoryButton_->setFocusPolicy(Qt::StrongFocus);
    closeButton_->setFocusPolicy(Qt::StrongFocus);

    // Tab Order 설정
    setTabOrder(googleRadio_, naverRadio_);
    setTabOrder(naverRadio_, bingRadio_);
    setTabOrder(bingRadio_, duckduckgoRadio_);
    setTabOrder(duckduckgoRadio_, homepageInput_);
    setTabOrder(homepageInput_, darkThemeRadio_);
    setTabOrder(darkThemeRadio_, lightThemeRadio_);
    setTabOrder(lightThemeRadio_, clearBookmarksButton_);
    setTabOrder(clearBookmarksButton_, clearHistoryButton_);
    setTabOrder(clearHistoryButton_, closeButton_);
}

void SettingsPanel::showPanel() {
    // 현재 설정 로드
    loadCurrentSettings();

    show();
    raise();

    // 패널이 부모 위젯 크기를 차지하도록 설정
    setGeometry(parentWidget()->rect());

    // 슬라이드 인 애니메이션 (오른쪽에서 왼쪽으로)
    int parentWidth = parentWidget()->width();
    int parentHeight = parentWidget()->height();
    int panelWidth = contentWidget_->width();
    int panelHeight = qMin(600, parentHeight - 160);

    contentWidget_->setFixedHeight(panelHeight);

    QRect startGeometry(parentWidth, 80, panelWidth, panelHeight);
    QRect endGeometry(parentWidth - panelWidth - 20, 80, panelWidth, panelHeight);

    slideAnimation_->setStartValue(startGeometry);
    slideAnimation_->setEndValue(endGeometry);
    slideAnimation_->start();

    // 첫 번째 항목에 포커스
    googleRadio_->setFocus();
}

void SettingsPanel::hidePanel() {
    QRect startGeometry = contentWidget_->geometry();
    int parentWidth = parentWidget()->width();
    QRect endGeometry(parentWidth, startGeometry.y(),
                      startGeometry.width(), startGeometry.height());

    slideAnimation_->setStartValue(startGeometry);
    slideAnimation_->setEndValue(endGeometry);
    slideAnimation_->start();

    connect(slideAnimation_, &QPropertyAnimation::finished, this, [this]() {
        hide();
        emit panelClosed();
    }, Qt::SingleShotConnection);
}

void SettingsPanel::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Escape || event->key() == Qt::Key_Back) {
        hidePanel();
        event->accept();
        return;
    }
    QWidget::keyPressEvent(event);
}

void SettingsPanel::onSearchEngineRadioToggled(bool checked) {
    if (!checked || !settingsService_) return;

    QString engineId;
    if (sender() == googleRadio_) {
        engineId = "google";
    } else if (sender() == naverRadio_) {
        engineId = "naver";
    } else if (sender() == bingRadio_) {
        engineId = "bing";
    } else if (sender() == duckduckgoRadio_) {
        engineId = "duckduckgo";
    }

    if (!engineId.isEmpty()) {
        bool success = settingsService_->setSearchEngine(engineId);
        if (success) {
            qDebug() << "[SettingsPanel] 검색 엔진 변경:" << engineId;
        }
    }
}

void SettingsPanel::onHomepageEditingFinished() {
    if (!settingsService_) return;

    QString url = homepageInput_->text().trimmed();

    // 빈 입력은 무시
    if (url.isEmpty()) {
        homepageErrorLabel_->setVisible(false);
        return;
    }

    // URL 검증
    if (!url.startsWith("about:") && !URLValidator::isValid(url)) {
        homepageErrorLabel_->setText("유효하지 않은 URL입니다.");
        homepageErrorLabel_->setVisible(true);
        return;
    }

    bool success = settingsService_->setHomepage(url);
    if (success) {
        homepageErrorLabel_->setVisible(false);
        showToast("홈페이지가 설정되었습니다.");
    } else {
        homepageErrorLabel_->setText("설정 저장에 실패했습니다.");
        homepageErrorLabel_->setVisible(true);
    }
}

void SettingsPanel::onThemeRadioToggled(bool checked) {
    if (!checked || !settingsService_) return;

    QString themeId;
    if (sender() == darkThemeRadio_) {
        themeId = "dark";
    } else if (sender() == lightThemeRadio_) {
        themeId = "light";
    }

    if (!themeId.isEmpty()) {
        bool success = settingsService_->setTheme(themeId);
        if (success) {
            qDebug() << "[SettingsPanel] 테마 변경:" << themeId;
        }
    }
}

void SettingsPanel::onClearBookmarksClicked() {
    if (!bookmarkService_) return;

    bool confirmed = showConfirmDialog(
        "모든 북마크를 삭제하시겠습니까?\n이 작업은 되돌릴 수 없습니다."
    );

    if (!confirmed) return;

    // 비동기로 모든 북마크 조회 후 삭제
    bookmarkService_->getAllBookmarks([this](bool success, const QVector<Bookmark> &bookmarks) {
        if (!success) {
            QMessageBox::critical(this, "오류", "북마크 조회에 실패했습니다.");
            return;
        }

        // 모든 북마크 삭제
        for (const auto &bookmark : bookmarks) {
            bookmarkService_->deleteBookmark(bookmark.id, [](bool) {});
        }

        showToast("북마크가 삭제되었습니다.");
    });
}

void SettingsPanel::onClearHistoryClicked() {
    if (!historyService_) return;

    bool confirmed = showConfirmDialog(
        "모든 히스토리를 삭제하시겠습니까?\n이 작업은 되돌릴 수 없습니다."
    );

    if (!confirmed) return;

    // 비동기로 모든 히스토리 조회 후 삭제
    historyService_->getAllHistory([this](bool success, const QVector<HistoryItem> &items) {
        if (!success) {
            QMessageBox::critical(this, "오류", "히스토리 조회에 실패했습니다.");
            return;
        }

        // 모든 히스토리 삭제
        for (const auto &item : items) {
            historyService_->deleteHistory(item.id, [](bool) {});
        }

        showToast("히스토리가 삭제되었습니다.");
    });
}

void SettingsPanel::onCloseButtonClicked() {
    hidePanel();
}

void SettingsPanel::onSettingsLoaded() {
    loadCurrentSettings();
}

void SettingsPanel::loadCurrentSettings() {
    if (!settingsService_) return;

    // 검색 엔진 반영
    QString searchEngine = settingsService_->searchEngine();
    if (searchEngine == "google") {
        googleRadio_->setChecked(true);
    } else if (searchEngine == "naver") {
        naverRadio_->setChecked(true);
    } else if (searchEngine == "bing") {
        bingRadio_->setChecked(true);
    } else if (searchEngine == "duckduckgo") {
        duckduckgoRadio_->setChecked(true);
    }

    // 홈페이지 반영
    QString homepage = settingsService_->homepage();
    homepageInput_->setText(homepage);
    homepageErrorLabel_->setVisible(false);

    // 테마 반영
    QString theme = settingsService_->theme();
    if (theme == "dark") {
        darkThemeRadio_->setChecked(true);
    } else if (theme == "light") {
        lightThemeRadio_->setChecked(true);
    }
}

bool SettingsPanel::showConfirmDialog(const QString &message) {
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("경고");
    msgBox.setText(message);
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);

    // 버튼 텍스트 한글화
    msgBox.button(QMessageBox::Yes)->setText("삭제");
    msgBox.button(QMessageBox::No)->setText("취소");

    int result = msgBox.exec();
    return (result == QMessageBox::Yes);
}

void SettingsPanel::showToast(const QString &message) {
    QMessageBox::information(this, "알림", message);
}

} // namespace webosbrowser
