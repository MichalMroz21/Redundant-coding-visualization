import QtQuick
import QtQuick.Controls 2.15
import QtQuick.Layouts 6.3

import "VisualizeComponents"

Page {
    width: root.width
    height: root.height
    visible: true

    background: Rectangle {
        color: "white"
    }
	
    ColumnLayout {

        id: columnBase

        width: parent.width
        height: parent.height

        anchors.horizontalCenter: parent.horizontalCenter

        spacing: 5

        Layout.alignment: Qt.AlignCenter

        Text{
            id: controlMatrixText
            font.pixelSize: 20
            color: "black"
            Layout.alignment: Qt.AlignHCenter
            text: qsTr("Macierz kontroli parzystości")
        }

        Column {
            id: errorMatrixColumn
            Layout.alignment: Qt.AlignHCenter

        }

        Text{
            id: generationMatrixText
            font.pixelSize: 20
            color: "black"
            Layout.alignment: Qt.AlignHCenter
            text: qsTr("Macierz generacyjna")
        }

        Column {
            id: generationMatrixColumn
            Layout.alignment: Qt.AlignHCenter

        }

        Button {
            id: nextStepButton
            Layout.alignment: Qt.AlignHCenter | Qt.AlignBottom

            text: qsTr("Następny krok")

            onClicked: {
                stackView.push("Hamming.qml");
                Settings.readFile(4)
            }
        }

    }

    Connections{

        target: hammingCode

        id: hammingVisualizeConnection

        Component.onCompleted: {
            var errorMatrixStr = hammingCode.getErrorMatrixStr(true);
            var errorRows = errorMatrixStr.split('\n');
            for (var row of errorRows) {
                let component = Qt.createComponent("VisualizeComponents/ArrayRowLayout.qml");
                component.createObject(errorMatrixColumn, {myArr: row, isExtended: hammingCode.getEncodingExtended()});
            }

            var generationMatrixStr = hammingCode.getGenerationMatrixStr(true);
            var generationRows = generationMatrixStr.split('\n');
            for (row of generationRows) {
                let component = Qt.createComponent("VisualizeComponents/ArrayRowLayout.qml");
                component.createObject(generationMatrixColumn, {myArr: row, isExtended: hammingCode.getEncodingExtended()});
            }
        }

    }

    Connections{
        id: hamGenMatSettingsCon

        target: Settings

        function onLoadedPageContent(output){

            const myArray = output.split("\n");
            controlMatrixText.text = qsTr(myArray[0])
            generationMatrixText.text = qsTr(myArray[1])
            nextStepButton.text = qsTr(myArray[2])
        }
    }

}
