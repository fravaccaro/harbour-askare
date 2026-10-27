import Nemo.DBus 2.0
import Nemo.KeepAlive 1.2
import Nemo.Notifications 1.0
import Opal.SupportMe 1.0
import QtMultimedia 5.6
import QtQuick 2.0
import Sailfish.Silica 1.0
import "components"
import "pages"

ApplicationWindow {
    id: app

    // Tracks live reminder notices so Open keeps reaching onClicked, and the
    // href a pending openTask() is waiting on while the stack settles.
    property var reminderNotices: []
    property string pendingTaskHref: ""
    property bool newTaskWanted: false

    function showMain() {
        if (pageStack.busy) {
            navigateAgain.restart();
            return ;
        }
        pageStack.replaceAbove(null, Askare.configured ? tasksPage : loginPage);
    }

    // A passing message: shown inside the window when the app is in front,
    // otherwise a banner that is gone once it has been read.
    function notify(text) {
        if (text === "")
            return ;

        if (app.applicationActive) {
            Notices.show(text, Notice.Short);
            return ;
        }
        backgroundNotice.previewBody = text;
        backgroundNotice.publish();
    }

    function openTask(href) {
        if (href === "")
            return ;

        if (pageStack.busy) {
            app.pendingTaskHref = href;
            openTaskAgain.restart();
            return ;
        }
        app.pendingTaskHref = "";
        if (!Askare.configured) {
            app.showMain();
            return ;
        }
        app.pushTask(href);
    }

    // Asked for from the cover, which can be tapped before the app has a page
    // to put in front of anyone, so the request waits for the stack to settle.
    function openNewTask() {
        if (!Askare.configured || Calendars.count === 0)
            return ;

        activate();
        app.newTaskWanted = true;
        app.takePendingNewTask();
    }

    function takePendingNewTask() {
        if (!app.newTaskWanted || !Askare.ready || !Askare.configured)
            return ;

        if (pageStack.busy) {
            newTaskAgain.restart();
            return ;
        }
        app.newTaskWanted = false;
        // An editor left open from an earlier tap is raised rather than buried
        // under a second copy of itself.
        if (pageStack.find(function(item) {
            return item.objectName === "taskEditPage";
        }))
            return ;

        pageStack.push(Qt.resolvedUrl("pages/TaskEditPage.qml"), {
            "calendarHref": Tasks.calendarHref
        });
    }

    function pushTask(href) {
        var fields = Askare.taskFields(href);
        if (!fields.href) {
            app.notify(qsTr("That task is no longer here."));
            return ;
        }
        var current = pageStack.currentPage;
        if (current && current.href === href)
            return ;

        pageStack.push(Qt.resolvedUrl("pages/TaskDetailPage.qml"), fields);
    }

    // Opens the editor for an existing task from its href alone, so a list
    // row's context menu can skip the detail page.
    function editTask(href) {
        var fields = Askare.taskFields(href);
        if (!fields.href) {
            app.notify(qsTr("That task is no longer here."));
            return ;
        }
        pageStack.push(Qt.resolvedUrl("pages/TaskEditPage.qml"), fields);
    }

    function reminderBody(dueText, parentSummary) {
        var due = dueText === "" ? qsTr("Task reminder") : qsTr("Due %1").arg(dueText);
        return parentSummary === "" ? due : parentSummary + " - " + due;
    }

    function publishReminder(href, summary, dueText, parentSummary) {
        var notice = reminderNoticeComponent.createObject(app, {
            "href": href
        });
        if (notice === null)
            return ;

        notice.summary = summary;
        notice.body = app.reminderBody(dueText, parentSummary);
        notice.previewSummary = notice.summary;
        notice.previewBody = notice.body;
        notice.remoteActions = app.reminderActions();
        notice.publish();
        app.playReminderSound();
        app.reminderNotices.push(notice);
    }

    function playReminderSound() {
        profile.call("get_profile", [], function(name) {
            if (name !== "silent")
                reminderSound.play();

        });
    }

    function reminderActions() {
        return [{
            "name": "default"
        }, {
            "name": "open",
            "displayName": qsTr("Open"),
            "icon": "icon-s-task"
        }, {
            "name": "done",
            "displayName": qsTr("Mark done"),
            "icon": "icon-s-installed"
        }];
    }

    function handleReminderAction(name, notice) {
        if (name === "done") {
            Askare.markDone(notice.href);
            notice.close();
            return ;
        }
        app.openTask(notice.href);
    }

    function forgetReminder(notice) {
        var kept = [];
        for (var i = 0; i < app.reminderNotices.length; ++i) {
            if (app.reminderNotices[i] !== notice)
                kept.push(app.reminderNotices[i]);

        }
        app.reminderNotices = kept;
    }

    initialPage: Askare.ready ? (Askare.configured ? tasksPage : loginPage) : startPage
    cover: Qt.resolvedUrl("cover/CoverPage.qml")
    allowedOrientations: defaultAllowedOrientations

    Timer {
        id: navigateAgain

        interval: 100
        onTriggered: app.showMain()
    }

    Timer {
        id: openTaskAgain

        interval: 100
        onTriggered: app.openTask(app.pendingTaskHref)
    }

    Timer {
        id: newTaskAgain

        interval: 100
        onTriggered: app.takePendingNewTask()
    }

    Component {
        id: startPage

        StartPage {
        }

    }

    Component {
        id: loginPage

        LoginPage {
        }

    }

    Component {
        id: tasksPage

        TasksPage {
        }

    }

    Connections {
        target: Askare
        onReadyChanged: {
            if (Askare.ready)
                app.showMain();

        }
        onConfiguredChanged: {
            if (!Askare.configured)
                app.showMain();

        }
        onErrorMessageChanged: app.notify(Askare.errorMessage)
        onBusyChanged: {
            if (!Askare.busy)
                Reminders.check();

        }
    }

    Notification {
        id: backgroundNotice

        appName: "Askare"
        appIcon: "harbour-askare"
        isTransient: true
        urgency: Notification.Low
        previewSummary: qsTr("Askare")
    }

    // Sailfish keeps notification sounds in the category definitions, and has
    // none for a reminder, so the tone is played here instead.
    Audio {
        id: reminderSound

        source: "file:///usr/share/sounds/jolla-ringtones/stereo/jolla-calendar-alarm.ogg"
        audioRole: Audio.NotificationRole
    }

    // Asked afresh for each reminder, so a silenced phone stays silent without
    // the window having to follow every profile change.
    DBusInterface {
        id: profile

        service: "com.nokia.profiled"
        path: "/com/nokia/profiled"
        iface: "com.nokia.profiled"
    }

    Component {
        id: reminderNoticeComponent

        Notification {
            id: notice

            property string href

            appName: "Askare"
            appIcon: "harbour-askare"
            isTransient: false
            urgency: Notification.Normal
            onClicked: app.openTask(notice.href)
            onActionInvoked: app.handleReminderAction(name, notice)
            onClosed: app.forgetReminder(notice)
        }

    }

    Connections {
        target: Reminders
        onDue: app.publishReminder(href, summary, dueText, parentSummary)
    }

    // The timer inside Reminders only ticks while the device is awake, so a
    // locked phone would hold a reminder back until something woke it.
    BackgroundJob {
        id: wakeUp

        enabled: Reminders.armed
        frequency: BackgroundJob.FiveMinutes
        onTriggered: {
            Reminders.check();
            wakeUp.finished();
        }
    }

    // Asks for support once the app has been used for a while, and not again
    // for a long time after the answer.
    AskForSupport {

        contents: Component {
            AskareSupportDialog {
            }

        }

    }

}
