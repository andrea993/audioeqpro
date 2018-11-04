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
            text: qsTr("There are more eqpro modules running, choose which module you have to control")
            font.pixelSize: 12
            Layout.margins: 10
        }

        ComboBox {
            id: comboModules
            objectName: "comboModules"
            Layout.alignment: Qt.AlignTop
            anchors.horizontalCenter: parent.horizontalCenter
            Layout.margins: 10

            function selectedMod(index) {
                return parseInt(comboModules.model[index]);

            }



        }
    }
}
