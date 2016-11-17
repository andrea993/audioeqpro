import QtQuick 2.0
import QtQuick.Dialogs 1.2

Dialog {
    visible: true
    title: qsTr("Loading pulseaudio module...");
    Text {
        id: dialogLoadingAlert
        text: "Please wait while connecting to Pulse and loading the module..."
    }
}
