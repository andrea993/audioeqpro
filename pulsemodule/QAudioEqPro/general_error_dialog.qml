import QtQuick 2.0
import QtQuick.Dialogs 1.2

Dialog {
    visible: true
    id: generalErrorDialog
    onClickedButtonChanged: Qt.quit()
    title: qsTr("Something bad happend :(")
    width: 600

    Text {
        id:genErrorText
        objectName: "editErrText"
        text:"General error happend. Aborting"
    }
}
