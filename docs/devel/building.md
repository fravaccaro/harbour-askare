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

`main.cpp` claims the application's bus name, so the build needs `QT += dbus` and the spec a `BuildRequires: pkgconfig(Qt5DBus)`.

## Translations

English and Italian ship with the app. Translators work on the [Transifex project](https://explore.transifex.com/fravaccaro/askare). A new language is a further `.ts` file beside it:

1. Copy `translations/harbour-askare.ts` to `translations/harbour-askare-<code>.ts`.
2. Add it to `TRANSLATIONS` in `harbour-askare.pro`.
3. Translate it with Qt Linguist and build; `sailfishapp_i18n` compiles and installs the `.qm`.
4. Add the language to the desktop file's strings too, which are a resource of their own: write the code into `translations/desktop/LINGUAS`, create the `.po` beside it with `msginit`, and run `scripts/desktop-translations.sh`.

Compiled `.qm` files are build artefacts and stay out of the repository.

### The strings in the desktop file

`harbour-askare.desktop` carries two strings of its own: the app's `Comment`, and the `Description` of the share method, which is what the system's sharing list calls Askare. Neither can come from the `.ts` files. A desktop file is read by the launcher and the sharing dialog rather than by the app, and each language needs a line of its own in the file - `Comment[it]`, `Description[it]` - in the style the desktop entry specification defines. A language code covers its regions, so `Description[it]` is what an `it_IT` phone reads, and a region needs a line only where the wording differs.

There is no way round that in a Harbour package. `MDesktopEntry` can take a translation from a catalog instead, through an `X-Amber-Translation-Id-Description` key, but it loads the catalog from `/usr/share/translations`, and a Harbour package may install nothing outside `/usr/share/harbour-askare`. The Sailfish Share documentation says as much: a method description is localised with desktop entry keys and falls back to the unlocalised value.

So they travel as **gettext**, a second Transifex resource beside the Qt one:

| File | Is |
|---|---|
| `harbour-askare.desktop.in` | the template, which holds the English and the notes to the translators |
| `translations/desktop/harbour-askare.pot` | the source resource, uploaded to Transifex |
| `translations/desktop/<code>.po` | what a translator sends back |
| `translations/desktop/LINGUAS` | the languages the next run should write |
| `harbour-askare.desktop` | **generated**, and committed |

`scripts/desktop-translations.sh` does the whole round trip: `xgettext` rewrites the `.pot` from the template, `msgmerge` brings each `.po` up to date with it, and `msgfmt --desktop` writes the desktop file with a translated line above every source one. Run it after editing the template or pulling translations, and commit what it changes.

The desktop file is generated **and committed** on purpose: the package is built by the SDK, by GitHub Actions and by OBS, and none of them should need gettext to produce a file that changes twice a year. The file says at the top that it is generated, since an edit made in it rather than in the template would be dropped by the next run.

The app's own **New task**, in the pulley menu, is a different string in a different resource and is translated separately.
