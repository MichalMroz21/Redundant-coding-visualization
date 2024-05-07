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


        Image {
            Layout.alignment: Qt.AlignHCenter

            // Layout.preferredWidth: parent.width / 5
            // Layout.preferredHeight: Layout.preferredWidth

            source: "../assets/galois.png"
            fillMode: Image.PreserveAspectFit
        }

        Button {
            Layout.alignment: Qt.AlignHCenter | Qt.AlignBottom

            text: qsTr("Następny krok")

            onClicked: {
                stackView.push("ReedSolomon.qml");
            }
        }

    }

}
