import QtQuick 2.0
import QtQuick.Dialogs 1.2

Dialog {
    visible: true
    id: failDialog
    onClickedButtonChanged: Qt.quit()
    Text {
        text: "Failed to load pulseaudio module."
    }
}
