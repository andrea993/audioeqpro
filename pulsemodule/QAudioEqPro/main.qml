import QtQuick 2.4
import QtQuick.Window 2.2
import QtQuick.Controls 1.4

Window {
    property int currentSliders: 0

    id: mainwindow
    visible: true
    width: 640
    height: 300
    title: qsTr("QAudioEqPro")

    function onCreateSlider(value) {
        if(currentSliders == value)
            return;
        else if(value > currentSliders) {
            for(var idx=1;idx<=(value-currentSliders);idx++) {
                console.log("Creating object: "+(idx+currentSliders));

            }

            currentSliders=value;
        }
        else if(value < currentSliders) {
            for(var idx=(currentSliders-value);idx>=1;idx--) {
                console.log("Destroying object...: "+currentSliders);

                currentSliders--;
            }
        }
    }

    Component.onCompleted: {
        onCreateSlider(1);
    }


    Rectangle {
        id: mainRect
        color: "green"
        width: 640
        height: 25
        Text {
            objectName: "welcomeTextObj"
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
        onEditingFinished: {
            utils.changedBands(chooseBands.value)
            onCreateSlider(chooseBands.value)
        }
    }
}
