import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.0

Item {

    id: page1
    objectName: "page1"

    signal dialChange(double val);

    property bool presetIsChanging: false
    property alias dialvalue: dial.value

    ColumnLayout {
        anchors.fill: parent

        Flickable {

            id: flickable
            Layout.fillHeight: true
            Layout.fillWidth: true
            focus: true

            contentHeight: height
            contentWidth: width*slidersRow.nBands/10

            ScrollBar.horizontal: ScrollBar {
                parent: flickable.parent
                anchors.left: flickable.left
                anchors.right:  flickable.right
                anchors.bottom: flickable.bottom

            }
            flickableDirection: Flickable.HorizontalFlick


            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
            RowLayout {
                objectName: "slidersRow"
                id: slidersRow

                height: parent.height
                width: page1.width/10*nBands

                property int nBands: 10
                property double fmin: 1000.0
                property double dB: 12
                property double r: 0.25;

                signal sliderChange(double val, int idx)

                Component.onCompleted: {
                    //redrawSlider();
                }

                function inSliderChanged(val,idx) {
                    sliderChange(val, idx)

                    if(!presetIsChanging)
                        comboBoxPresets.currentIndex=0;
                }

                function redrawSlider(par) {
                    var i;

                    for (i=0; i<slidersRow.children.length; i++) {
                       slidersRow.children[i].destroy();

                    }

                    var sli=Qt.createComponent("EqproSlider.qml");
                    var fmax=fmin*Math.pow(r,nBands-1);
                    for (i=0; i<nBands; i++) {
                        var f=Math.round(Math.exp(Math.log(fmin)+Math.log(fmax / fmin)*i/(nBands-1)))
                        var sli_i=sli.createObject(slidersRow,{
                                                "id": "slider_"+i,
                                                "objectName": "slider_"+i,
                                                "Layout.alignment": Qt.AlignHCenter | Qt.AlignVCenter,
                                                "freq": f,
                                                "sliderIdx": i,
                                                "val": par[i]
                                            });
                        sli_i.inSliderChange.connect(slidersRow.inSliderChanged);
                    }

                }

            }


        }

        RowLayout {
            id: rowLayout1
            width: 100
            height: 100
            Layout.columnSpan: 1
            spacing: 5
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
            Layout.rowSpan: 1
            Layout.fillWidth: true
            Layout.maximumHeight: 100
            Layout.maximumWidth: 65535


            Text {
                text: qsTr("Preset")
                fontSizeMode: Text.FixedSize
                scale: 1
                Layout.fillWidth: false
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                font.pixelSize: 15
                Layout.leftMargin: 10
            }

            ComboBox {
                id: comboBoxPresets
                scale: 0.8
                Layout.minimumWidth: 300
                Layout.fillWidth: false

                model: ListModel {
                    id: presets
                    ListElement { text: "Custom" }
                    ListElement { text: "Flat" }
                }

                onCurrentIndexChanged: {
                    var preset = presets.get(currentIndex).text

                    presetIsChanging = true;
                    if (preset === "Flat") {
                        for (var i=0; i<slidersRow.nBands; i++)
                            slidersRow.children[i].val=0;
                    }

                    presetIsChanging = false;
                }

            }
            GridLayout {
                id: gridLayout
                width: 100
                height: 100
                transformOrigin: Item.Center
                Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                columns: 2
                Layout.fillWidth: true

                Text {
                    id: text1
                    text: qsTr("Gain:")
                    font.pixelSize: 12
                }

                TextInput {
                    id: textInput
                    width: 80
                    height: 20
                    text: qsTr("%1 dB".arg(slidersRow.dB))
                    Layout.alignment: Qt.AlignRight | Qt.AlignTop
                    font.pixelSize: 12
                }


                Text {
                    text: qsTr("Bands:")
                }

                Text {
                    id: text3
                    text: slidersRow.nBands.toString()
                    font.pixelSize: 12
                }
            }

            Dial {
                id: dial
                value: 0
                from: 0
                to: 1
                Layout.margins: 5
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                Layout.fillHeight: true
                Layout.fillWidth: false

                onValueChanged: {
                    dialChange((Math.pow(10,dial.value/20.0)-1)/0.1220184543019634355910389) //(10^(1/20)-1)

                }
            }



        }








    }


}
