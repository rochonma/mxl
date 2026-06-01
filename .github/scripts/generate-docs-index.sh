#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2025 Contributors to the Media eXchange Layer project.
# SPDX-License-Identifier: CC-BY-4.0
#
# Generate docs/index.md from the project README.md, rewriting links so they
# resolve correctly when the site is built from the docs/ directory:
#
#   - Links to docs/<file>           -> relative <file>           (sibling pages)
#   - Links to repo-root files       -> absolute github.com/.../main/... URLs
#   - Pinned-SHA GitHub blob image   -> local docs-relative image
#
# The README.md remains the single source of truth.

set -euo pipefail

REPO_URL="https://github.com/dmf-mxl/mxl/blob/main"
SRC="README.md"
DST="docs/index.md"

if [[ ! -f "${SRC}" ]]; then
    echo "error: ${SRC} not found (run from repo root)" >&2
    exit 1
fi

sed -E \
    -e "s#\]\(docs/([^)]+)\)#](\1)#g" \
    -e "s#\]\(\./?LICENSE\.txt\)#](${REPO_URL}/LICENSE.txt)#g" \
    -e "s#\]\(LICENSE\.txt\)#](${REPO_URL}/LICENSE.txt)#g" \
    -e "s#\]\(CONTRIBUTING\.md\)#](${REPO_URL}/CONTRIBUTING.md)#g" \
    -e "s#\]\(SECURITY\.md\)#](${REPO_URL}/SECURITY.md)#g" \
    -e "s#\]\(CODE_OF_CONDUCT\.md\)#](${REPO_URL}/CODE_OF_CONDUCT.md)#g" \
    -e "s#\]\(GOVERNANCE/([^)]+)\)#](${REPO_URL}/GOVERNANCE/\1)#g" \
    -e "s#\]\(examples/([^)]+)\)#](${REPO_URL}/examples/\1)#g" \
    -e "s#https://github.com/dmf-mxl/mxl/blob/[0-9a-f]+/docs/([^)\" ]+)#\1#g" \
    "${SRC}" > "${DST}"

echo "Generated ${DST} from ${SRC}"
