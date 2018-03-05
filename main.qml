import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.3
import Qt.labs.platform 1.0

ApplicationWindow {
    visible: true
    width: 640
    height: 480
    title: qsTr("Gen5e Deployment Assistant")

    Item {
        width: parent.width / 3.5
        height: parent.height / 8
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        Image {
            id: logo
            source: "qrc:/status/images/ctdiLogo.png"
            anchors.right: parent.right
            anchors.bottom: parent.bottom
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

    Button {
        anchors.centerIn: parent
        text: "Deploy File to Remote location"
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
