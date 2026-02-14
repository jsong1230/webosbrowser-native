/**
 * @file SecurityIndicator.h
 * @brief 보안 상태 아이콘 표시 위젯
 */

#pragma once

#include <QLabel>
#include <QPixmap>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QFocusEvent>
#include "../utils/SecurityClassifier.h"

namespace webosbrowser {

/**
 * @class SecurityIndicator
 * @brief 보안 상태 아이콘 표시 위젯
 *
 * QLabel 기반으로 SecurityLevel에 따라 아이콘과 툴팁을 표시합니다.
 * 리모컨 포커스 가능 (Qt::StrongFocus).
 */
class SecurityIndicator : public QLabel {
    Q_OBJECT

public:
    explicit SecurityIndicator(QWidget *parent = nullptr);
    ~SecurityIndicator() override;

    SecurityIndicator(const SecurityIndicator&) = delete;
    SecurityIndicator& operator=(const SecurityIndicator&) = delete;

    /**
     * @brief 보안 등급 설정 (아이콘 및 툴팁 갱신)
     * @param level SecurityLevel
     */
    void setSecurityLevel(SecurityLevel level);

    /**
     * @brief 현재 보안 등급 반환
     * @return SecurityLevel
     */
    SecurityLevel securityLevel() const;

signals:
    /**
     * @brief 아이콘 클릭 시그널 (향후 FR-5 상세 정보 다이얼로그용)
     */
    void clicked();

protected:
    /**
     * @brief 마우스 클릭 이벤트 (향후 FR-5)
     */
    void mousePressEvent(QMouseEvent *event) override;

    /**
     * @brief 리모컨 Enter 키 이벤트 (향후 FR-5)
     */
    void keyPressEvent(QKeyEvent *event) override;

    /**
     * @brief 포커스 인 이벤트 (툴팁 표시)
     */
    void focusInEvent(QFocusEvent *event) override;

private:
    /**
     * @brief UI 초기화
     */
    void setupUI();

    /**
     * @brief 스타일 적용 (QSS)
     */
    void applyStyles();

    /**
     * @brief 아이콘 캐시 로드 (성능 최적화)
     */
    void loadIconCache();

private:
    SecurityLevel currentLevel_;     ///< 현재 보안 등급
    QPixmap secureIcon_;             ///< Secure 아이콘 (캐시)
    QPixmap insecureIcon_;           ///< Insecure 아이콘 (캐시)
    QPixmap localIcon_;              ///< Local 아이콘 (캐시)
};

} // namespace webosbrowser
