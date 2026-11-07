/*!
    \qmltype ShareReceiver
    \brief Hands on the text and the links other apps share with this one.

    It holds the receiving end of the single share method the desktop file
    declares - \c task, which takes both \c text/plain and \c text/x-url - and
    reports what arrives through \c textShared and \c linkShared. What is worth
    making of it is the owner's business: this knows nothing of tasks, so it
    sanitises nothing and opens nothing.

    Everything is taken off the bus through a \c DBusAdaptor on
    \c /share/task, rather than a \c ShareProvider. A \c ShareProvider would
    claim the same path, and it hands on \c ShareResource objects, which carry
    only a \c name and \c data and so have nowhere to put the address and the
    page title a link is made of. One adaptor handles every shape, which is
    also what keeps a share working when the app is already open.

    The application must claim its own bus name - the organisation and
    application names of the desktop file joined by a dot - from C++ once the
    window has loaded. Claiming it from QML races with the object put on the
    bus here, which is why this does not ask for it.

    \qml
    ShareReceiver {
        onTextShared: app.openSharedTask(Askare.sharedTextFields(text))
        onLinkShared: app.openSharedTask(Askare.sharedLinkFields(url, linkTitle))
    }
    \endqml
*/
import QtQuick 2.0
import Sailfish.Share 1.0
import Nemo.DBus 2.0

Item {
    id: receiver

    signal textShared(string text)
    signal linkShared(string url, string linkTitle)

    // What the share says it is comes first. A resource read off the bus is a
    // plain map rather than a ShareResource, and carries the keys the system's
    // own share pages read - \c type, which is a mime type here, beside
    // \c status and \c linkTitle. Only a share naming no type is guessed at,
    // and then an address standing on its own is the one taken for a link.
    function takeShared(resources) {
        if (resources.length === 0)
            return ;

        var item = resources[0];
        var link = item.type ? item.type === "text/x-url" : (item.status && !item.data);
        if (link)
            receiver.takeLink(resources);
        else
            receiver.takeText(resources);

    }

    // The first link of a share, which arrives as the address the sender was
    // showing and the name it was showing it under.
    function takeLink(resources) {
        if (resources.length > 0)
            receiver.linkShared(resources[0].status || "", resources[0].linkTitle || "");

    }

    // A share carries a list, while a task is one thing written down, so the
    // first resource that says anything is the one taken. Neither kind here
    // accepts files, so a resource naming one is passed over.
    function takeText(resources) {
        for (var i = 0; i < resources.length; ++i) {
            var item = resources[i];
            var text = item.data || item.status || "";
            if (text !== "") {
                receiver.textShared(text);
                return ;
            }
        }
    }

    // Used by the adaptor below to read a share configuration into resources.
    ShareAction {
        id: sharedAction
    }

    // The path matches the method name in the desktop file. This is the only
    // object on that path: a ShareProvider beside it would own /share/task
    // instead, and a share arriving while the app was already open would
    // never reach here. One method also keeps the sharing dialog from naming
    // Askare twice over.
    DBusAdaptor {
        path: "/share/task"
        iface: "org.sailfishos.share"

        // Named for the method the share framework calls, not for what it does.
        function share(configuration) {
            sharedAction.loadConfiguration(configuration);
            receiver.takeShared(sharedAction.resources);
        }

    }

}
