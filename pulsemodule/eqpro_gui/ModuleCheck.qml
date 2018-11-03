import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.0


Item {
    width: 400
    height: 400

    ColumnLayout {
        anchors.fill: parent

        Text {
            Layout.alignment: Qt.AlignBottom
            anchors.horizontalCenter: parent.horizontalCenter
            text: qsTr("Pulse module eqpro seems to be not running")
            font.pixelSize: 12
            Layout.margins: 10
        }

        Button {
            objectName: "retryButton"
            Layout.alignment: Qt.AlignTop
            anchors.horizontalCenter: parent.horizontalCenter
            id: button
            text: qsTr("Retry")
            Layout.margins: 10
        }
    }
}
