---
description: >
  기능 완료 후 문서 상태 점검 + 운영 문서 작성 + 커밋을 일괄 처리합니다.
---

## Step 1: 전체 문서 상태 점검
- ✅/❌ docs/specs/{기능명}/requirements.md (요구사항 분석서)
- ✅/❌ docs/specs/{기능명}/design.md (기술 설계서)
- ✅/❌ docs/specs/{기능명}/plan.md (구현 계획서)
- ✅/❌ docs/api/{기능명}.md (API 스펙 확정본)
- ✅/❌ docs/db/{기능명}.md (DB 스키마 확정본)
- ✅/❌/해당없음 docs/components/ (컴포넌트 문서)

누락 문서가 있으면 사용자에게 알림 (doc-writer가 직접 작성하지 않음)

## Step 2: 운영 문서 (doc-writer 에이전트)
- docs/dev-log.md 진행 기록 (위 문서 상태 체크리스트 포함)
- CHANGELOG.md 업데이트

## Step 3: Git 커밋
- 논리적 단위로 분리하여 커밋

## Step 4: 최종 보고
- 파일 목록, 커밋 히스토리, 문서 상태, 남은 작업

기능명: $ARGUMENTS
