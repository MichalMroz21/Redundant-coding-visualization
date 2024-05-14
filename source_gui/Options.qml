import QtQuick
import QtQuick.Controls 2.15
import QtQuick.Layouts 6.3


Page {
    width: root.width
    height: root.height
    visible: true
    id: options

    ButtonGroup {
        id: selectLangGroup
    }

    background: Rectangle {
        color: "white"
    }

    ColumnLayout{

        width: options.width
        height: options.height
        spacing: 20

        ColumnLayout{
            Layout.alignment: Qt.AlignCenter

            Text{
                id: languageText
                Layout.alignment: Qt.AlignHCenter
                text: qsTr("Choose language / Wybierz język")
                font.pixelSize: 20

                wrapMode: Text.WordWrap

                width: parent.width //without this it won't wrap LOL

                padding: 10
            }
            RowLayout{

                Layout.alignment: Qt.AlignCenter
                ColumnLayout{
                    Image {
                        id: enIcon
                        source: "../assets/en.png"
                    }
                    RadioButton{
                        checked: Settings.getLanguage() === 0 ? true : false
                        Layout.alignment: Qt.AlignCenter
                        id: enRadioButton
                        ButtonGroup.group: selectLangGroup
                        onClicked: {
                            Settings.setLanguage(0)
                        }
                    }
                }
                ColumnLayout{
                    Layout.alignment: Qt.AlignCenter

                    Image{
                        id: plIcon
                        source: "../assets/pl.png"
                    }
                    RadioButton{
                        checked: Settings.getLanguage() === 1 ? true : false
                        Layout.alignment: Qt.AlignCenter
                        id: plRadioButton
                        ButtonGroup.group: selectLangGroup
                        onClicked: {
                            Settings.setLanguage(1)
                        }
                    }
                }

            }


            Button {
                id: backButton
                Layout.alignment: Qt.AlignHCenter
                ToolTip.delay: 1000
                ToolTip.visible: hovered
                ToolTip.text: qsTr("Wróć do menu głównego")
                text: qsTr("Wróć")

                onClicked:{
                    stackView.clear(); //will go back to Main.qml, dont do push(main.qml)
                    Settings.readFile(0);
                    //Settings.refreshLanguage()
                }
            }
        }
    }

    Connections{

        target: Settings

        id: settingsConnection

        function onLoadedPageContent(output){
            const myArray = output.split("\n");
            backButton.ToolTip.text = qsTr(myArray[0])
            backButton.text = qsTr(myArray[1])
        }

        function onLangaugeChanged(value){
            switch(value){
            //English
            case 0:
                enRadioButton.checked = true
                backButton.ToolTip.text = qsTr("Go back to main menu")
                backButton.text = qsTr("Back")
                break
            //Polish
            case 1:
                plRadioButton.checked = true
                backButton.ToolTip.text = qsTr("Wróć do menu głównego")
                backButton.text = qsTr("Wróć")
                break
            }
        }
    }



}
