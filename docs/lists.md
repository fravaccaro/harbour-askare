---
layout: default
title: Lists and tasks
parent: Using Askare
nav_order: 2
---

# Lists and tasks

Askare opens on a list of tasks. Swipe left from it to reach the **Lists** page, which chooses what that list holds.

## The views

Above the task lists of the server sit views that read across all of them:

* **All** - every task, the completed ones after the open ones and the cancelled ones last
* **Current** - what is open and either due today or earlier, or carries no due date at all
* **Today** - what is still open and due today. A task due earlier stays in Current, and one with no due date does too
* **Important** - tasks with high priority
* **Completed** - everything no longer open: finished tasks, and cancelled ones in a section of their own

## Smart lists

A set of filters worth returning to can be kept by name. With at least one filter on, **Save as smart list** in the search page's pulley menu asks for a name and files the filters under it. Only the filters are saved, never the words typed in the field, so the list reads the same whatever was being searched for at the time.

Saved sets stand in a **Smart lists** section of their own on the Lists page, between the views above and the task lists of the server, each with the number of tasks it keeps. Choosing one shows what the search showed: every list, at every depth, finished and cancelled tasks included, narrowed by the filters it holds. The section only appears once something has been saved.

Press and hold a smart list for **Delete**, which waits behind a remorse timer. Deleting the one on show leaves the task list on All. Smart lists are kept on the device rather than on the server, so they are yours alone and they outlive a sign-out.

## Tags

Last on the page stands a **Tags** section, one row per tag in use, with the number of tasks still open under it. The section only appears once something carries a tag. Choosing a tag shows every task wearing it, from any list and at any depth, so a tagged subtask stands beside the rest rather than behind its parent. The view holds open and finished tasks alike, and answers the two settings below just as All does.

Tags are kept on the task itself, as its categories on the server, so they match what the Nextcloud web interface shows. See [Viewing and editing](editing) for putting them on a task.

Above them stand the task lists the server keeps, each with the colour you gave it in Nextcloud and the number of tasks still open in it. Choosing one narrows the task list to it.

All and the view of one list count a subtask behind its parent; Current, Today and Completed lay every task out flat, so a subtask due today is not hidden under a parent that is not.

**Show completed tasks**, on the settings page, decides whether All and the view of one list keep finished tasks in sight. It is on to begin with. Turning it off leaves only open tasks there; the Completed view still shows them all.

## Cancelled tasks

A task whose status is **Cancelled** asks for nothing further, so it counts as neither open nor done. It stays out of Current, Today, Important and the **To do** group, and the counts beside All and beside each list pass over it. The **Completed** view always keeps it, though, in a **Cancelled** section below the finished tasks, so cancelled work is never lost from sight.

**Show cancelled tasks**, on the settings page, decides whether All and the view of one list show them as well. It is off to begin with, so a cancelled task shows only under Completed. Turning it on gathers them into a **Cancelled** section at the foot of those lists too.

Wherever it appears, a cancelled task reads as struck through and muted. That is what tells it apart from a finished one, which is struck through but keeps its usual colour and a ticked box. A cancelled row carries no done box, and press and hold offers only **Edit** and **Delete**, because a cancelled task is not work waiting to be finished. It opens like any other, and **Edit** is where its status goes back to something open.

## The task list

Tasks are grouped under **To do**, **Completed** and, where they are shown, **Cancelled**. Within a group, dated tasks come first in order of their date and undated ones trail. A row shows the title, the due date and time if the task has one, and how many of its subtasks are done. In the views that mix lists, a coloured bar on the left says which list a task belongs to.

Tap the switch on a row to mark a task done or open again. Tap the rest of the row to open the task. Press and hold for **Mark done** and **Delete**; deleting a task with subtasks says how many tasks will go.

The pulley menu offers **Refresh**, **Search** and **New task**. A new task started here lands in the list currently shown, or can be moved to another one in the editor.

## Searching

**Search** in the pulley menu opens its own page, with the field ready to type in. Typing looks through every task in every list, including subtasks and tasks that are finished or cancelled, matched without regard to case against the title, the notes, the location and the tags. A matching subtask shows on its own. The view you came from plays no part. When nothing matches, the page says so. Going back clears the search, and the list is as it was. A task opened from a result keeps the search underneath, so coming back to it finds the same matches.

### Filters

**Filters** under the search field slides out a panel of filters. The line turns highlighted and counts them, **Filters (2)**, once any filter is on. Each one is optional, and the results narrow as soon as a filter is set, even with the field empty. Where a filter offers several choices, a task matches any one of them; across filters, every filter that is set must match.

- **Status, List, Priority** and **Tags** each open a list of toggles, so more than one choice can be on at once.
- **Completion** sets a percentage, then **At least** or **At most** keeps tasks at or above, or at or below, that mark.
- **Tasks due today** keeps tasks due today, finished and cancelled ones included.
- **Start date** and **Due date** are set apart from each other, each matching on, before or after the day you pick.
- **Reminder set** keeps tasks that do, or do not, carry a reminder.

**Clear filters** empties the panel. Leaving the search page clears the filters along with the text; a set worth keeping is saved as a [smart list](#smart-lists) first.

On the Lists page the pulley menu leads to **About**, **Settings** and **Refresh**. About shows the version, donations, translations, and the licence.

## The cover

The cover shows the name of the current view and the first of its tasks, struck through once they are done. Its two actions add a task and sync. When nobody is signed in it says so instead.
