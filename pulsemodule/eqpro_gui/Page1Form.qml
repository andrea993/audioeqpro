import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.0

Item {

    id: page_form


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
                width: page_form.width/10*nBands


                property int nBands
                property double fmin: 30.0
                property double oct: 0.5
                property double sR: 44100;




                Component.onCompleted: {
                    redrawSlider();
                }

                function redrawSlider() {
                    var i;

                    for (i=0; i<slidersRow.children.length; i++) {
                       slidersRow.children[i].destroy();

                    }

                    var sli=Qt.createComponent("Eqpro_slider.qml");
                    var R=Math.pow(2,oct);
                     var n=Math.round(Math.log(sR/2/fmin)/Math.log(R));
                    nBands=n;
                    var fmax=fmin*Math.pow(R,nBands-1);
                    for (i=0; i<n; i++) {
                        var f=Math.round(Math.exp(Math.log(fmin)+Math.log(fmax / fmin)*i/(nBands-1)))
                        sli.createObject(slidersRow,{
                                                "id": "slider_"+i,
                                                "Layout.alignment": Qt.AlignHCenter | Qt.AlignVCenter,
                                                "freq": f
                                            });

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
                id: text4
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
                id: comboBox
                scale: 0.8
                Layout.minimumWidth: 300
                Layout.fillWidth: false
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
                    text: qsTr("12 dB")
                    Layout.alignment: Qt.AlignRight | Qt.AlignTop
                    font.pixelSize: 12
                }


                Text {
                    id: text2
                    text: qsTr("Bands:")

                }

                Text {
                    id: text3
                    text: qsTr("10")
                    font.pixelSize: 12
                }
            }

            Dial {
                id: dial
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                Layout.fillHeight: true
                Layout.fillWidth: false
            }




        }








    }


}
