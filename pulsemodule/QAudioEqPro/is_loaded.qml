import QtQuick 2.0
import QtQuick.Dialogs 1.2

Dialog {
    title: qsTr("Warning")
    id: checkModuleIsLoaded
    visible: true

    Text {
        id: showWarning
        text: "Ensure that pulseaudio module is properly running"
    }
}
