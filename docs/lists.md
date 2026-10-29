---
layout: default
title: Lists and tasks
parent: Using Askare
nav_order: 2
---

# Lists and tasks

Askare opens on a list of tasks. Swipe left from it to reach the **Lists** page, which chooses what that list holds.

## The views

Above the task lists of the server sit three views that read across all of them:

* **All** - every task, the completed ones after the open ones and the cancelled ones last
* **Current** - what is open and either due today or earlier, or carries no due date at all
* **Important** - tasks with high priority
* **Completed** - everything no longer open: finished tasks, and cancelled ones in a section of their own

## Tags

Under the views stands a **Tags** section, one row per tag in use, with the number of tasks still open under it. The section only appears once something carries a tag. Choosing a tag shows every task wearing it, from any list and at any depth, so a tagged subtask stands beside the rest rather than behind its parent. The view holds open and finished tasks alike, and answers the two settings below just as All does.

Tags are kept on the task itself, as its categories on the server, so they match what the Nextcloud web interface shows. See [Viewing and editing](editing) for putting them on a task.

Below them stand the task lists the server keeps, each with the colour you gave it in Nextcloud and the number of tasks still open in it. Choosing one narrows the task list to it.

All and the view of one list count a subtask behind its parent; Current and Completed lay every task out flat, so a subtask due today is not hidden under a parent that is not.

**Show completed tasks**, on the settings page, decides whether All and the view of one list keep finished tasks in sight. It is on to begin with. Turning it off leaves only open tasks there; the Completed view still shows them all.

## Cancelled tasks

A task whose status is **Cancelled** asks for nothing further, so it counts as neither open nor done. It stays out of Current, Important and the **To do** group, and the counts beside All and beside each list pass over it. The **Completed** view always keeps it, though, in a **Cancelled** section below the finished tasks, so cancelled work is never lost from sight.

**Show cancelled tasks**, on the settings page, decides whether All and the view of one list show them as well. It is off to begin with, so a cancelled task shows only under Completed. Turning it on gathers them into a **Cancelled** section at the foot of those lists too.

Wherever it appears, a cancelled task reads as struck through and muted. That is what tells it apart from a finished one, which is struck through but keeps its usual colour and a ticked box. A cancelled row carries no done box, and press and hold offers only **Edit** and **Delete**, because a cancelled task is not work waiting to be finished. It opens like any other, and **Edit** is where its status goes back to something open.

## The task list

Tasks are grouped under **To do**, **Completed** and, where they are shown, **Cancelled**. Within a group, dated tasks come first in order of their date and undated ones trail. A row shows the title, the due date and time if the task has one, and how many of its subtasks are done. In the views that mix lists, a coloured bar on the left says which list a task belongs to.

Tap the switch on a row to mark a task done or open again. Tap the rest of the row to open the task. Press and hold for **Mark done** and **Delete**; deleting a task with subtasks says how many tasks will go.

The pulley menu offers **Refresh** and **New task**. A new task started here lands in the list currently shown, or can be moved to another one in the editor.

On the Lists page the pulley menu leads to **About**, **Settings** and **Refresh**. About shows the version, donations, translations, and the licence.

## The cover

The cover shows the name of the current view and the first of its tasks, struck through once they are done. Its two actions add a task and sync. When nobody is signed in it says so instead.
