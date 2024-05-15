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

        spacing: 0

        Layout.alignment: Qt.AlignCenter

        Text{
            id: receivedMessageText
            font.pixelSize: 20
            color: "black"
            Layout.alignment: Qt.AlignHCenter
            text: qsTr("Otrzymana wiadomość")
        }

        Column {
            id: receivedColumn
            Layout.alignment: Qt.AlignHCenter

        }

        Text {
            id: syndromeText
            font.pixelSize: 20
            color: "black"
            Layout.alignment: Qt.AlignHCenter
            text: qsTr("Syndrom błędu")
        }

        Column {
            id: syndromColumn
            Layout.alignment: Qt.AlignHCenter

        }

        Text{
            id: vectorText
            font.pixelSize: 20
            color: "black"
            Layout.alignment: Qt.AlignHCenter
            text: qsTr("Wektor błędów")
        }

        Column {
            id: errorColumn
            Layout.alignment: Qt.AlignHCenter

        }

        Text{
            id: xorText
            font.pixelSize: 20
            color: "black"
            Layout.alignment: Qt.AlignHCenter
            text: qsTr("Uzyskana wiadomość; Otrzymana wiadomość XOR Wektor błędów")
        }

        Column {
            id: encodedColumn
            Layout.alignment: Qt.AlignHCenter
        }

        Text{
            id: decodedText
            font.pixelSize: 20
            color: "black"
            Layout.alignment: Qt.AlignHCenter
            text: qsTr("Odkodowana wiadomość")
        }

        Column {
            id: endColumn
            Layout.alignment: Qt.AlignHCenter
        }

        Button {
            id: mainMenuButton
            Layout.alignment: Qt.AlignHCenter | Qt.AlignBottom

            //text: qsTr("Main Menu")
            text: qsTr("Menu główne")

            onClicked: {
                stackView.clear();
                Settings.readFile(0)
            }
        }

    }

    Connections{
        id: hamSynSettingsCon

        target: Settings

        function onLoadedPageContent(output){

            const myArray = output.split("\n");
            receivedMessageText.text = qsTr(myArray[0])
            syndromeText.text = qsTr(myArray[1])
            vectorText.text = qsTr(myArray[2])
            xorText.text = qsTr(myArray[3])
            decodedText.text = qsTr(myArray[4])
            mainMenuButton.text = qsTr(myArray[5])
        }
    }

    Connections{

        target: hammingCode

        id: hammingVisualizeConnection

        Component.onCompleted: {
            let component = Qt.createComponent("VisualizeComponents/ArrayRowLayout.qml");

            let syndrom = hammingCode.getSyndrome();
            component.createObject(syndromColumn, {myArr: syndrom, isExtended: hammingCode.getEncodingExtended()});


            let error = hammingCode.getError();
            component.createObject(errorColumn, {myArr: error, isExtended: hammingCode.getEncodingExtended()});

            let encoded = hammingCode.getEncodedStr();
            component.createObject(encodedColumn, {myArr: encoded, isExtended: hammingCode.getEncodingExtended(), showSymbols: true});

            let received = hammingCode.getReceivedCode();
            component.createObject(receivedColumn, {myArr: received, isExtended: hammingCode.getEncodingExtended()});

            let decoded = hammingCode.getDecodedStr();
            component.createObject(endColumn, {myArr: decoded, isExtended: hammingCode.getEncodingExtended(), showSymbols: true, simpleSymbols: true});
        }

    }
}
