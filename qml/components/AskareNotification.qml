/*!
    \qmltype AskareNotification
    \brief The one transient notice the app speaks through.

    Publishing is a single call, so every passing message - a failed sync, a
    task that is no longer there - reaches the user the same way whether the
    app is in front or in the background. Nothing persists in the events view.

    No category is set: Sailfish keeps notification sounds and styling in the
    category definitions, and this package ships none.

    \qml
    AskareNotification {
        id: globalNotification
    }

    // globalNotification.toast(qsTr("The server refused the connection."))
    \endqml
*/
import QtQuick 2.0
import Nemo.Notifications 1.0

Notification {
    function toast(message) {
        summary = "";
        body = message;
        isTransient = true;
        publish();
    }

    appName: "Askare"
    appIcon: "harbour-askare"
    urgency: Notification.Low
}
