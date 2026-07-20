#!/usr/bin/env bash
set -euo pipefail

version="${1:?usage: generate-sbom.sh VERSION}"
created="$(date -u +%Y-%m-%dT%H:%M:%SZ)"

printf '%s\n' \
  'SPDXVersion: SPDX-2.3' \
  'DataLicense: CC0-1.0' \
  'SPDXID: SPDXRef-DOCUMENT' \
  "DocumentName: Verbuno-$version" \
  "DocumentNamespace: https://github.com/Trendorin/Verbuno/releases/tag/v$version/spdx" \
  'Creator: Organization: Trendorin' \
  "Created: $created" \
  '' \
  '##### Package: Verbuno' \
  '' \
  'PackageName: Verbuno' \
  'SPDXID: SPDXRef-Package-Verbuno' \
  "PackageVersion: $version" \
  "PackageDownloadLocation: https://github.com/Trendorin/Verbuno/releases/tag/v$version" \
  'FilesAnalyzed: false' \
  'PackageLicenseConcluded: GPL-3.0-or-later' \
  'PackageLicenseDeclared: GPL-3.0-or-later' \
  'PackageCopyrightText: Copyright 2026 Trendorin' \
  'ExternalRef: PACKAGE-MANAGER purl pkg:github/Trendorin/Verbuno' \
  '' \
  'Relationship: SPDXRef-DOCUMENT DESCRIBES SPDXRef-Package-Verbuno' \
  '' \
  '##### System dependency: Tesseract OCR' \
  '' \
  'PackageName: Tesseract OCR' \
  'SPDXID: SPDXRef-Package-Tesseract' \
  'PackageDownloadLocation: https://github.com/tesseract-ocr/tesseract' \
  'FilesAnalyzed: false' \
  'PackageLicenseConcluded: Apache-2.0' \
  'PackageLicenseDeclared: Apache-2.0' \
  'PackageCopyrightText: NOASSERTION' \
  '' \
  'Relationship: SPDXRef-Package-Verbuno DEPENDS_ON SPDXRef-Package-Tesseract' \
  '' \
  '##### System dependency: Leptonica' \
  '' \
  'PackageName: Leptonica' \
  'SPDXID: SPDXRef-Package-Leptonica' \
  'PackageDownloadLocation: https://github.com/DanBloomberg/leptonica' \
  'FilesAnalyzed: false' \
  'PackageLicenseConcluded: Leptonica' \
  'PackageLicenseDeclared: Leptonica' \
  'PackageCopyrightText: NOASSERTION' \
  '' \
  'Relationship: SPDXRef-Package-Tesseract DEPENDS_ON SPDXRef-Package-Leptonica'
