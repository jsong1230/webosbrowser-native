/**
 * @file BookmarkCard.h
 * @brief 홈 화면의 북마크 카드 UI
 */

#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QString>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QFocusEvent>

namespace webosbrowser {

/**
 * @class BookmarkCard
 * @brief 홈 화면의 개별 북마크 카드
 *
 * 파비콘(기본 아이콘), 제목을 표시하며
 * 클릭/Enter 키로 페이지를 엽니다.
 */
class BookmarkCard : public QWidget {
    Q_OBJECT

public:
    /**
     * @brief 생성자
     * @param id 북마크 ID
     * @param title 북마크 제목
     * @param url 북마크 URL
     * @param parent 부모 위젯
     */
    explicit BookmarkCard(const QString &id, const QString &title,
                          const QString &url, QWidget *parent = nullptr);
    ~BookmarkCard() override;

    BookmarkCard(const BookmarkCard&) = delete;
    BookmarkCard& operator=(const BookmarkCard&) = delete;

    /**
     * @brief 북마크 ID 조회
     */
    QString id() const { return id_; }

    /**
     * @brief 북마크 URL 조회
     */
    QString url() const { return url_; }

signals:
    /**
     * @brief 카드 클릭/Enter 키 시그널
     * @param url 북마크 URL
     */
    void clicked(const QString &url);

protected:
    /**
     * @brief 마우스 클릭 이벤트
     * @param event 마우스 이벤트
     */
    void mousePressEvent(QMouseEvent *event) override;

    /**
     * @brief 키 입력 이벤트 (Enter 키)
     * @param event 키 이벤트
     */
    void keyPressEvent(QKeyEvent *event) override;

    /**
     * @brief 포커스 인 이벤트
     * @param event 포커스 이벤트
     */
    void focusInEvent(QFocusEvent *event) override;

    /**
     * @brief 포커스 아웃 이벤트
     * @param event 포커스 이벤트
     */
    void focusOutEvent(QFocusEvent *event) override;

private:
    /**
     * @brief UI 초기화
     */
    void setupUI();

    /**
     * @brief 스타일 적용
     * @param focused 포커스 여부
     */
    void updateStyle(bool focused);

    /**
     * @brief 제목 텍스트를 말줄임표 처리
     * @param text 원본 텍스트
     * @param maxLength 최대 길이
     * @return 말줄임표 처리된 텍스트
     */
    QString elideText(const QString &text, int maxLength);

    // 데이터
    QString id_;
    QString title_;
    QString url_;

    // UI 컴포넌트
    QVBoxLayout *layout_;
    QLabel *faviconLabel_;  // 파비콘 (기본 아이콘)
    QLabel *titleLabel_;    // 제목

    // 상태
    bool isFocused_;
};

} // namespace webosbrowser
