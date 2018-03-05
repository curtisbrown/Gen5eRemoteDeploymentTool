import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.3

Rectangle {
    id: platformSelectWindow
    color: "white"
    visible: true

    RowLayout {
        anchors.centerIn: parent

        Button {
            id: oktra
            visible: platformSelectWindow.visible
            text: qsTr("Bay")
            onClicked: {
                console.log("Bay Button Pressed");
                platformSelectWindow.visible = false
                controllerDeploy.visible = true;
            }
        }
        Button {
            id: raspberryPi
            visible: platformSelectWindow.visible
            text: qsTr("Raspberry Pi")
            onClicked: {
                console.log("Raspberry Pi Button Pressed");
                platformSelectWindow.visible = false
                piDeploy.visible = true
            }
        }
    }
}
