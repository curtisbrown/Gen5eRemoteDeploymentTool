import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import Qt.labs.platform 1.0
import QtQuick.Dialogs 1.2
import QtQuick.Window 2.3

Rectangle {
    id: platformSelectWindow
    color: "white"
    visible: true

    Item {
        anchors.fill: parent

        Image {
            id: logo
            source: "qrc:/status/images/ctdiBackground.png"
            opacity: 0.4
            fillMode: Image.PreserveAspectFit
        }
    }

    RowLayout {
        anchors.centerIn: parent

        Item {
            width: 100
            height: 50
            RoundButton {
                id: oktraButton
                anchors.fill: parent
                visible: platformSelectWindow.visible
                Text { text: "Bay"; color: "blue"; font.bold: true; anchors.centerIn: parent }
                onClicked: {
                    console.log("Bay Button Pressed");
                    platformSelectWindow.visible = false
                    controllerDeploy.visible = true;
                }
            }
        }
        Item {
            width: 100
            height: 50
            RoundButton {
                id: raspberryPi
                anchors.fill: parent
                visible: platformSelectWindow.visible
                width: 100
                height: 50
                Text { text: "Raspberry Pi"; color: "blue"; font.bold: true; anchors.centerIn: parent }
                onClicked: {
                    console.log("Raspberry Pi Button Pressed");
                    platformSelectWindow.visible = false
                    piDeploy.visible = true
                }
            }
        }
    }
}
