#!/usr/bin/env bash
# Self-initialisation of the listening phase from the repository as it is NOW (plan step S-000/S-010).
#   plan/bootstrap/bootstrap.sh WORKDIR
# Idempotent: safe to run again after any interruption. Writes WORKDIR/bootstrap.json. Exit codes:
#   0 ready | 2 usage | 3 merge conflict (BLOCKED) | 4 vendored pair tool hash mismatch | 5 render differs from the committed render | 6 local and remote branch diverged (BLOCKED)
set -euo pipefail
WORK="${1:-}"; [ -n "$WORK" ] || { echo "usage: bootstrap.sh WORKDIR" >&2; exit 2; }
URL="${OILDRUM_URL:-https://github.com/qualitycoding/oildrumkit}"
AUDITED=35fbfead61a11a8c7f5609a95831a29f2cb2bf78            # last commit whose Source/ and Tools/ the rewrite was audited at
SHA_MAKE=3c4f75854e2a692d3a77460d470e289be405c2166bbc30abbd4eeb053d6824c7
SHA_SCORE=f2d9ea5c76a731fd3dd553d698a9f1a92b393d272f6ea898e6a9e5cbde49defa
mkdir -p "$WORK"; cd "$WORK"
if [ ! -d oildrumkit/.git ]; then git clone --quiet "$URL" oildrumkit; fi
cd oildrumkit
git fetch --quiet origin '+refs/heads/*:refs/remotes/origin/*'
MAIN=$(git rev-parse origin/main)
# Never reset a local branch: unpushed implementation commits would be lost.
if git show-ref --verify --quiet refs/heads/impl/listening-validation; then
  RESUMED=true; git checkout --quiet impl/listening-validation
  if git rev-parse --verify --quiet origin/impl/listening-validation >/dev/null; then
    if git merge-base --is-ancestor HEAD origin/impl/listening-validation; then git merge --ff-only --quiet origin/impl/listening-validation
    elif ! git merge-base --is-ancestor origin/impl/listening-validation HEAD; then echo "BLOCKED: local and remote impl/listening-validation have diverged" >&2; exit 6; fi
  fi
elif git rev-parse --verify --quiet origin/impl/listening-validation >/dev/null; then
  RESUMED=true; git checkout --quiet -b impl/listening-validation origin/impl/listening-validation
else
  RESUMED=false; git checkout --quiet -b impl/listening-validation origin/main
fi
if ! git merge-base --is-ancestor origin/main HEAD; then
  git merge --no-edit --quiet origin/main || { git merge --abort || true; echo "BLOCKED: merge of origin/main into impl/listening-validation conflicts" >&2; exit 3; }
fi
BASE=$(git rev-parse HEAD)
DRIFT=$(git diff --name-only "$AUDITED" HEAD -- Source Tools CMakeLists.txt | tr '\n' ' ')
for pair in "scripts/make_blind_pairs.py:$SHA_MAKE" "scripts/score_blind.py:$SHA_SCORE"; do
  f=${pair%%:*}; want=${pair##*:}
  have=$(git show "HEAD:$f" 2>/dev/null | sha256sum | cut -d' ' -f1 || true)
  if [ "$have" != "$want" ]; then
    # tools absent (fresh branch from main) are vendored from qualitycoding/skills by S-010; present-but-different is an error
    if git cat-file -e "HEAD:$f" 2>/dev/null; then echo "FAIL: $f hash $have != $want" >&2; exit 4; fi
    TOOLS_MISSING="${TOOLS_MISSING:-} $f"
  fi
done
SPIKE="$(mktemp -d)"; trap 'rm -rf "$SPIKE"' EXIT
bash Tools/build_render.sh new "$SPIKE/render_new" >/dev/null
( cd "$SPIKE" && ./render_new demo out >/dev/null )
NEW=$(sha256sum "$SPIKE/out.wav" | cut -d' ' -f1)
COMMITTED=$(sha256sum Renders/after/oildrumkit_after.wav | cut -d' ' -f1)
RENDER=identical; [ "$NEW" = "$COMMITTED" ] || RENDER=differs
cat > "$WORK/bootstrap.json" <<J
{"repo": "$URL", "main": "$MAIN", "resumed_existing_branch": $RESUMED, "branch": "impl/listening-validation", "base": "$BASE",
 "code_changed_since_audited_commit": "${DRIFT}", "vendored_tools_missing": "${TOOLS_MISSING:-}",
 "render_sha256": "$NEW", "committed_render_sha256": "$COMMITTED", "render": "$RENDER"}
J
cat "$WORK/bootstrap.json"
[ "$RENDER" = identical ] || { echo "render differs from the committed render: run S-010 decision rule D-003" >&2; exit 5; }
