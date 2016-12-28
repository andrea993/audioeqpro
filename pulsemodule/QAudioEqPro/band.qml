import QtQuick 2.0
import QtQuick.Controls 1.4

Item {
    property int idx: 1
    id: qsTr("bandItem"+idx)

    Slider {
        id: band
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
        id: textCurrentBandValue
        x: 0
        y: 64
        width: 14
        height: 17
        text: band.value
    }
}
