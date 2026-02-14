---
description: >
  새 프로젝트의 초기 기획을 수행합니다.
  PRD 작성 → 기능 분해 → 마일스톤 로드맵 생성.
---

## Step 1: PRD 작성 (project-planner 에이전트)
- 사용자와 대화하며 프로젝트 목적, 대상 사용자, 핵심 기능 파악
- docs/project/prd.md 작성

## Step 2: 기능 분해 (project-planner 에이전트)
- PRD 기반으로 기능 목록 도출
- 각 기능의 우선순위 (Must/Should/Could) 결정
- 기능 간 의존성 파악
- 각 기능의 충돌 영역 분석 (수정 대상 모듈/테이블/컴포넌트)
- 병렬 그룹(PG-*) 배정 (같은 마일스톤 + 의존성 없음 + 충돌 영역 미겹침)
- docs/project/features.md 작성

## Step 3: 마일스톤 로드맵 (project-planner 에이전트)
- 의존성과 우선순위 기반으로 마일스톤 구성
- 각 마일스톤의 목표와 완료 기준 정의
- docs/project/roadmap.md 작성

## Step 4: 사용자 검토
- PRD, 기능 백로그, 로드맵을 사용자에게 제시
- 피드백 반영 후 확정
- "다음 기능 개발은 /next-feature 또는 /fullstack-feature {기능명}으로 시작하세요" 안내

프로젝트 설명: $ARGUMENTS
