import QtQuick 2.4
import QtQuick.Window 2.2

Window {
    visible: true
    width: 640
    height: 300
    title: qsTr("QAudioEqPro")

    Rectangle {
        id: mainRect
        color: "green"
        width: 640
        height: 25
        Text {
            id: welcomeText
            text: "Welcome to QAudioEqPro!"
        }
    }
}
