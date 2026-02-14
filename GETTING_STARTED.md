# Getting Started - webOS Browser Native

이 문서는 새 Claude 세션에서 프로젝트를 시작할 때 따라야 할 단계를 설명합니다.

## 1. 개발 환경 확인

### Qt 설치 확인
```bash
qmake --version
# 출력 예시: QMake version 3.1, Using Qt version 5.15.2
```

Qt가 없다면:
```bash
brew install qt@5
echo 'export PATH="/usr/local/opt/qt@5/bin:$PATH"' >> ~/.zshrc
source ~/.zshrc
```

### CMake 설치 확인
```bash
cmake --version
# 출력 예시: cmake version 3.27.0
```

CMake가 없다면:
```bash
brew install cmake
```

### webOS Native SDK 확인
webOS Native SDK는 LG 공식 사이트에서 다운로드 필요:
https://webostv.developer.lge.com/develop/tools/sdk-download

## 2. 프로젝트 초기화

### Git 초기화 및 첫 커밋
```bash
cd /Users/jsong/dev/jsong1230-github/webosbrowser-native
git init
git add .
git commit -m "chore: 프로젝트 초기 설정"
```

### GitHub 리포지토리 연결
```bash
# GitHub에서 webosbrowser-native 리포지토리 생성 후
git remote add origin https://github.com/jsong1230/webosbrowser-native.git
git branch -M main
git push -u origin main
```

## 3. Claude 실행 명령어

새 Claude 세션에서 다음 명령어로 시작:

```
/init-project webOS Native Browser - C++/Qt 기반 실제 웹 브라우저 구현. Web App PoC(webosbrowser)의 설계 문서를 기반으로 webOS WebView API를 사용하여 외부 웹사이트 렌더링 기능 구현. 타겟 플랫폼: LG HU715QW 프로젝터(webOS 6.x).
```

또는 다음 내용을 복사해서 Claude에게 전달:

---

## Claude에게 전달할 내용

```
안녕하세요! webOS Browser Native 프로젝트를 시작합니다.

프로젝트 개요:
- C++/Qt 기반 webOS Native App
- webOS WebView API로 실제 웹 브라우저 구현
- 전신 프로젝트: webosbrowser (Web App PoC, iframe 제약으로 마무리)
- 설계 문서: docs/specs/ (15개 기능 설계서 있음)
- 타겟: LG HU715QW 프로젝터 (webOS 6.x)

다음 단계:
1. Qt 프로젝트 구조 생성
2. CMakeLists.txt 작성
3. webOS 메타데이터 설정
4. 기본 WebView 통합
5. 15개 기능 순차 구현

시작해주세요!
```

---

## 4. 예상 프로젝트 구조

```
webosbrowser-native/
├── CMakeLists.txt
├── src/
│   ├── main.cpp
│   ├── browser/
│   │   ├── BrowserWindow.cpp
│   │   ├── BrowserWindow.h
│   │   ├── WebView.cpp
│   │   └── WebView.h
│   ├── ui/
│   │   ├── URLBar.cpp
│   │   ├── URLBar.h
│   │   ├── NavigationBar.cpp
│   │   └── NavigationBar.h
│   └── services/
│       ├── BookmarkService.cpp
│       ├── BookmarkService.h
│       ├── HistoryService.cpp
│       └── HistoryService.h
├── include/
├── resources/
│   └── icons/
├── webos-meta/
│   ├── appinfo.json
│   └── icon.png
└── docs/
    ├── project/
    └── specs/
```

## 5. 참고 문서

- Web App PoC: https://github.com/jsong1230/webosbrowser
- 설계 문서: `docs/specs/` (이미 복사됨)
- webOS Native API: https://webostv.developer.lge.com
- Qt Documentation: https://doc.qt.io/qt-5/

---

**준비 완료!** 이제 새 Claude 세션에서 위 명령어를 실행하세요.
