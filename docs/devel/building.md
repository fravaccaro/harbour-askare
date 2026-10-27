---
layout: default
title: Building
parent: Developers
nav_order: 2
---

# Building

## With the Sailfish SDK

With the [Sailfish SDK](https://docs.sailfishos.org/Tools/Sailfish_SDK/Installation/) installed:

```bash
sfdk config target SailfishOS-5.0.0.62-aarch64
sfdk qmake
sfdk make
sfdk rpm
```

Deploy to a connected device:

```bash
sfdk config device "My Phone"
sfdk deploy --sdk
```

The app has been built for Sailfish OS 5.0.

## Continuous integration

RPMs are built by [`.github/workflows/build.yml`](https://github.com/fravaccaro/harbour-askare/blob/main/.github/workflows/build.yml) for `aarch64`, `armv7hl` and `i486` on every push to `main` and on pull requests, and attached to a GitHub release when one is published.

The documentation site is built by `.github/workflows/pages.yml` (Jekyll with the just-the-docs theme, configured in `_config.yml`) and deployed to GitHub Pages. The marketing page under `landing/` is kept out of that build; its published copy lives in the portfolio site.

## Packaging notes

Harbour refuses an unstripped binary, and the SDK packaging path does not always strip by itself, so `rpm/harbour-askare.spec` strips the binary before the package is sealed. The spec also carries the Chum metadata block.

The spec file excludes automatic `Provides:` for the bundled Opal QML modules - Harbour forbids an application package from providing QML modules.

The runtime dependencies are the Silica components, the WebView, the notification and D-Bus QML plugins, `libkeepalive` for the reminder wake-ups, and the multimedia import for the reminder tone. Sailfish Secrets is linked through `pkgconfig(sailfishsecrets)` in `harbour-askare.pro`.

## Translations

Italian ships with the app. Translators work on the [Transifex project](https://explore.transifex.com/fravaccaro/askare). A new language is a further `.ts` file beside it:

1. Copy `translations/harbour-askare.ts` to `translations/harbour-askare-<code>.ts`.
2. Add it to `TRANSLATIONS` in `harbour-askare.pro`.
3. Translate it with Qt Linguist and build; `sailfishapp_i18n` compiles and installs the `.qm`.

Compiled `.qm` files are build artefacts and stay out of the repository.
