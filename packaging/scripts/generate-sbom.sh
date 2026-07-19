#!/usr/bin/env bash
set -euo pipefail

version="${1:?usage: generate-sbom.sh VERSION}"
created="$(date -u +%Y-%m-%dT%H:%M:%SZ)"

printf '%s\n' \
  'SPDXVersion: SPDX-2.3' \
  'DataLicense: CC0-1.0' \
  'SPDXID: SPDXRef-DOCUMENT' \
  "DocumentName: TranslUnix-$version" \
  "DocumentNamespace: https://github.com/Trendorin/TranslUnix/releases/tag/v$version/spdx" \
  'Creator: Organization: Trendorin' \
  "Created: $created" \
  '' \
  '##### Package: TranslUnix' \
  '' \
  'PackageName: TranslUnix' \
  'SPDXID: SPDXRef-Package-TranslUnix' \
  "PackageVersion: $version" \
  "PackageDownloadLocation: https://github.com/Trendorin/TranslUnix/releases/tag/v$version" \
  'FilesAnalyzed: false' \
  'PackageLicenseConcluded: GPL-3.0-or-later' \
  'PackageLicenseDeclared: GPL-3.0-or-later' \
  'PackageCopyrightText: Copyright 2026 Trendorin' \
  'ExternalRef: PACKAGE-MANAGER purl pkg:github/Trendorin/TranslUnix' \
  '' \
  'Relationship: SPDXRef-DOCUMENT DESCRIBES SPDXRef-Package-TranslUnix'
