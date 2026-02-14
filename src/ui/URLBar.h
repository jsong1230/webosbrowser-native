/**
 * @file URLBar.h
 * @brief URL 입력 바 위젯 (리모컨 최적화)
 */

#pragma once

#include <QWidget>
#include <QLineEdit>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QString>
#include <QUrl>
#include <QKeyEvent>
#include <QFocusEvent>

namespace webosbrowser {

// Forward declaration
class URLValidator;

/**
 * @class URLBar
 * @brief URL 입력 바 위젯 (리모컨 최적화)
 *
 * QLineEdit 기반 URL 입력 필드와 에러 표시 기능 포함.
 * 향후 가상 키보드 및 자동완성 기능이 추가될 예정.
 */
class URLBar : public QWidget {
    Q_OBJECT

public:
    explicit URLBar(QWidget *parent = nullptr);
    ~URLBar() override;

    URLBar(const URLBar&) = delete;
    URLBar& operator=(const URLBar&) = delete;

    /**
     * @brief 현재 입력된 URL 반환
     */
    QString text() const;

    /**
     * @brief URL 텍스트 설정 (프로그래밍 방식)
     */
    void setText(const QString &url);

    /**
     * @brief 입력 필드에 포커스 설정
     */
    void setFocusToInput();

    /**
     * @brief 에러 메시지 표시
     */
    void showError(const QString &message);

    /**
     * @brief 에러 메시지 숨김
     */
    void hideError();

signals:
    /**
     * @brief URL 입력 완료 (Enter 키 입력 시)
     * @param url 입력된 URL (자동 보완 후)
     */
    void urlSubmitted(const QUrl &url);

    /**
     * @brief 입력 취소 (ESC 키 입력 시)
     */
    void editingCancelled();

protected:
    /**
     * @brief 키 이벤트 처리 (리모컨 방향키, Enter, ESC 등)
     */
    void keyPressEvent(QKeyEvent *event) override;

    /**
     * @brief 포커스 인 이벤트 (테두리 하이라이트)
     */
    void focusInEvent(QFocusEvent *event) override;

    /**
     * @brief 포커스 아웃 이벤트
     */
    void focusOutEvent(QFocusEvent *event) override;

private slots:
    /**
     * @brief Return/Enter 키 입력 시 URL 제출
     */
    void onReturnPressed();

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
     * @brief 스타일 적용 (QSS)
     */
    void applyStyles();

    /**
     * @brief URL 검증 및 자동 보완
     * @return 검증된 QUrl, 실패 시 빈 QUrl
     */
    QUrl validateAndCompleteUrl(const QString &input);

private:
    // UI 컴포넌트
    QLineEdit *inputField_;              ///< URL 입력 필드
    QLabel *errorLabel_;                 ///< 에러 메시지 라벨

    // 레이아웃
    QVBoxLayout *mainLayout_;            ///< 메인 레이아웃
    QHBoxLayout *inputLayout_;           ///< 입력 필드 레이아웃

    // 상태 관리
    QString previousUrl_;                ///< 입력 전 URL (취소 시 복원)
};

} // namespace webosbrowser
