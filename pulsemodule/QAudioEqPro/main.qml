import QtQuick 2.4
import QtQuick.Window 2.2
import QtQuick.Controls 1.4

Window {
    id: mainwindow
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

    SpinBox {
        id: chooseBands
        x: 0
        y: 25
        decimals: 0
        maximumValue: utils.getMaxBands()
        minimumValue: 1
        suffix: (chooseBands.value == 1) ? " band" : " bands"
        onEditingFinished: utils.changedBands(chooseBands.value)
    }

    Slider {
        id: firstBand
        x: 20
        y: 64
        width: 22
        height: 200
        stepSize: 0.5
        value: 0.0
        maximumValue: 15.0
        minimumValue: -15.0
        updateValueWhileDragging: true
        orientation: Qt.Vertical
    }

    Text {
        id: reprCurrentValueFirstBand
        x: 0
        y: 64
        width: 14
        height: 17
        text: firstBand.value
    }
}
