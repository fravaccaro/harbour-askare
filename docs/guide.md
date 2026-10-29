---
layout: default
title: Using Askare
nav_order: 2
permalink: docs/guide
has_children: true
has_toc: true
---

# Using Askare

Askare is a native client for the task lists of a Nextcloud server you already run. The tasks live on the server as CalDAV to-dos; the phone keeps a copy of them so the lists can be read and changed while the server is out of reach.

Askare does not use the Nextcloud account in the Sailfish OS settings. That account syncs the calendar, which carries events but not to-dos, so this app signs in on its own.

## Requirements

* Sailfish OS 5.0 or newer (the app password is kept in Sailfish Secrets, and signing in uses the WebView)
* A reachable Nextcloud server with the Tasks app

## The pages

* **[Signing in](signin)** - the server address, the sign-in page of the server, and what Askare keeps afterwards
* **[Lists and tasks](lists)** - the task lists, the All, Current and Completed views, and the cover
* **[Viewing and editing](editing)** - dates, priority, completion, notes in Markdown, and subtasks
* **[Reminders](reminders)** - a notice before a task is due, and what it takes for one to arrive
* **[Sync, files and permissions](files)** - when the lists are refreshed, where the copy is kept, and why the app asks for each permission

## Further help

* [Report an issue](https://github.com/fravaccaro/harbour-askare/issues)
