# Verbuno 0.3.0

This release adds a complete review-first workflow for translating text from photos without uploading the image itself.

## Highlights

- open a photo from disk, paste one from the clipboard or drop it directly onto the translation workspace;
- decode and recognize PNG, JPEG, WebP, BMP and TIFF images locally with the system Tesseract 5 library;
- run OCR outside the UI thread so the standard resizable Qt window remains responsive;
- discover installed Tesseract language data at runtime and ship English, German, Russian and Ukrainian support with native packages;
- choose automatic, single-block or sparse-text page segmentation and retry recognition without reopening the image;
- automatically apply a second local contrast pass when the initial OCR confidence is low;
- preview the selected image, inspect dimensions, recognition language and confidence, then edit the extracted text before translating;
- reject oversized or unsupported images and bound both processing resolution and extracted text;
- keep image pixels, local paths, filenames and OCR metadata out of provider requests and local history.

Photo recognition never starts a network request. Only the extracted text is sent to the configured provider, and only after the user explicitly presses **Translate**. OCR quality still depends on the image, typography and installed language data, so the editable result should be reviewed.

## Release files

- Arch Linux `.pkg.tar.zst`
- Fedora 44 `.rpm`
- Ubuntu 24.04 `.deb`
- install tree `.tar.xz`
- source `.tar.xz`
- `PKGBUILD`
- SPDX 2.3 document
- `SHA256SUMS`

Every binary package is built and installed in its native target before publication. Verify downloaded files with:

```bash
sha256sum --ignore-missing --check SHA256SUMS
```

Translation text is sent to the configured external provider. The displayed upstream route is response metadata, not a privacy guarantee. Review OpenRouter and the selected inference provider's current retention and training policies before submitting sensitive text.
