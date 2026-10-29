---
layout: default
title: Viewing and editing
parent: Using Askare
nav_order: 3
---

# Viewing and editing

## The task

Opening a task shows its priority, its status, when it starts and when it is due, the reminder it carries, where it is and the web address it links to, the tags it wears, how far along it is, the notes, and its subtasks with a count of the ones already done. An empty field still shows, reading **None**, so the task reads the same whether or not it carries one. The web address is a link that opens in the browser. Tapping a subtask opens it like any other task, with the same fields again however deep it sits; press and hold offers **Edit**, **Mark done** and **Delete**, as a row of the task list does.

The pulley menu holds **Edit**, **Mark done**, **Add subtask** and **Delete**.

## Editing

The editor takes:

* **List** - for a task that stands on its own, whether it is being created or edited; choosing another list moves the task there
* **Name**
* **Completion** - from 0 to 100 percent in steps of 5; 100 marks the task done
* **Status** - No status, Needs action, In process, Completed or Cancelled; it moves with Completion, so marking a task done sets it Completed and dropping the bar below full sends a completed task back to in process. **No status** clears it, leaving the task without a state at all, the way a task created on the Nextcloud side starts out
* **Priority** - 1 to 4 high, 5 medium, 6 to 9 low, 0 none
* **Start** and **Due** - a date each, with a time or left as all day
* **Reminder** - see [Reminders](reminders); a due date is needed first
* **Location** - a place in plain words
* **Web address** - a link, opened in the browser from the task view
* **Tags** - see below
* **Notes**

A task may not start after it is due; the editor says so and refuses to save until one of the two dates moves. **Clear start date** and **Clear due date** remove a date again, and clearing the due date clears the reminder with it.

## Tags

A task carries as many tags as you like. The editor shows them as chips: tap one to take it off, or write a name in **New tag** and confirm to add it. Under the field stand the tags already used on other tasks that this one does not wear yet, and a tap adds one, so the same spelling comes back instead of a near miss. A name is kept once however it is capitalised, and a blank one is ignored.

Tags are the **categories** of the task on the server, so they are the same tags the Nextcloud web interface shows, and a tag written there arrives on the next sync. The Lists page gathers them; see [Lists and tasks](lists).

## Notes in Markdown

Notes are written as plain text and shown with their Markdown applied: headings, bulleted and numbered lists, block quotes, horizontal rules, links, `code`, **bold** and *italic*. A link in the notes opens in the browser. The text itself stays Markdown, so the notes read the same way in the Nextcloud web interface.

## Subtasks

**Add subtask** creates a task under the one being read, in the list of its parent, because a server resolves the relation between the two by an identifier that is unique only within one list. The task list counts the subtasks of a task on its row, and the Current and Completed views show subtasks in their own right rather than behind their parent.

Deleting a task deletes its subtasks with it; the confirmation says how many tasks that is.

Moving a task to another list takes its subtasks along. Moving a subtask to another list, on the other hand, turns it into a task of its own: it can no longer point at the parent it leaves behind, for the same reason a subtask stays in its parent's list to begin with. Its own subtasks, if it has any, follow it across and stay under it. The editor says so before the move is saved. A move is sent to the server on the next sync, so a task moved while offline shows in its new list on the device until then.
