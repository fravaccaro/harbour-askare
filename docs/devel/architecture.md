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
| `qml/components/*` | Shared QML such as the SupportMe dialog |
| `qml/modules/Opal/*` | Bundled Opal About, SupportMe, and LinkHandler modules |

`src/main.cpp` publishes four context properties - `Askare` (the session), `Calendars`, `Tasks` and `Reminders` - and the pages bind to those. It also adds `qml/modules` to the QML import path so the bundled Opal modules resolve at runtime. Networking and ICS handling stay in C++; QML holds no request and parses no calendar data.

## Messages to the user

A passing message has one route: `app.showToast()` in `qml/harbour-askare.qml` hands it to the single `AskareNotification`, which publishes a transient notice. Anything the session reports through `errorMessage` - a failed sync, a server that refused a change - arrives that way, as does a task that is no longer there. No page carries its own copy of `errorMessage`.

A failed sign-in is the exception: `loginFailed` reaches the sign-in pages, which say so in place, because the reason belongs to the form the user is looking at. That is why `onLoginFailed()` in `session.cpp` does not also set `errorMessage`.

Reminders keep their own notifications, in `reminderNoticeComponent`: those persist, carry **Open** and **Mark done**, and are not toasts.

## CalDAV rules

A sync walks the server in a fixed order: PROPFIND for the current user principal, PROPFIND for its calendar home set, PROPFIND for the calendars under it, then one REPORT per calendar for its VTODOs. Only afterwards does it upload what the device changed, with PUT and DELETE.

Collection hrefs keep their **trailing slash**, through `Url::pathWithSlash()`, so a task href built from a calendar href matches what the server returns.

A change is uploaded with `If-Match` carrying the ETag the task was last read at, and a new task with `If-None-Match: *`, so neither a task changed on both sides nor a name already taken is overwritten. A task the server refuses stays marked dirty and is offered again on the next run; only a run that uploaded something may pull afterwards, so a rejected task cannot drive an endless loop.

Reminders are a `VALARM` inside the VTODO, with a trigger relative to the end of the task, which is how Nextcloud Tasks writes them. Subtasks are a `RELATED-TO` naming the UID of the parent, which resolves only within one calendar. A subtask is therefore created in its parent's list, and a subtask moved to another list is detached: `moveSubtree()` passes the root through `detached()`, which clears `parentUid` on the row and rewrites the ICS without the parent link, so it arrives as a task of its own. This is not only tidiness: `Store::topLevelClause()` and `Store::children()` match `parent_uid` across the whole table, with no calendar scope, so a moved subtask that kept its parent would be dropped from the top-level views while still listed under its old parent in the list it left.

A task has no address of its own beyond its collection: `hrefIn()` builds `href` from the calendar href and the UID, and `href` is the primary key of the `tasks` table. Carrying a task to another list is therefore a create followed by a delete, which `Session::moveSubtree()` writes with the pieces that already exist: the task and every `Store::descendants()` entry are upserted at their new hrefs with no ETag, so the push uploads them with `If-None-Match: *`, and `dropTask()` then retires each original - removed outright when the server never saw it, otherwise marked deleted so the push issues the `DELETE`. Nothing in the DAV layer knows about moving.

The whole subtree travels together, since `RELATED-TO` would otherwise point at a UID outside the collection; only the root loses its parent, the links inside the subtree are kept. The UID is kept too, which is what holds those inner links together, and so is the reminded stamp, so a move does not replay a reminder. Creating before deleting means a `DELETE` that fails leaves a copy in the old list for the next push to retry, rather than losing the task. The old and new rows share a UID until the push completes; every query that reads relations filters on `deleted = 0`, so only the new subtree is seen.

A move changes the href a page may be holding, so `Session` emits `taskMoved(oldHref, newHref)` and `TaskDetailPage` follows it. Without that the page would keep acting on the row left behind, and `editedTask()` clears the deleted flag, so the next pulley action there would put the task back in both lists.

## Sandbox layout

Sailjail gives the app three private directories, named after the `OrganizationName` and `ApplicationName` of the desktop file:

* `~/.config/org.frapps/harbour-askare` - `settings.conf`
* `~/.local/share/org.frapps/harbour-askare` - `askare.db`
* `~/.cache/org.frapps/harbour-askare` - whatever the web view leaves behind; nothing the app writes itself

The permissions are `Internet`, `Secrets` and `WebView`; no document, picture or calendar directory is asked for. The config and share directories are declared for the system backup through `X-HarbourBackup`.

## Reminders without a service

`Reminders` checks the stored tasks every minute while the app runs, and each check notes the moment it announced, so a reminder fires once and an unchanged due date stays silent. That timer only ticks while the device is awake, so `qml/harbour-askare.qml` holds a `BackgroundJob` that wakes the app every five minutes for as long as a reminder is still ahead - `Reminders.armed` says whether one is.
