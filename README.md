---
layout: default
title: Home
nav_order: 1
description: "A native Nextcloud Tasks client for Sailfish OS"
permalink: /
---

# Askare

Askare is a native [Nextcloud Tasks](https://apps.nextcloud.com/apps/tasks) client for Sailfish OS. It talks over CalDAV to a Nextcloud server that you already run, and keeps a copy of the lists on the device.

Askare does not use the Nextcloud account already in Settings. The system calendar only syncs events, so this app keeps its own copy of the tasks.

[![GitHub license](https://img.shields.io/github/license/fravaccaro/harbour-askare.svg)](https://github.com/fravaccaro/harbour-askare/blob/main/LICENSE) [![GitHub issues](https://img.shields.io/github/issues/fravaccaro/harbour-askare.svg)](https://github.com/fravaccaro/harbour-askare/issues) [![GitHub releases](https://img.shields.io/github/release/fravaccaro/harbour-askare.svg)](https://github.com/fravaccaro/harbour-askare/releases/latest)

## Donate

The SailfishOS Community Team is on Liberapay:

[![Liberapay receiving](https://img.shields.io/liberapay/receives/SailfishOScommunityTeam?logo=liberapay&label=SailfishOSCommunity)](https://liberapay.com/SailfishOScommunityTeam)

[![Liberapay receiving](https://img.shields.io/liberapay/receives/fravaccaro?logo=liberapay&label=fravaccaro)](https://liberapay.com/fravaccaro)

## Features

- Sign in through the server's own page, inside the app, so single sign-on and two-factor authentication work as they do in a browser.
- Every task list of the server, plus the All, Current and Completed views across lists.
- Add, edit and delete tasks with a title, completion, priority, start and due dates, and notes written in Markdown.
- Subtasks under a task, counted on the row of their parent.
- A reminder before a task is due, announced while Askare is running.
- A local copy of the lists, synced by hand, when the app starts, or on a timer.

## Requirements

- Sailfish OS 4.5 or newer, for Sailfish Secrets and the WebView
- A reachable Nextcloud server with the Tasks app, reached over CalDAV

## Using Askare

[Using Askare](docs/guide.md) - signing in, working through the lists, editing tasks, reminders, and where your data lives.

## Download

RPMs are attached to the [GitHub releases](https://github.com/fravaccaro/harbour-askare/releases/latest).

## Developers

[Developers](docs/devel/) - architecture, building, deploying, and CI (maintainers and contributors).

## Translate

Request a new language or contribute on the [Transifex project page](https://app.transifex.com/fravaccaro/askare).

## Credits

- [Nextcloud Tasks](https://apps.nextcloud.com/apps/tasks), the server app whose to-dos this client reads and writes.
- [Opal](https://github.com/Pretty-SFOS/opal) QML modules (About, SupportMe, LinkHandler) by [Mirian Margiani](https://github.com/Pretty-SFOS/opal-about).
- Thanks to all translators and testers.

## AI disclosure

- **Cursor-assisted work.** Askare is developed with [Cursor](https://cursor.com) as an IDE with AI assistance, for tasks such as scaffolding, documentation, translation upkeep and RPM packaging. Output is reviewed and edited by the maintainer before commit.
- **Not a substitute for testing.** AI suggestions do not replace testing on Sailfish OS hardware, reading the code, or applying your own knowledge. Generated changes are treated like any other patch: understand it, test it, then ship it.

## Licence

[GPL-3.0-only](https://github.com/fravaccaro/harbour-askare/blob/main/LICENSE)
