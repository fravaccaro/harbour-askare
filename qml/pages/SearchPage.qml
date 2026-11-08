import QtQuick 2.0
import Sailfish.Silica 1.0

Page {
    id: page

    function clearQuery() {
        Tasks.search = "";
        Tasks.filters = {
        };
        drawer.open = false;
    }

    function openFilters() {
        searchField.focus = false;
        drawer.open = true;
    }

    // The search becomes a view of its own, words and filters together. The set
    // is read before the dialog opens, so what is saved is what was on screen.
    function saveSmartList() {
        searchField.focus = false;
        var set = Tasks.searchSet();
        var dialog = pageStack.push(Qt.resolvedUrl("SmartListDialog.qml"), {
            "suggestion": Tasks.search
        });
        dialog.accepted.connect(function() {
            Askare.saveSmartList(dialog.name, set);
        });
    }

    allowedOrientations: Orientation.All
    // A page pushed on top deactivates this one too. Back is what says the
    // search itself is leaving, so the list underneath is whole again before
    // it shows. A task opened from a result leaves the query in place.
    onStatusChanged: {
        if (status === PageStatus.Deactivating && page._navigation === PageNavigation.Back)
            page.clearQuery();

    }
    Component.onDestruction: page.clearQuery()

    Drawer {
        id: drawer

        anchors.fill: parent
        // The filter form docks like the component gallery's panel: across the
        // top held upright, down the side when the phone is turned.
        dock: page.isPortrait ? Dock.Top : Dock.Left

        Item {
            anchors.fill: parent

            // Loader first, column second — same order as the gallery. The
            // Loader defers the list until the column has its final height, so
            // a page opened on an already-filled model still shows the title.
            Loader {
                anchors.fill: parent
                sourceComponent: listViewComponent
            }

            MouseArea {
                anchors.fill: parent
                enabled: drawer.open
                onClicked: drawer.open = false
            }

        }

        background: SearchFilters {
            anchors.fill: parent
        }

    }

    Column {
        id: headerContainer

        width: page.width

        PageHeader {
            title: qsTr("Search")
        }

        SearchField {
            id: searchField

            width: parent.width
            placeholderText: qsTr("Search tasks")
            inputMethodHints: Qt.ImhNoAutoUppercase | Qt.ImhNoPredictiveText
            EnterKey.iconSource: "image://theme/icon-m-enter-close"
            EnterKey.onClicked: focus = false
            onTextChanged: Tasks.search = text
        }

        BackgroundItem {
            width: parent.width
            onClicked: page.openFilters()

            Label {
                x: Theme.horizontalPageMargin
                anchors.verticalCenter: parent.verticalCenter
                color: Tasks.filtersActive ? Theme.highlightColor : Theme.primaryColor
                text: Tasks.filtersActive ? qsTr("Filters (%n)", "", Tasks.filterCount) : qsTr("Filters")
            }

        }

    }

    Component {
        id: listViewComponent

        SilicaListView {
            id: listView

            model: Tasks
            // Nothing here is picked out. A current row would take the focus
            // the field just gained, and every keystroke rebuilds the model.
            currentIndex: -1
            section.property: "group"
            // While the drawer is out, a tap on a row closes it rather than
            // opening the task behind it.
            enabled: !drawer.opened
            // The gallery fills its model after the view exists. Ours is already
            // full, so pin content to origin once the header height is known.
            Component.onCompleted: contentY = originY

            PullDownMenu {
                MenuItem {
                    // Shown always, so the way to keep a search is never
                    // hidden; it only waits for something to save.
                    enabled: Tasks.filtersActive || Tasks.search !== ""
                    text: qsTr("Save as smart list")
                    onClicked: page.saveSmartList()
                }

            }

            ViewPlaceholder {
                enabled: listView.count === 0 && !Askare.busy
                // Silica puts the text a third of the way down the screen,
                // which a header carrying the field and the filters as well as
                // the title reaches all the way to. Two thirds of that header
                // moves it to a third of the way down what is left of the view.
                verticalOffset: headerContainer.height * 2 / 3
                text: Tasks.search !== "" || Tasks.filtersActive ? qsTr("No matches") : qsTr("No tasks")
                hintText: Tasks.search !== "" || Tasks.filtersActive ? qsTr("Try another search") : ""
            }

            VerticalScrollDecorator {
            }

            // Empty slot sized to the already-laid-out column; the column moves
            // in on completion so the pulley still opens from the page top.
            header: Item {
                id: headerSlot

                width: headerContainer.width
                height: headerContainer.height
                Component.onCompleted: headerContainer.parent = headerSlot
            }

            section.delegate: SectionHeader {
                text: section === "cancelled" ? qsTr("Cancelled") : (section === "completed" ? qsTr("Completed") : qsTr("To do"))
            }

            delegate: TaskListDelegate {
            }

        }

    }

}
