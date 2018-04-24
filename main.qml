import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import Qt.labs.platform 1.0
import QtQuick.Dialogs 1.2
import QtQuick.Window 2.3

ApplicationWindow {
    visible: true
    width: 640
    height: 480
    title: qsTr("Gen5e Deployment Assistant")

    Item {
        anchors.fill: parent

        Image {
            id: logo
            source: "qrc:/status/images/ctdiBackground.png"
            opacity: 0.4
            fillMode: Image.PreserveAspectFit
        }
    }


    MenuBar {
        Menu {
            title: "File"
            MenuItem {
                text: "Close"
                onTriggered: Qt.quit()
            }
        }

        Menu {
            title: "Edit"
            MenuItem {
                text: "Reset"
                onTriggered: {
                    reset()
                    guiAI.reset()
                }
            }
        }
    }

    RoundButton {
        anchors.centerIn: parent
        width: 200
        height: 50
        Text { text: "Deploy File to Remote location"; color: "blue"; font.bold: true; anchors.centerIn: parent }
        onClicked: selectPlatform.visible = true
    }

    SelectRemotePlatform {
        id: selectPlatform
        visible: false
        anchors.fill: parent
    }

    PiDeployment {
        id: piDeploy
        visible: false
        anchors.fill: parent
    }

    OktraControllerDeployment {
        id: controllerDeploy
        visible: false
        anchors.fill: parent
    }

    function reset() {
        console.log("RESET QML features")
        selectPlatform.visible = false
        piDeploy.reset()
        piDeploy.visible = false
        controllerDeploy.visible = false
    }
}
