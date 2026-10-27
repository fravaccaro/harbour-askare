import Opal.SupportMe 1.0
import QtQuick 2.0

SupportDialog {
    greeting: qsTr("Hi there!")
    hook: qsTr("Thank you for using Askare. If it saves you some time, " + "maybe you can give a little back?")
    goodbye: qsTr("Thank you for your support!")

    SupportAction {
        icon: SupportIcon.Liberapay
        title: qsTr("Donate via Liberapay")
        description: qsTr("Send a tip, once or every month.")
        link: "https://liberapay.com/fravaccaro"
    }

    SupportAction {
        icon: SupportIcon.Weblate
        title: qsTr("Help with translations")
        description: qsTr("Bring Askare to your language on Transifex.")
        link: "https://app.transifex.com/fravaccaro/askare"
    }

    SupportAction {
        icon: SupportIcon.Git
        title: qsTr("Report issues on GitHub")
        description: qsTr("Tell me what went wrong, or what is missing.")
        link: "https://github.com/fravaccaro/harbour-askare/issues"
    }

    DetailsDrawer {
        title: qsTr("Why support this app?")

        DetailsParagraph {
            text: qsTr("Askare is free software under the GPLv3, written in spare time " + "next to other projects.")
        }

        DetailsParagraph {
            text: qsTr("Support keeps it working with new Nextcloud releases and new " + "versions of Sailfish OS.")
        }

    }

    DetailsDrawer {
        title: qsTr("Other ways to help")

        DetailsParagraph {
            text: qsTr("Report what breaks, suggest what is missing, or tell other " + "Nextcloud users about the app.")
        }

    }

}
