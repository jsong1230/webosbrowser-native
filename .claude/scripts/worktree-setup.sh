#!/bin/bash
set -e

FEATURE_NAME="${1:?기능명을 입력하세요 (예: coupon)}"
shift
MEMBERS=("$@")

if [ ${#MEMBERS[@]} -eq 0 ]; then
  echo "❌ 팀원을 하나 이상 지정하세요 (예: backend frontend)"
  exit 1
fi

echo "🚀 Worktree 세팅: feature/$FEATURE_NAME"
mkdir -p .worktrees

if ! grep -q "^\.worktrees/" .gitignore 2>/dev/null; then
  echo ".worktrees/" >> .gitignore
fi

CURRENT_BRANCH=$(git branch --show-current)

for MEMBER in "${MEMBERS[@]}"; do
  BRANCH_NAME="feature/${FEATURE_NAME}-${MEMBER}"
  WORKTREE_PATH=".worktrees/${FEATURE_NAME}-${MEMBER}"

  if [ -d "$WORKTREE_PATH" ]; then
    echo "⏭️  ${MEMBER}: 이미 존재"
    continue
  fi

  git branch "$BRANCH_NAME" "$CURRENT_BRANCH" 2>/dev/null || true
  git worktree add "$WORKTREE_PATH" "$BRANCH_NAME"

  # .env 파일이 있으면 worktree로 복사 (gitignore되어 공유 안 되므로)
  for ENV_FILE in .env .env.local; do
    [ -f "$ENV_FILE" ] && cp "$ENV_FILE" "$WORKTREE_PATH/$ENV_FILE"
  done

  echo "✅ ${MEMBER}: $WORKTREE_PATH → $BRANCH_NAME"
done

echo ""
git worktree list
