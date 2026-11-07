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

    // The three things that can be waiting on the app being ready: a task a
    // reminder asked to open, a new task asked for from the cover, and the
    // fields of a share. A share always names the task it makes, so a summary
    // is what says one is still waiting.
    property string pendingTaskHref: ""
    property bool newTaskWanted: false
    property var pendingShare: ({})
    readonly property bool shareWanted: app.pendingShare.summary !== undefined
    // Sailjail grants the app the name its desktop file gives it, so the
    // service is the OrganizationName and ApplicationName of that file. A bus
    // name takes a hyphen where a path and an interface name do not.
    readonly property string dbusService: "org.frapps.harbour-askare"
    readonly property string dbusPath: "/org/frapps/harbour_askare"
    readonly property string dbusIface: "org.frapps.harbour_askare"

    function showMain() {
        if (pageStack.busy) {
            navigateAgain.restart();
            return ;
        }
        pageStack.replaceAbove(null, Askare.configured ? tasksPage : loginPage);
        // Something may have been waiting on the start page: try once the
        // stack has been pointed at the tasks or the sign-in. Each of them
        // still waits while the replace is busy, so nothing is pushed under
        // a wipe.
        app.takePending();
    }

    // Every passing message goes through one notice, so the app speaks the
    // same way whether it is in front or in the background.
    function showToast(text) {
        if (text !== "")
            globalNotification.toast(text);

    }

    // Asked for from a reminder notice, which is tapped while the app is in
    // the background or behind another page, so the window is raised first.
    function openTask(href) {
        if (href === "")
            return ;

        app.pendingTaskHref = href;
        activate();
        app.takePendingTask();
    }

    // A notice tapped while the app was closed starts it, and the href arrives
    // before there is a store to look the task up in, so it waits for one.
    function takePendingTask() {
        if (app.pendingTaskHref === "" || !Askare.ready || !Askare.configured)
            return ;

        if (pageStack.busy) {
            openTaskAgain.restart();
            return ;
        }
        var href = app.pendingTaskHref;
        app.pendingTaskHref = "";
        app.pushTask(href);
    }

    // Asked for from the cover, which can be tapped before the app has a page
    // to put in front of anyone, so the request waits for the stack to settle.
    function openNewTask() {
        app.newTaskWanted = true;
        activate();
        app.takePendingNewTask();
    }

    // A list is what a new task is written into, so there is no editor worth
    // opening until the lists have been read.
    function takePendingNewTask() {
        if (!app.newTaskWanted || !Askare.ready || !Askare.configured || Calendars.count === 0)
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

    // A share arrives from outside, so it may find the app signed out, or on a
    // cold start still deciding what page it opens on. It waits here instead
    // of being dropped, and the latest one is the one that opens.
    function openSharedTask(fields) {
        if (!fields.summary) {
            app.showToast(qsTr("There was nothing in that to write down."));
            return ;
        }
        app.pendingShare = fields;
        activate();
        app.takePendingShare();
    }

    // Pages whose next navigation would throw an editor away: the start page
    // on a cold start, and the sign-in pages before the tasks replace them.
    function shareBlocked() {
        var page = pageStack.currentPage;
        if (!page)
            return true;

        var name = page.objectName;
        return name === "startPage" || name === "loginPage" || name === "webLoginPage";
    }

    // Held back until the app knows where it is going: until the settings have
    // been read there is no telling whether there is an account to write to,
    // and an editor pushed over the start or sign-in pages is thrown away by
    // the navigation that follows them. pendingShare is kept until the push
    // lands, so a wipe cannot drop the only copy of what was shared.
    function takePendingShare() {
        if (!app.shareWanted || !Askare.ready)
            return ;

        if (!Askare.configured) {
            app.showToast(qsTr("Sign in, and what you shared opens as a new task."));
            return ;
        }
        // A task is written into a list, and an editor with no list to offer
        // cannot be accepted at all, so a share waits for the lists the way a
        // new task asked for from the cover does.
        if (Calendars.count === 0)
            return ;

        if (pageStack.busy || app.shareBlocked()) {
            shareAgain.restart();
            return ;
        }
        app.pushSharedTask();
    }

    // The share is let go of as the editor takes it, so a task saved and a
    // task abandoned both leave nothing behind to open a second time. It gets
    // an editor of its own even where one is already open, which a new task
    // asked for from the cover does not: raising that one would drop what was
    // shared.
    function pushSharedTask() {
        var fields = app.pendingShare;
        app.pendingShare = {};
        fields.calendarHref = Tasks.calendarHref;
        pageStack.push(Qt.resolvedUrl("pages/TaskEditPage.qml"), fields);
    }

    // What waits on the app being ready waits on the same thing, so one call
    // picks up whichever of the three is owed.
    function takePending() {
        app.takePendingTask();
        app.takePendingNewTask();
        app.takePendingShare();
    }

    function pushTask(href) {
        var fields = Askare.taskFields(href);
        if (!fields.href) {
            app.showToast(qsTr("That task is no longer here."));
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
            app.showToast(qsTr("That task is no longer here."));
            return ;
        }
        pageStack.push(Qt.resolvedUrl("pages/TaskEditPage.qml"), fields);
    }

    function reminderBody(dueText, parentSummary) {
        var due = dueText === "" ? qsTr("Task reminder") : qsTr("Due %1").arg(dueText);
        return parentSummary === "" ? due : parentSummary + " - " + due;
    }

    // One notice serves every reminder: publish() asks for a fresh id each
    // time it is given none, so a second reminder stands beside the first
    // rather than replacing it, and nothing here is needed once it is out.
    function publishReminder(href, summary, dueText, parentSummary) {
        reminderNotice.replacesId = 0;
        reminderNotice.summary = summary;
        reminderNotice.body = app.reminderBody(dueText, parentSummary);
        reminderNotice.previewSummary = reminderNotice.summary;
        reminderNotice.previewBody = reminderNotice.body;
        reminderNotice.remoteActions = app.reminderActions(href);
        reminderNotice.publish();
        app.playReminderSound();
    }

    function playReminderSound() {
        profile.call("get_profile", [], function(name) {
            if (name !== "silent")
                reminderSound.play();

        });
    }

    // A notice outlives the process that published it, and an action signal
    // only ever reaches the object that holds its id. So the tap is a D-Bus
    // call the notification manager places itself, which arrives at whichever
    // instance of the app answers to the name - a later one included.
    function reminderAction(name, displayName, icon, method, href) {
        return {
            "name": name,
            "displayName": displayName,
            "icon": icon,
            "service": app.dbusService,
            "path": app.dbusPath,
            "iface": app.dbusIface,
            "method": method,
            "arguments": [href]
        };
    }

    // "default" is the notice itself being tapped, rather than one of the
    // actions drawn under it.
    function reminderActions(href) {
        return [app.reminderAction("default", "", "", "openTask", href), app.reminderAction("open", qsTr("Open"), "icon-s-task", "openTask", href), app.reminderAction("done", qsTr("Done"), "icon-s-installed", "markTaskDone", href)];
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
        onTriggered: app.takePendingTask()
    }

    Timer {
        id: shareAgain

        interval: 100
        onTriggered: app.takePendingShare()
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
            if (Askare.configured)
                app.takePending();
            else
                app.showMain();
        }
        onErrorMessageChanged: app.showToast(Askare.errorMessage)
        onBusyChanged: {
            if (!Askare.busy)
                Reminders.check();

        }
    }

    // A task needs a list, so the first sync of a new account is what lets a
    // share, or a new task asked for from the cover, open at all.
    Connections {
        target: Calendars
        onCountChanged: app.takePending()
    }

    AskareNotification {
        id: globalNotification
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

    Notification {
        id: reminderNotice

        appName: "Askare"
        appIcon: "harbour-askare"
        isTransient: false
        urgency: Notification.Normal
    }

    // Where a tapped reminder arrives. The notification manager closes the
    // notice itself once an action is invoked, so neither of these does.
    //
    // The bus name is not asked for here: main.cpp claims it once the window
    // has loaded, so that this object and the share receiver's are both on the
    // bus before anything can call either of them.
    DBusAdaptor {
        function openTask(href) {
            app.openTask(href);
        }

        function markTaskDone(href) {
            Askare.markDone(href);
        }

        path: app.dbusPath
        iface: app.dbusIface
        xml: '  <interface name="' + app.dbusIface + '">\n' + '    <method name="openTask">\n' + '      <arg name="href" type="s" direction="in"/>\n' + '    </method>\n' + '    <method name="markTaskDone">\n' + '      <arg name="href" type="s" direction="in"/>\n' + '    </method>\n' + '  </interface>\n'
    }

    Connections {
        target: Reminders
        onDue: app.publishReminder(href, summary, dueText, parentSummary)
    }

    // Text and links shared from other apps. They are shaped into the fields
    // of a task by the session and opened in the editor, never saved on their
    // own: what another app sent is the writer's to look at before it is kept.
    ShareReceiver {
        onTextShared: app.openSharedTask(Askare.sharedTextFields(text))
        onLinkShared: app.openSharedTask(Askare.sharedLinkFields(url, linkTitle))
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
