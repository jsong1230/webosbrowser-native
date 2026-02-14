# 보안 아이콘 리소스

## 필요한 아이콘 파일

F-14 HTTPS 보안 표시 기능을 위해 다음 아이콘 파일이 필요합니다:

### 1. lock_secure.png
- **크기**: 32x32px
- **설명**: HTTPS 보안 연결 표시 (녹색 자물쇠)
- **사용처**: SecurityIndicator (SecurityLevel::Secure)

### 2. lock_insecure.png
- **크기**: 32x32px
- **설명**: HTTP 비보안 연결 경고 (노란색 경고 아이콘)
- **사용처**: SecurityIndicator (SecurityLevel::Insecure)

### 3. info.png
- **크기**: 32x32px
- **설명**: localhost/file 로컬 연결 정보 (파란색 정보 아이콘)
- **사용처**: SecurityIndicator (SecurityLevel::Local)

## 아이콘 제작 가이드

### 디자인 요구사항
- 크기: 32x32px (PNG 형식)
- 배경: 투명
- 색상 대비: WCAG AA 기준 (4.5:1 이상)
- 스타일: 플랫 디자인, 대화면 프로젝터 가독성 고려

### 색상 권장사항
- **Secure (녹색)**: #00aa00, #4caf50
- **Insecure (노란색)**: #ffaa00, #ff9800
- **Local (파란색)**: #0099ff, #2196f3

## Qt 리소스 파일 (resources.qrc)

아이콘 파일 생성 후 `resources.qrc`에 등록 필요:

```xml
<!DOCTYPE RCC>
<RCC version="1.0">
    <qresource>
        <file>icons/lock_secure.png</file>
        <file>icons/lock_insecure.png</file>
        <file>icons/info.png</file>
    </qresource>
</RCC>
```

## 임시 대안

실제 아이콘 파일이 없을 경우:
1. SecurityIndicator는 qWarning 로그 출력
2. 빈 QLabel 표시 (툴팁만 제공)
3. 기능은 정상 동작 (아이콘 없이)

## TODO

- [ ] lock_secure.png 생성
- [ ] lock_insecure.png 생성
- [ ] info.png 생성
- [ ] resources.qrc 파일 생성 및 등록
- [ ] CMakeLists.txt에 Qt 리소스 빌드 설정 추가
