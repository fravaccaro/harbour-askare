---
layout: default
title: Viewing and editing
parent: Using Askare
nav_order: 3
---

# Viewing and editing

## The task

Opening a task shows its priority, when it starts and when it is due, the reminder it carries, how far along it is, the notes, and its subtasks with a count of the ones already done. Tapping a subtask opens it like any other task; press and hold offers **Edit**, **Mark done** and **Delete**, as a row of the task list does.

The pulley menu holds **Edit**, **Mark done**, **Add subtask** and **Delete**.

## Editing

The editor takes:

* **List** - only while a task is being created, and only when it stands on its own
* **Name**
* **Completion** - from 0 to 100 percent in steps of 5; 100 marks the task done
* **Priority** - 1 to 4 high, 5 medium, 6 to 9 low, 0 none
* **Start** and **Due** - a date each, with a time or left as all day
* **Reminder** - see [Reminders](reminders); a due date is needed first
* **Notes**

A task may not start after it is due; the editor says so and refuses to save until one of the two dates moves. **Clear start date** and **Clear due date** remove a date again, and clearing the due date clears the reminder with it.

## Notes in Markdown

Notes are written as plain text and shown with their Markdown applied: headings, bulleted and numbered lists, block quotes, horizontal rules, links, `code`, **bold** and *italic*. A link in the notes opens in the browser. The text itself stays Markdown, so the notes read the same way in the Nextcloud web interface.

## Subtasks

**Add subtask** creates a task under the one being read. A subtask stays in the list of its parent, because a server resolves the relation between the two by an identifier that is unique only within one list. The task list counts the subtasks of a task on its row, and the Current and Completed views show subtasks in their own right rather than behind their parent.

Deleting a task deletes its subtasks with it; the confirmation says how many tasks that is.
