---
description: >
  features.md에서 다음으로 개발할 기능을 선택하고 파이프라인을 실행합니다.
---

## Step 1: 백로그 확인
- docs/project/features.md를 읽고 현재 상태 파악
- "⏳ 대기" 상태이면서 의존성이 모두 "✅ 완료"인 기능을 찾기
- 우선순위(Must > Should > Could)와 ID 순서 기반으로 다음 기능 선택

## Step 2: 사용자 확인
- 선택된 기능을 사용자에게 제시
- 실행 모드 제안: 독립적 백엔드+프론트엔드 작업이면 /fullstack-feature-team, 그 외 /fullstack-feature
- 사용자 승인 대기

## Step 3: 상태 업데이트
- features.md에서 해당 기능 상태를 "🔄 진행중"으로 변경

## Step 4: 파이프라인 실행
- 사용자가 선택한 모드로 기능 파이프라인 실행
- /fullstack-feature {기능명} 또는 /fullstack-feature-team {기능명}

## Step 5: 완료 후 상태 업데이트
- features.md에서 해당 기능 상태를 "✅ 완료"로 변경
- 다음 기능이 있으면 안내

추가 지시사항: $ARGUMENTS
