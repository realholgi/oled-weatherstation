#!/usr/bin/env sh
set -eu

version_file="${VERSION_FILE:-VERSION}"

if [ ! -f "$version_file" ]; then
    printf '0.0.0-dev\n'
    exit 0
fi

version="$(tr -d '[:space:]' < "$version_file")"
release_tag="v${version}"
actual_tag=""
worktree_clean=false

if git rev-parse --is-inside-work-tree >/dev/null 2>&1; then
    actual_tag="$(git describe --exact-match --tags HEAD 2>/dev/null || true)"
    if [ -z "$(git status --porcelain)" ]; then
        worktree_clean=true
    fi
fi

if [ "$actual_tag" = "$release_tag" ] && [ "$worktree_clean" = true ]; then
    printf '%s\n' "$version"
else
    printf '%s-dev\n' "$version"
fi
