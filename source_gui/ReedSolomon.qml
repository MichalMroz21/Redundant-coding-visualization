import QtQuick
import QtQuick.Controls 2.15
import QtQuick.Layouts 6.3

import "VisualizeComponents"

Page {
    width: root.width
    height: root.height
    visible: true
    id: reedSolomonPage

    property var wordsWithTooltips: {
        "coef_pos": translationArray.ar[37],
        "errata_locator": translationArray.ar[38],
        "error_evaluator": translationArray.ar[39],
        "err_loc_prime": translationArray.ar[40],
        "Locator": translationArray.ar[41]
    }

    QtObject {
        id: translationArray
        property var ar: []
    }

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
            id: stageText
            topPadding: root.height / 16
            font.pixelSize: 40
            color: "black"
            Layout.alignment: Qt.AlignHCenter | Qt.AlignTop
        }

        Text{
            id: stageTextExt
            Layout.alignment: Qt.AlignHCenter

            text: "Zmień od 0 do 1 liczb klikając na nie"
            font.pixelSize: 15
            font.italic: true
            color: "black"
            visible: false
            Layout.fillHeight: true
        }

        Column {
            id: emptyarrayRowLayoutRowColumn

            Layout.alignment: Qt.AlignHCenter

            topPadding: 20
            visible: false
            Layout.fillHeight: true
        }

        Row {
            spacing: 10 // wielkość spacji między słowami
            Layout.alignment: Qt.AlignHCenter
            Layout.fillHeight: true
            Repeater {
                id: belowText
                model: []
                anchors.horizontalCenter: parent.horizontalCenter
                Layout.alignment: Qt.AlignHCenter
                Layout.fillHeight: true

                Item {
                    width: textItem.implicitWidth
                    height: textItem.implicitHeight

                    Text {
                        id: textItem
                        text: modelData
                        font.pixelSize: 40
                        color: "black"
                        MouseArea {
                            anchors.fill: parent
                            hoverEnabled: true
                            onEntered: {
                                // wyświetla tooltip tylko nad określonymi słowami
                                if (wordsWithTooltips[textItem.text]) {
                                    tooltip.text = wordsWithTooltips[textItem.text]
                                    tooltip.show(textItem.mapToItem(null, 0, 0))
                                } else if (textItem.text && textItem.text === "coef_pos)") { // obejście
                                    tooltip.text = wordsWithTooltips["coef_pos"]
                                    tooltip.show(textItem.mapToItem(null, 0, 0))
                                }
                                else if (textItem.text && textItem.text === "Lokator") { // tłumaczenie
                                    tooltip.text = wordsWithTooltips["Locator"]
                                    tooltip.show(textItem.mapToItem(null, 0, 0))
                                }
                            }
                            onExited: tooltip.hide()
                        }
                    }
                }
            }
        }

        Text{
            id: belowTextExtended
            anchors.horizontalCenter: parent.horizontalCenter
            font.pixelSize: 40
            color: "black"
            text: ""
            Layout.alignment: Qt.AlignHCenter
            Layout.fillHeight: true
        }

        Column {
            bottomPadding: root.height / 8
            Layout.alignment: Qt.AlignHCenter
            ColumnLayout {
                spacing: 10
                Layout.alignment: Qt.AlignHCenter

                Text {
                    id: animationDelayText
                    Layout.alignment: Qt.AlignHCenter

                    text: "Odstępy animacji: " + animationDelay.getValueStr() + " ms"
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
                    value: reedSolomonCode.getAnimationDelayMs()

                    property bool isInfinite: value == to

                    onValueChanged: {
                        if (value == to) {
                            reedSolomonCode.setInfiniteWait(true);
                        } else {
                            reedSolomonCode.setInfiniteWait(false);
                            reedSolomonCode.setAnimationDelayMs(value);
                        }
                    }

                    function getValueStr() {
                        if (isInfinite) {
                            return "∞";
                        } else {
                            return value.toString();
                        }
                    }
                }

            }
        }

        Button {
            id: visualiseButton
            Layout.alignment: Qt.AlignHCenter

            text: qsTr("Zacznij")

            onClicked:{
                rsVisualizeConnection.correctingErrors();
            }

            visible: false
        }

        Button {
            id: mainMenuButton
            Layout.alignment: Qt.AlignHCenter

            text: qsTr("Menu Główne")

            Layout.margins: root.height / 20

            onClicked:{
                stackView.clear();
                Settings.readFile(0);
            }

            visible: false
        }

        ToolTip {
            id: tooltip
            visible: false
            delay: 100
            font.italic: true

            font.pixelSize: 30
            background: Rectangle {
                color: "white"
            }

            // timeout: 3000
            width: Math.min(parent.width / 2, contentWidth)

            function show(pos) {
                x = pos.x
                y = pos.y - height - 10
                visible = true
            }
            function hide() {
                visible = false
            }
        }

        Button {
            id: nextStepButton
            Layout.alignment: Qt.AlignHCenter

            text: qsTr("Następny krok")

            onClicked: {
                reedSolomonCode.pressButton();
            }

            visible: animationDelay.isInfinite && !visualiseButton.visible && !mainMenuButton.visible
        }

    }


    Item{
        id: formBase
    }

    Timer {
       id: timer
    }

    Connections{
        id: reeSolSettingsCon

        target: Settings

        function onLoadedPageContent(output){

            translationArray.ar = output.split("\n");
            stageTextExt.text = qsTr(translationArray.ar[0])
            animationDelayText.text = qsTr(translationArray.ar[1])
            animationDelay.ToolTip.text = qsTr(translationArray.ar[2])
            visualiseButton.text = qsTr(translationArray.ar[3])
            mainMenuButton.text = qsTr(translationArray.ar[4])
            nextStepButton.text = qsTr(translationArray.ar[5])
            stageText.text = qsTr(translationArray.ar[6])
        }
    }

    Connections{

        target: reedSolomonCode

        id: rsVisualizeConnection

        property var arrays : []

        function delay(delayTime, cb) {
            timer.interval = delayTime;
            timer.repeat = false;
            timer.triggered.connect(cb);
            timer.start();
        }

        function getArrayStr(arrIndex){

            var bitStr = "";

            for(var i = 0; i < arrays[arrIndex].array.length - 1; i++){
                bitStr += arrays[arrIndex].array[i].children[0].text;
            }

            return bitStr;
        }

        function onPushEmptyArray(size){
            var component = Qt.createComponent("VisualizeComponents/RSEmptyArrayRowLayout.qml");
            var emptyArrayLayout = component.createObject(emptyarrayRowLayoutRowColumn, {model: size});

            arrays.push(emptyArrayLayout);

            emptyarrayRowLayoutRowColumn.visible = true;
        }

        function onEndErrorCorrection() {
            nextStepButton.visible = false;
            mainMenuButton.visible = true;
        }

        function onInsertArray(index, str, showSymbols){

            if(arrays.length === 0){
                onPushArray(str, showSymbols);
                return;
            }

            var bitStrs = [];

            for(var i = 0; i < arrays.length; i++){
                bitStrs.push(getArrayStr(i));
            }

            for(var k = 0; k < arrays.length; k++){
                onDeleteArrayAtIndex(k);
            }

            for(var j = 0; j < bitStrs.length; j++){
                if(j === index) onPushArray(str, showSymbols);
                onPushArray(bitStrs[j], showSymbols);
            }
        }

        function onPopArray(){
            if(arrays.length > 0){
                arrays[arrays.length - 1].destroy();
                arrays.pop();
            }
        }

        function onDeleteArrayAtIndex(index){
            if(arrays.length > index){
                arrays[index].destroy();
                arrays.splice(index, 1);
            }
        }

        function onSetBit(arrIndex, index, bit){
            arrays[arrIndex].array[index].children[0].text = bit
        }

        function onSetTopText(text) {
            stageText.text = text
        }

        function onSetTopTextTranslation(text, array) {
            stageText.text = translationArray.ar[text] + array.join("");
        }

        function onInsertBit(arrIndex, index, bit, showSymbols){
            var bitStr = getArrayStr(arrIndex);

            onDeleteArrayAtIndex(arrIndex);

            bitStr = bitStr.slice(0, index) + bit + bitStr.slice(index);

            onInsertArray(arrIndex, bitStr, showSymbols);
        }

        function onInsertEmptyBit(arrIndex, index, bit){
            var bitStr = getArrayStr(arrIndex);

            onDeleteArrayAtIndex(arrIndex);

            bitStr = bitStr.slice(0, index) + " " + bitStr.slice(index);

            onInsertArray(arrIndex, bitStr);
        }

        function onTurnBitOff(arrIndex, index){
            arrays[arrIndex].array[index].color = "white";
        }

        function onTurnBitOn(arrIndex, index, color){
            if(color === "") color = "red";
            arrays[arrIndex].array[index].color = color;
        }

        function onSetBelowText(str){
            belowText.model = str.split(" ");
        }

        function onSetBelowTextExtended(str) {
            belowTextExtended.text = str;
        }

        function onSetBelowTextTranslation(text, array){
            belowText.model = (translationArray.ar[text] + array.join("")).split(" ");
        }

        function onSetBelowTextExtendedTranslation(text, array) {
            belowTextExtended.text = translationArray.ar[text] + array.join("");
        }

        function onSetBelowTextTranslationColorized(color, text, array){
            //make sure to write "</font> in array at the desired position
            belowText.model = ("<font color=\"" + color + "\">" + translationArray.ar[text] + array.join("")).split(" ");
        }

        function onSetBelowTextCalcSyndromes(i, j){
            //calcSyndromes1 + i + " - " + calcSyndromes2 + j
            //see language pack - reedSolomonPage at the 20th and 21st indexes
            belowText.model = (translationArray.ar[21] + i + " - " + translationArray.ar[22] + j).split(" ");
        }

        function onSetClickAllow(arrIndex, isAllowed){
            arrays[arrIndex].clickChange = isAllowed;
        }

        Component.onCompleted: {
            //stageText.text = "Encoding...";
            stageText.text = "Kodowanie...";
            reedSolomonCode.encodeData(true);
        }

        function onEncodingEnd(){

            belowTextExtended.text = "";
            onSetBelowText("");
            //stageText.text = "Poprawianie błędów";
            stageText.text = translationArray.ar[7];
            stageTextExt.visible = true;
            visualiseButton.visible = true;
            arrays[0].myArr = reedSolomonCode.getDataEncoded();
            onSetClickAllow(0, true);
        }

        function correctingErrors(){

            onSetClickAllow(0, false);
            stageText.text = translationArray.ar[8];
            stageTextExt.visible = false;
            visualiseButton.visible = false;

            reedSolomonCode.sendCode(getArrayStr(0));
            reedSolomonCode.correctErrorQml(true);
        }
    }

}
