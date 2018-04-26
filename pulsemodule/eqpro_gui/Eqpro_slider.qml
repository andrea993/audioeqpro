import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.0

Item {

    Layout.fillHeight: true
    Layout.fillWidth: true

    property double freq: 0.0
    property int sliderIdx: 0

    signal inSliderChange(double val, int idx);

        ColumnLayout {
            id: columnLayout
            anchors.fill: parent
            //property int freq

            Slider {
                id: slider
                clip: false
                from: -1
                to: 1
                Layout.fillHeight: true
                Layout.fillWidth: true
                orientation: Qt.Vertical
                value: 0


                onValueChanged: {
                     textF.text=value.toFixed(3).toString();
                     inSliderChange(value, sliderIdx);
                }

            }

            Text {
                id: freqstr
                text: "%1 Hz".arg(freq)
                Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                font.pixelSize: 12
            }

            TextField {
                id: textF
                text: qsTr("0.0")
                Layout.maximumWidth: 70
                scale: 0.8
                horizontalAlignment: Text.AlignHCenter
                font.letterSpacing: 0
                Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                inputMethodHints: Qt.ImhFormattedNumbersOnly
                onTextChanged: {
                    slider.value=parseFloat(text);
                }
            }


            property alias value: slider.value




        }

}
