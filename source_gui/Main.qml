import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts 6.3
import QtQuick.Dialogs
import QtQuick

Window {

    id: root
    width: 1280
    height: 720
    visible: true
    title: qsTr("Redundant Coding")

    RotationAnimator {
        id: rotationAnimator
        from: 0;
        to: 360;
        duration: 1000
        loops: Animation.Infinite
    }

    Row{

        ColumnLayout{
            id:menuSidebarColumn
            width: root.width / 4
            height: root.height

            ColumnLayout{

                id:menuColumn
                Layout.alignment: Qt.AlignCenter
                spacing: 10

                Text {
                    id: selectedAlgorithmText
                    text: "Wybrany algorytm:"
                    font.pixelSize: 16
                }

                Text {
                    id: algorithmText
                    text: qsTr("Hamming")
                    font.pixelSize: 20
                }

                ComboBox {
                    id: comboBox
                    model: ListModel{
                        ListElement { text: "Hamming" }
                        ListElement { text: "Reed–Solomon" }
                    }
                    onActivated:{
                        switchContent(currentIndex);
                    }
                }

                Button {
                    id: confirmButton
                    text: qsTr("Zatwierdź")
                    ToolTip.delay: 1000
                    ToolTip.visible: hovered
                    ToolTip.text: qsTr("Potwierdź wybór algorytmu")

                    onClicked: {
                        switchPage(comboBox.currentIndex);
                    }
                }

                Button {
                    id: exitButton
                    text: qsTr("Wyjście")
                    ToolTip.delay: 1000
                    ToolTip.visible: hovered
                    ToolTip.text: qsTr("Zakończ działanie programu")

                    onClicked: messageDialog.open()
                }

                MessageDialog {
                    id: messageDialog
                    title: "Zakończ"
                    text: "Czy chcesz zakończyć działanie programu?"
                    buttons: MessageDialog.Yes | MessageDialog.Cancel
                    onButtonClicked: function (button, role) {
                        switch (button) {
                        case MessageDialog.Yes:
                            console.log("End of the program.")
                            Qt.quit()
                            break;
                        }

                    }
                    Component.onCompleted: visible = false
                }

                Image {
                    Layout.preferredWidth: parent.width / 5
                    Layout.preferredHeight: Layout.preferredWidth

                    id: options

                    source: "../assets/options.png"
                    fillMode: Image.PreserveAspectFit

                    MouseArea{
                        id:optionsMouseArea
                        anchors.fill: parent
                        hoverEnabled: true

                        onEntered:{
                            rotationAnimator.target = parent;
                            rotationAnimator.running = true;
                        }

                        onExited:{
                            rotationAnimator.running = false;
                        }

                        onClicked:{
                            stackView.push("Options.qml");
                            Settings.readFile(1);
                            //Settings.refreshLanguage();
                        }
                    }
                }

            }

        }

        ColumnLayout{
            id:descriptiveColumn
            height: root.height
            width: root.width / 4 * 3
            spacing: 0

            Rectangle{
                id:imageRectangle

                width: parent.width
                height: parent.height*0.7
                Layout.alignment: Qt.AlignCenter

                Image {
                    id: image
                    anchors.centerIn: parent
                    Layout.alignment: Qt.AlignCenter
                    source: "../assets/Hamming.png"
                    fillMode: Image.PreserveAspectFit
                }
            }

            Rectangle {
                id:descriptionRectangle
                Layout.alignment: Qt.AlignCenter
                width: parent.width
                height: parent.height*0.3

                Text {
                    id: descriptionText

                    text: qsTr("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.")
                    font.pixelSize: 20

                    wrapMode: Text.WordWrap

                    onLinkActivated: {
                        Qt.openUrlExternally(link)
                    }

                    width: parent.width

                    padding: 10
                }
            }

        }
    }

    StackView{
       id: stackView
       anchors.fill: parent
    }

    Component.onCompleted: {
        switchContent(0);
    }

    Connections{
        id: mainSettingsCon

        target: Settings

        function onLoadedPageContent(output){

            const myArray = output.split("\n");
            selectedAlgorithmText.text = qsTr(myArray[0])
            confirmButton.text = qsTr(myArray[1])
            confirmButton.ToolTip.text = qsTr(myArray[2])
            exitButton.text = qsTr(myArray[3])

            messageDialog.title = qsTr(myArray[4])
            messageDialog.text = qsTr(myArray[5])
            switchContent(comboBox.currentIndex)
        }

        // function onLangaugeChanged(value){

        //     switch(value){

        //     //English
        //     case 0:

        //         selectedAlgorithmText.text = qsTr("Selected algorithm: ")
        //         confirmButton.text = qsTr("Confirm")
        //         confirmButton.ToolTip.text = qsTr("Confirm your choice of algorithm")
        //         exitButton.text = qsTr("Exit")

        //         messageDialog.title = qsTr("Close")
        //         messageDialog.text = qsTr("Do you want to close the program?")
        //         break

        //     //Polish
        //     case 1:
        //         selectedAlgorithmText.text = qsTr("Wybrany algorytm: ")
        //         confirmButton.text = qsTr("Zatwierdź")
        //         confirmButton.ToolTip.text = qsTr("Potwierdź wybór algorytmu")
        //         exitButton.text = qsTr("Zakończ")

        //         messageDialog.title = qsTr("Zakończ")
        //         messageDialog.text = qsTr("Czy chcesz zakończyć działanie programu?")
        //         break
        //     }

        //     switchContent(comboBox.currentIndex)
        //     //Settings.readFile(0)
        // }
    }

    function switchPage(index) {

        var pageSelected;

        switch (index) {
            case 0: pageSelected = "inputHamming.qml"; break;
            case 1: pageSelected = "InputReedSolomon.qml"; break;
        }

        stackView.push(pageSelected);
        switch(index){
            case 0: Settings.readFile(2); break;
            case 1: Settings.readFile(6); break;
        }
    }

    function switchContent(index) {

        var imgDescAlgo = []
        var language = Settings.getLanguage()

        switch (language){
        case 0:
            switch (index) {

                case 0: {
                    imgDescAlgo = ["The Hamming (n,k) code encodes k bits of information on n bits using additional parity bits. It allows detection and repair of a single error.In this implementation, the code is generated based on a parity check matrix, where the numerical representation of the error syndrome indicates the position at which the error occurred (syndrome 0 means no error). The parity bits are placed at positions that are powers of 2 (1, 2, 4...), the first contains the XOR of bits that are at positions that have a 1 on the youngest bit, the second on the second youngest bit, etc.", "../assets/Hamming.png", "Hamming"];
                    break;
                }
                case 1: {
                    imgDescAlgo = ["A sample implementation of Reed-Solomon code using 5 data symbols and 2 redundant symbols in Galois Field (3-bit data). In practice, blocks can be significantly larger and represent any binary data, e.g. 232 + 23 symbols in GF(256). Thanks to the operation on blocks and the ability to correct clustered errors, such as scratches on a CD, Reed-Solomon code is exceptionally effective in protecting data. Based on <a href='https://en.wikiversity.org/wiki/Reed%E2%80%93Solomon_codes_for_coders#Finite_field_arithmetic'>wikiversity implementation.</a>", "../assets/RS_BER.png", "Reed–Solomon"];
                    break;
                }
            }
            break

        case 1:
            switch (index) {

                case 0: {
                    imgDescAlgo = ["Kod Hamminga (n,k) koduje k bitów informacji na n bitach z użyciem dodatkowych bitów parzystości. Pozwala na detekcję i naprawę pojedynczego błędu.\nW tej implementacji kod jest generowany na podstawie macierzy kontroli parzystości, gdzie reprezentacja liczbowa syndromu błędu wskazuje na pozycję, na której wystąpił błąd (syndrom 0 oznacza brak błędu). Bity parzystości są umieszczone na pozycjach będących potęgami 2 (1, 2, 4...), pierwszy zawiera XORa bitów będących na pozycjach, które mają 1 na najmłodszym bicie, drugi na drugim najmłodszym bicie itp", "../assets/Hamming.png", "Hamminga"];
                    break;
                }
                case 1: {
                    imgDescAlgo = ["Przykładowa implementacja kodu Reeda-Solomona używająca 5 symboli danych i 2 symboli redundantnych w Galois Field(8) (dane 3-bitowe). W praktyce bloki są znacznie większe i reprezentować dowolne dane binarne, przykładowo 232 + 23 symboli w GF(256). Dzięki działaniu na blokach i możliwości korekcji skupionych błędów, jak na przykład zadrapania na płycie CD, kod Reeda-Solomona jest wyjątkowo skuteczny w ochronie danych. Na podstawie <a href='https://en.wikiversity.org/wiki/Reed%E2%80%93Solomon_codes_for_coders'>implementacji wikiversity.</a>", "../assets/RS_BER.png", "Reeda–Solomona"];
                    break;
                }
            }
            break
        }


        descriptionText.text = imgDescAlgo[0];
        image.source = imgDescAlgo[1];
        algorithmText.text = imgDescAlgo[2];
    }

}
