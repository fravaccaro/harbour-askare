---
layout: default
title: Sync, files and permissions
parent: Using Askare
nav_order: 5
---

# Sync, files and permissions

## Syncing

A sync reads every task list of the server, merges what it finds into the copy on the device, and uploads whatever was changed here. **Sync** sits in the pulley menu of the task list and of the Lists page, and on the cover.

Three settings decide when a sync happens on its own:

| Setting | Default | What it does |
|---|---|---|
| Sync when the app starts | On | One sync as Askare opens |
| Automatic sync | Off | Every 5, 15, 30 or 60 minutes, only while Askare is running |
| Sync after saving a task | On | Fetches the server's changes once a saved task has been uploaded |

Changes made on the device are kept and uploaded on the next sync, so tasks can be written while the server is out of reach. A task the server keeps rejecting stays marked as changed and is offered again on each run; the reason the server gave is shown above the task list and on the settings page.

## Where the data lives

Sailjail, the Sailfish OS sandbox, gives the app three private directories named after the organisation and application of its desktop file:

* `~/.config/org.frapps/harbour-askare` - `settings.conf`, holding the server address, the user name, and the preferences above
* `~/.local/share/org.frapps/harbour-askare` - `askare.db`, the copy of the lists and tasks
* `~/.cache/org.frapps/harbour-askare` - whatever the web view leaves behind while signing in; Askare writes nothing of its own there

The app password is not in any of them; it lives in Sailfish Secrets. Signing out empties the copy of the lists and removes the password.

## Permissions

| Permission | Why |
|---|---|
| `Internet` | Talking to the Nextcloud server over CalDAV |
| `Secrets` | The app password in Sailfish Secrets |
| `WebView` | Signing in through the server's own page |

Askare reads no documents, pictures or memory cards, and does not open the calendar of the device.

## Backup

The settings and the local copy of the tasks are declared for the Sailfish OS backup through `X-HarbourBackup`. The app password lives in Sailfish Secrets and is not part of a backup; sign in again after a restore.
