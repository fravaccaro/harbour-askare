---
layout: default
title: Reminders
parent: Using Askare
nav_order: 4
---

# Reminders

A task with a due date can carry a reminder: at the due time, 5, 15, 30, 60 or 120 minutes before it, or a day before. The choice sits in the editor, below the due date, and is written into the task itself, so the server and its other clients see the same alarm.

Clearing the due date clears the reminder, since there is nothing left for it to count back from.

## When a reminder arrives

Askare has no background service. A reminder is announced while the app is running, whether in front or in the background; while one is still ahead, the app asks Sailfish OS to wake it every five minutes so a locked phone does not hold the notice back. An app that has been closed announces nothing, and a reminder whose moment passed while it was closed arrives when the app is opened again.

The notice carries the title of the task, when it is due, and the title of its parent when the task is a subtask. Tapping it, or its **Open** action, opens the task. Each reminder is announced once: changing the due date or the offset makes a new moment, which is announced in its turn.

A reminder sounds with the calendar alarm tone of Sailfish OS, unless the phone is in the silent profile.
