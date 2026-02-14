#!/bin/bash
TASK_NAME="${1:-unknown}"
LOG_FILE="docs/dev-log.md"

[ ! -f "$LOG_FILE" ] && mkdir -p docs && echo "# 개발 진행 로그" > "$LOG_FILE"

echo "" >> "$LOG_FILE"
echo "#### [$(date '+%Y-%m-%d %H:%M')] Task: $TASK_NAME" >> "$LOG_FILE"
echo "- 변경 파일: $(git diff --name-only 2>/dev/null | head -10)" >> "$LOG_FILE"
