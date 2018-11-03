import QtQuick 2.7
import QtQuick.Window 2.3
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.0

Window {
    id: mainWindow
    objectName: "mainWindow"
    visible: true

    width: 640
    height: 480
    title: qsTr("EqualizerPro")

    Loader {
        id: pageloader
        objectName: "pageloader"

        source: "EqSettings.qml"
        anchors.fill: parent
    }



}
