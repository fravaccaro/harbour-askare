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
| `src/askare/loginflow.*` | Nextcloud login flow v2: ask for a login page, then poll until the server hands over an app password, or give up after twenty minutes, when the server drops the token |
| `src/askare/httpclient.*` | `QNetworkAccessManager` wrapper: basic auth, custom methods, ETags, TLS preference, and a watchdog that abandons a reply thirty seconds after the last byte moved |
| `src/askare/davclient.*` | CalDAV PROPFIND, REPORT, PUT and DELETE, and the order they run in |
| `src/askare/davxml.*` | Readers for the DAV responses: principal, home set, calendars, to-dos, errors |
| `src/askare/ics.*` | VTODO unfold, parse, rewrite, and the date arithmetic reminders need |
| `src/askare/markdown.*` | The notes, rendered to the subset of HTML a Silica `Label` draws |
| `src/askare/url.*` | Server URL normalisation and href joining |
| `src/askare/sharedtext.*` | Text and addresses another app shared, made fit to write into a task |
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

Reminders keep their own notifications, published through `reminderNotice`: those persist, carry **Open** and **Mark done**, and are not toasts. The one notice object serves every reminder, since `publish()` asks the notification manager for a fresh id whenever it is given none, so a second reminder stands beside the first rather than replacing it.

A reminder outlives the run that published it, and `Notification` delivers `clicked` and `actionInvoked` only to the object still holding the notice's id - the plugin says as much, and `checkActionInvoked()` compares the id it is given against `replacesId`. A notice tapped after a restart would therefore reach nobody. So each action instead carries a full D-Bus call, which `reminderAction()` builds and the notification manager places itself, and the `DBusAdaptor` in `qml/harbour-askare.qml` answers: `openTask` raises the window on the task, `markTaskDone` completes it where it stands. The call goes by name, so it arrives at whichever instance of the app is running. Nothing listens for the in-process signals any more, because the manager raises both - the remote call and `ActionInvoked` - and every tap would otherwise be served twice.

The name is the `OrganizationName` and `ApplicationName` of the desktop file, `org.frapps.harbour-askare`, which is what Sailjail lets the app own; the path and the interface drop the hyphen, which a bus name allows and they do not. It is claimed in `main.cpp`, once `view->show()` has returned, rather than through the `service` property of an adaptor: the share receiver puts an object of its own on the bus, and every object has to be in place before the name starts answering calls. The manager closes a notice itself once an action is invoked, so neither adaptor method does.

The package deliberately installs no `dbus-1/services` file - the `Paths` suite of `sfdk check` answers `Installation not allowed in this location` for `/usr/share/dbus-1/services`. What starts the app from cold instead is `ExecDBus=harbour-askare` in the `[X-Sailjail]` section of the desktop file, which Harbour accepts. The activation belongs to the bus name rather than to whoever calls it, so a reminder tapped while the app is gone arrives by the same route a share does.

## CalDAV rules

A sync walks the server in a fixed order: PROPFIND for the current user principal, PROPFIND for its calendar home set, PROPFIND for the calendars under it, then one REPORT per calendar for its VTODOs. Only afterwards does it upload what the device changed, with PUT and DELETE.

Collection hrefs keep their **trailing slash**, through `Url::pathWithSlash()`, so a task href built from a calendar href matches what the server returns.

A change is uploaded with `If-Match` carrying the ETag the task was last read at, and a new task with `If-None-Match: *`, so neither a task changed on both sides nor a name already taken is overwritten. A task the server refuses stays marked dirty and is offered again on the next run; only a run that uploaded something may pull afterwards, so a rejected task cannot drive an endless loop.

`Store::dirtyTasks()` orders the queue `ORDER BY deleted`, so writes go out before deletes. A task carried to another list is both at once, and taking it away first would leave it on no server at all until the write that puts it in its new list goes through.

The answer to a write is taken against **the task that was sent**, held in `m_inFlight`, and not against the row as it stands now. `saveTask()` writes to the store whatever is on the wire, so a second edit landing while the first PUT is still travelling leaves the row holding something the server has not been told about. `onUploaded()` compares the row's ICS with `m_inFlight`: where they still agree it calls `Store::markClean()` as before, and where they do not - a newer edit, or a delete made meanwhile, which `markClean()` would unmark - it calls `Store::setEtag()`, which takes the version the server just gave but leaves the task owed. Clearing the mark then would strand the edit silently: the row would go on reading correctly until the next pull, where `upsertRemoteTask()` skips only dirty rows and would write the server's older copy over it.

Saving pushes straight away, through `saveTask()`'s own `pushDirty()`; the **Refresh after saving a task** setting decides only whether a pull follows. An edit made while a run is already on the wire cannot push there and then, so `pushDirty()` notes it in `m_pushDeferred` and the run honours it when it ends - `finishSync()` already pushes what is owed, and `onDavFailed()` pushes once more when the flag is set, which is what carries an edit past a run that failed under it. Only an edit sets the flag and `pushDirty()` clears it before sending, so a server that keeps refusing is tried once rather than in a circle.

A refused version is a question rather than a failure. `Store::mergeRemote()` returns the remote versions it would not write because the task carries an edit of its own made against a version the server no longer holds, and `Session::noteConflict()` either settles the pair - the two say the same thing field for field, or the task was deleted here and only wants the ETag the delete is refused over - or keeps it for `ConflictPage` to ask about. The pairs live no longer than the app does, so a question left unanswered is met again by the next sync. The merge is where this is caught, rather than the 412: a CalDAV refusal carries no body to read the server's version from, and the REPORT that precedes every push has already read it. A task waiting to be settled is kept out of the push queue by `withoutConflicts()`, and so is one the server refused outright, held in `m_refused` until a pull says what to make of it - without that, `finishSync()`'s own `pushDirty()` would offer the same refused version again in a circle.

The pull may answer that refusal by **not mentioning the task at all**, which `Session::settleVanished()` reads as the server no longer having it: the `If-Match` named a resource deleted elsewhere since, and a REPORT is taken for the whole of its calendar, as `dropMissing()` already takes it. A task marked `deleted` here is then simply removed, the delete having happened anyway. Any other is an edit nobody has seen, so `restartVanished()` drops its ETag and leaves it owed: the next write is the `If-None-Match: *` that puts the task back under the href and uid it already has, which is what keeps a subtask naming its parent. Nothing else settles that case - `noteConflict()` only ever sees tasks the pull returned, and `dropMissing()` skips dirty rows - so without it the task stayed owed, held in `m_refused` for as long as the app ran, offering an edit that could never go out.

Reminders are a `VALARM` inside the VTODO, with a trigger relative to the end of the task, which is how Nextcloud Tasks writes them. Subtasks are a `RELATED-TO` naming the UID of the parent, which resolves only within one calendar. A subtask is therefore created in its parent's list, and a subtask moved to another list is detached: `moveSubtree()` passes the root through `detached()`, which clears `parentUid` on the row and rewrites the ICS without the parent link, so it arrives as a task of its own. This is not only tidiness: `Store::topLevelClause()` and `Store::children()` match `parent_uid` across the whole table, with no calendar scope, so a moved subtask that kept its parent would be dropped from the top-level views while still listed under its old parent in the list it left.

A task has no address of its own beyond its collection: `hrefIn()` builds `href` from the calendar href and the UID, and `href` is the primary key of the `tasks` table. Carrying a task to another list is therefore a create followed by a delete, which `Session::moveSubtree()` writes with the pieces that already exist: the task and every `Store::descendants()` entry are upserted at their new hrefs with no ETag, so the push uploads them with `If-None-Match: *`, and `dropTask()` then retires each original - removed outright when the server never saw it, otherwise marked deleted so the push issues the `DELETE`. Nothing in the DAV layer knows about moving.

The whole subtree travels together, since `RELATED-TO` would otherwise point at a UID outside the collection; only the root loses its parent, the links inside the subtree are kept. The UID is kept too, which is what holds those inner links together, and so is the reminded stamp, so a move does not replay a reminder. Creating before deleting - in the store, and on the wire through the ordered queue - means a `DELETE` that fails leaves a copy in the old list for the next push to retry, rather than losing the task. The old and new rows share a UID until the push completes; every query that reads relations filters on `deleted = 0`, so only the new subtree is seen.

A move changes the href a page may be holding, so `Session` emits `taskMoved(oldHref, newHref)` and `TaskDetailPage` follows it. Without that the page would keep acting on the row left behind, and `editedTask()` clears the deleted flag, so the next pulley action there would put the task back in both lists.

A list is a model and reads itself again, but a single task has no model of its own: `TaskDetailPage` holds one task in its own properties, which `Session::taskFields()` names and `pushTask()` builds the page from. The page keeps nothing of its own beyond that, and reads every field again on `tasksChanged` - its `reload()` is the one place a field is copied, over properties named after the keys `taskFields()` returns. An editor that mirrored its fields back on `accepted` would answer only for the edit the page itself opened, leaving a row's own **Edit**, a quick toggle and an incoming sync to go unseen until the page was pushed again.

## Sandbox layout

Sailjail gives the app three private directories, named after the `OrganizationName` and `ApplicationName` of the desktop file:

* `~/.config/org.frapps/harbour-askare` - `settings.conf`
* `~/.local/share/org.frapps/harbour-askare` - `askare.db`
* `~/.cache/org.frapps/harbour-askare` - whatever the web view leaves behind; nothing the app writes itself

The permissions are `Internet`, `Secrets` and `WebView`; no document, picture or calendar directory is asked for. The config and share directories are declared for the system backup through `X-HarbourBackup`.

## Reminders without a service

`Reminders` checks the stored tasks every minute while the app runs, and each check notes the moment it announced, so a reminder fires once and an unchanged due date stays silent. That timer only ticks while the device is awake, so `qml/harbour-askare.qml` holds a `BackgroundJob` that wakes the app every five minutes for as long as a reminder is still ahead - `Reminders.armed` says whether one is.

## Shares from other apps

The desktop file declares a single share method, `task`, taking `text/plain` and `text/x-url` together. Two methods would have the sharing dialog name Askare twice over, because a page shared from the browser satisfies both.

A share is received through the `DBusAdaptor` on `/share/task` in `qml/components/ShareReceiver.qml`, which reads the configuration it is handed into the resources of a `ShareAction`. There is deliberately no `ShareProvider`: it would own that same path, leaving the adaptor unreachable whenever the app was already running, and it hands on `ShareResource` objects, which carry only a `name` and `data` and so have nowhere to put the address and the page title a link is made of.

`ShareAction.resources` is a plain `QVariantList`, so each resource is a map holding the keys the system's own share pages read - `type`, which is a mime type here rather than the `ShareResource` enum, beside `status` and `linkTitle`. The type is therefore what says whether a share is a link or a passage, and is believed over anything else: a passage shared from the browser arrives as `text/plain` with its text in `status` and no `data`, so guessing from the fields alone would take it for an address and throw it away as one no task could follow. Only a share naming no type at all is guessed at.

`SharedText` makes what arrived fit to store - the line endings of wherever it was written, and no control characters - and `Session::sharedTextFields()` and `sharedLinkFields()` shape it into a map keyed the way `taskFields()` is, so the editor opens on it exactly as it opens on an existing task. Neither writes anything: the task is the editor's to save. An empty map says the share held nothing worth writing down, which is also what an address no task could have followed comes back as.

The window holds the fields in `pendingShare` until it can push the editor over something that will still be there afterwards. `takePendingShare()` waits while the settings are still being read, while no list has been read yet - an editor with none to offer cannot be accepted - while the page stack is busy, and while the current page is the start page or either sign-in page, since the navigation that follows those throws a pushed page away. `shareAgain` tries again every tenth of a second, `showMain()` tries once the stack has been pointed at the tasks, and `Calendars.onCountChanged` tries once the lists arrive. The fields are cleared only when the push lands, so a wipe cannot drop the only copy of what was shared.

A pending share, a new task asked for from the cover and a task a reminder asked to open all wait on the same things, so `takePending()` offers all three at once and every one of those wake-ups calls it rather than picking one out.
