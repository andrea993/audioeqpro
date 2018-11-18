import QtQuick 2.7
import QtQuick.Window 2.3
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.0
import Qt.labs.platform 1.0

Window {
    id: mainWindow
    objectName: "mainWindow"
    visible: true

    width: 640
    height: 480
    minimumWidth: 640
    minimumHeight: 300
    title: qsTr("EqualizerPro")

    Loader {
        id: pageloader
        objectName: "pageloader"

        source: "EqSettings.qml"
        anchors.fill: parent
    }

    SystemTrayIcon {
        visible: true
        iconSource: "file://usr/share/pixmaps/eqpro-icon.png"

        menu: Menu {
                MenuItem {
                    text: qsTr("Quit")
                    onTriggered: Qt.quit()
                }
            }
        onActivated: {
            mainWindow.show()
            mainWindow.raise()
            mainWindow.requestActivate()
        }
    }
}
