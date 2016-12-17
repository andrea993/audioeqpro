import QtQuick 2.0
import QtQuick.Dialogs 1.2

Dialog {
    visible: true
    id: failDialog
    title: qsTr("Failed to load pulseaudio module")
    onClickedButtonChanged: Qt.quit()

    Text {
        id: pulseModuleFailed
        text: "Failed to load pulseaudio module."
    }
}
