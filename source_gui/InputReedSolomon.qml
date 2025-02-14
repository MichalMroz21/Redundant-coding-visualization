import QtQuick
import QtQuick.Controls 6.3
import QtQuick.Layouts 6.3
import QtQuick 2.15

import "FormComponents"

Page {
    width: root.width
    height: root.height
    visible: true

    Connections{
        target: reedSolomonCode
    }

    Connections{
        id: inputReeSolSettingsCon

        target: Settings

        function onLoadedPageContent(output){

            const myArray = output.split("\n");
            selectedAlgorithmText.text = qsTr(myArray[0])
            //delayText.text = qsTr(myArray[1]) + animationDelay.getValueStr() + " ms"

            animationDelay.ToolTip.text = qsTr(myArray[2])

            vizualizeButton.ToolTip.text = qsTr(myArray[3])
            vizualizeButton.text = qsTr(myArray[4])
            errorMsg.text = qsTr(myArray[5])
            backButton.ToolTip.text = qsTr(myArray[6])
            backButton.text = qsTr(myArray[7])
        }
    }

    background: Rectangle {
        color: "white"
    }

    ColumnLayout{

        scale: (root.width + root.height) / 1500.0

        width: root.width
        height: root.height

        ColumnLayout {

            Layout.alignment: Qt.AlignCenter
            spacing: 20

            Text {
                Layout.alignment: Qt.AlignHCenter

                id: selectedAlgorithmText
                text: "Startowe dane"
                font.pixelSize: 32
            }

            TextField {

                id: rsData
                color: "black"

                property int maxLength : 5;

                Layout.alignment: Qt.AlignHCenter

                background: Item {
                    implicitWidth: 0.4 * root.width
                    implicitHeight: 0.09 * root.height

                    Rectangle {
                        color: "black"
                        height: 1
                        width: parent.width
                        anchors.bottom: parent.bottom
                    }
                }

                validator: RegularExpressionValidator {regularExpression: /^[0-7]{0,5}$/}

                onTextChanged: {
                    if(length > maxLength) remove(maxLength, length);
                }

                font.pointSize: 0.01 * (root.width + root.height)
            }

            Text {
                id: delayText
                Layout.alignment: Qt.AlignHCenter

                //text: "Odstępy animacji: " + animationDelay.getValueStr() + " ms"
                text: animationDelay.getValueStr() + " ms"
                font.pixelSize: 20
            }

            Slider {
                id: animationDelay

                Layout.alignment: Qt.AlignHCenter

                ToolTip.delay: 1000
                ToolTip.visible: hovered
                ToolTip.text: qsTr("Zmień czas")
                snapMode: RangeSlider.SnapOnRelease
                stepSize: 100
                to: 3000
                value: 3000

                property bool isInfinite: value == to

                function getValueStr() {
                    if (isInfinite) {
                        return "∞";
                    } else {
                        return value.toString();
                    }
                }
            }

            Button {
                Layout.alignment: Qt.AlignHCenter

                id: vizualizeButton
                ToolTip.delay: 1000
                ToolTip.visible: hovered
                ToolTip.text: qsTr("Przejdź do wizualizacji algorytmu")
                text: qsTr("Wizualizuj")

                onClicked:{

                    if(rsData.text.length === 5){
                        reedSolomonCode.setInitialData(rsData.text, animationDelay.value, animationDelay.isInfinite);
                        stackView.push("Galois.qml");
                        Settings.readFile(7);
                    }

                    else{
                        errorMsg.visible = true
                        errorMsg.text = "Wpisz 5 cyfr!";
                    }
                }
            }

            Button {
                Layout.alignment: Qt.AlignHCenter

                id: backButton
                ToolTip.delay: 1000
                ToolTip.visible: hovered
                ToolTip.text: qsTr("Wróć do menu głównego")
                text: qsTr("Wróć")

                onClicked:{
                    stackView.clear();
                    Settings.readFile(0);
                }
            }

            Text {
                id: errorMsg
                Layout.alignment: Qt.AlignHCenter

                visible: false
                text: ""
                font.pixelSize: 15
                color: "red"
            }
        }
    }
}
