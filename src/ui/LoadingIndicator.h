/**
 * @file LoadingIndicator.h
 * @brief 로딩 인디케이터 - 웹 페이지 로딩 진행률 표시
 */

#pragma once

#include <QWidget>
#include <QProgressBar>

namespace webosbrowser {

/**
 * @class LoadingIndicator
 * @brief 웹 페이지 로딩 진행률을 표시하는 QProgressBar 위젯
 *
 * WebView의 loadProgress 시그널과 연동하여 실시간 진행률 표시.
 * 간소화 버전: QProgressBar만 구현 (스피너 오버레이는 향후 추가).
 */
class LoadingIndicator : public QWidget {
    Q_OBJECT

public:
    /**
     * @brief 생성자
     * @param parent 부모 위젯
     */
    explicit LoadingIndicator(QWidget *parent = nullptr);

    /**
     * @brief 소멸자
     */
    ~LoadingIndicator() override;

    // 복사 생성자 및 대입 연산자 삭제
    LoadingIndicator(const LoadingIndicator&) = delete;
    LoadingIndicator& operator=(const LoadingIndicator&) = delete;

public slots:
    /**
     * @brief 진행률 업데이트
     * @param progress 진행률 (0~100)
     */
    void setProgress(int progress);

    /**
     * @brief 로딩 시작 (프로그레스바 표시)
     */
    void startLoading();

    /**
     * @brief 로딩 완료 (프로그레스바 숨김)
     * @param success 성공 여부
     */
    void finishLoading(bool success);

private:
    /**
     * @brief UI 초기화
     */
    void setupUI();

    /**
     * @brief 스타일시트 적용 (QSS)
     */
    void applyStyles();

private:
    QProgressBar *progressBar_;  ///< 진행률 표시 바
};

} // namespace webosbrowser
