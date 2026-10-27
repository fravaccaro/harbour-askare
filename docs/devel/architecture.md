---
layout: default
title: Architecture
parent: Developers
nav_order: 1
---

# Architecture

## Source map

| Part | Purpose |
|---|---|
| `src/askare/config.*` | Server URL, user name, TLS preference and the sync settings in `~/.config/org.frapps/harbour-askare/settings.conf` |
| `src/askare/secretsstore.*` | The app password in a device-lock collection of Sailfish Secrets |
| `src/askare/loginflow.*` | Nextcloud login flow v2: ask for a login page, then poll until the server hands over an app password |
| `src/askare/httpclient.*` | `QNetworkAccessManager` wrapper: basic auth, custom methods, ETags, TLS preference |
| `src/askare/davclient.*` | CalDAV PROPFIND, REPORT, PUT and DELETE, and the order they run in |
| `src/askare/davxml.*` | Readers for the DAV responses: principal, home set, calendars, to-dos, errors |
| `src/askare/ics.*` | VTODO unfold, parse, rewrite, and the date arithmetic reminders need |
| `src/askare/markdown.*` | The notes, rendered to the subset of HTML a Silica `Label` draws |
| `src/askare/url.*` | Server URL normalisation and href joining |
| `src/askare/store.*` | SQLite copy of lists and tasks, in `askare.db` |
| `src/askare/session.*` | QML-facing controller: login, sync, save, delete |
| `src/askare/calendarmodel.*` | Task lists, with their colour and open count |
| `src/askare/taskmodel.*` | The tasks of one view, grouped and ordered for the list |
| `src/askare/reminders.*` | Watches stored tasks and reports a reminder whose moment has passed |
| `qml/pages/*` | Silica UI |

`src/main.cpp` publishes four context properties - `Askare` (the session), `Calendars`, `Tasks` and `Reminders` - and the pages bind to those. Networking and ICS handling stay in C++; QML holds no request and parses no calendar data.

## CalDAV rules

A sync walks the server in a fixed order: PROPFIND for the current user principal, PROPFIND for its calendar home set, PROPFIND for the calendars under it, then one REPORT per calendar for its VTODOs. Only afterwards does it upload what the device changed, with PUT and DELETE.

Collection hrefs keep their **trailing slash**, through `Url::pathWithSlash()`, so a task href built from a calendar href matches what the server returns.

A change is uploaded with `If-Match` carrying the ETag the task was last read at, and a new task with `If-None-Match: *`, so neither a task changed on both sides nor a name already taken is overwritten. A task the server refuses stays marked dirty and is offered again on the next run; only a run that uploaded something may pull afterwards, so a rejected task cannot drive an endless loop.

Reminders are a `VALARM` inside the VTODO, with a trigger relative to the end of the task, which is how Nextcloud Tasks writes them. Subtasks are a `RELATED-TO` naming the UID of the parent, which resolves only within one calendar - hence a subtask is pinned to the list of its parent.

## Sandbox layout

Sailjail gives the app three private directories, named after the `OrganizationName` and `ApplicationName` of the desktop file:

* `~/.config/org.frapps/harbour-askare` - `settings.conf`
* `~/.local/share/org.frapps/harbour-askare` - `askare.db`
* `~/.cache/org.frapps/harbour-askare` - whatever the web view leaves behind; nothing the app writes itself

The permissions are `Internet`, `Secrets` and `WebView`; no document, picture or calendar directory is asked for. The config and share directories are declared for the system backup through `X-HarbourBackup`.

## Reminders without a service

`Reminders` checks the stored tasks every minute while the app runs, and each check notes the moment it announced, so a reminder fires once and an unchanged due date stays silent. That timer only ticks while the device is awake, so `qml/harbour-askare.qml` holds a `BackgroundJob` that wakes the app every five minutes for as long as a reminder is still ahead - `Reminders.armed` says whether one is.
