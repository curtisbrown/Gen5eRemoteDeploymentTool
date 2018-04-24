import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Dialogs 1.2
import QtQuick.Window 2.3

import com.CTDI.DeployTool 1.0

Rectangle {
    id: piDeployment
    color: "white"
    visible: true
    property string fileToDeploy: ""
    property string subnet: ""

    Column {
        id: options
        spacing: 10
        width: parent.width
        height: parent.height / 2

        Row {
            spacing: 5
            width: parent.width

            Label {
                width: parent.width / 3
                wrapMode: Label.NoWrap
                color: "blue"
                font.pixelSize: 16
                font.italic: true
                text: "Select Controller Subnet"
            }
            ComboBox {
                id: networkSelect
                currentIndex: -1
                model: ["172.168.1", "192.168.9"]
                onActivated: {
                    subnet = currentText
                    for(var i = 0; i < 16; ++i) {
                        guiAI.piControl(i).setControllerSubNet(currentText)
                        guiAI.piControl(i).remoteConnectionActive()
                    }
                }
            }
        }
        Row {
            spacing: 5
            width: parent.width

            Label {
                width: parent.width / 3
                wrapMode: Label.NoWrap
                color: "blue"
                font.pixelSize: 16
                font.italic: true
                text: "Select Raspberry Pi IP"
            }
            ComboBox {
                id: rasPiIpSelect
                currentIndex: -1
                model: ["192.168.1.80"]
                onActivated: {
                    for(var i = 0; i < 16; ++i) {
                        guiAI.piControl(i).setRaspberryPiIp(currentText)
                    }
                }
            }
        }
        Row {
            spacing: 5
            width: parent.width

            Label {
                width: parent.width / 3
                wrapMode: Label.NoWrap
                color: "blue"
                font.pixelSize: 16
                font.italic: true
                text: "Select remote destination"
            }
            ComboBox {
                id: piDestSelect
                currentIndex: -1
                model: ["/home/root", "/home"]
                onActivated: {
                    for(var i = 0; i < 16; ++i) {
                        guiAI.piControl(i).setRemoteDeployDestination(currentText)
                    }
                }
            }
        }
        Row {
            spacing: 5
            width: parent.width

            Label {
                width: parent.width / 3
                wrapMode: Label.NoWrap
                color: "blue"
                font.pixelSize: 16
                font.italic: true
                text: "Select file to deploy"
            }
            Button {
                id: browse
                text: "Browse"
                onClicked: fileDialog.visible = true
            }
            TextField {
                id: fileSelected
                text: "Select File..."
            }
        }
    }

    Rectangle {
        width: parent.width
        height: parent.height / 2
        anchors.top: options.bottom

        GridView {
            id: grid
            anchors.fill: parent
            flow: GridView.FlowTopToBottom

            cellWidth: parent.width / 4
            cellHeight: parent.height / 4

            model: 16
            delegate: Rectangle {
                id: baysDelegate
                width: grid.cellWidth
                height: grid.cellHeight
                border.color: "blue"
                radius: width / 9

                Column {
                    id: container
                    anchors.fill: parent
                    spacing: 3
                    Text { id: bayName; text: "Bay " + (index + 1); color: "blue"; font.bold: true; anchors.horizontalCenter: parent.horizontalCenter }
                    Text {
                        id: onlinesource;
                        text: guiAI.piControl(index).remoteConnection === true ? "ONLINE" : "OFFLINE"
                        color: guiAI.piControl(index).remoteConnection === true ? "green" : "red"
                        anchors.horizontalCenter: parent.horizontalCenter
                    }
                    Row {
                        id: actions
                        width: parent.width - 20
                        anchors.horizontalCenter: parent.horizontalCenter
                        spacing: 2
                        RoundButton {
                            id: deployButton
                            width: parent.width / 2
                            height: container.height - bayName.height - onlinesource.height - 10
                            text: "Deploy"
                            enabled: guiAI.piControl(index).updateStatus !== Enums.UPDATE_IN_PROGRESS
                                     && guiAI.piControl(index).remoteConnection !== false
                            onPressed: {
                                var temp = subnet + "." + (index + 1)
                                console.log("deploying to Pi via" + temp)
                                guiAI.piControl(index).processRequest(temp)
                                statusAnimation.start()
                            }
                        }
                        Rectangle {
                            width: parent.width / 2
                            height: container.height - bayName.height - onlinesource.height - 10
                            Image {
                                id: statusImage
                                anchors.centerIn: parent
                                anchors.fill: parent
                                fillMode: Image.PreserveAspectFit
                                property var statusImages: ["", "qrc:/status/images/inProgress.gif", "qrc:/status/images/pass.png", "qrc:/status/images/fail.png"]
                                source: statusImages[guiAI.piControl(index).updateStatus]

                                RotationAnimator {
                                    id: statusAnimation
                                    target: statusImage;
                                    from: 0;
                                    to: 360;
                                    duration: 2000;
                                    loops: Animation.Infinite
                                }

                                Connections {
                                    target: guiAI.piControl(index)
                                    onUpdateStatusChanged: {
                                        if (guiAI.piControl(index).updateStatus === Enums.UPDATE_FAILED ||
                                                guiAI.piControl(index).updateStatus === Enums.UPDATE_SUCCESS) {
                                            statusAnimation.duration = 10
                                            statusAnimation.loops = 1
                                            statusAnimation.restart()
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            focus: true
        }
    }

    FileDialog {
        id: fileDialog
        title: "Choose deployment file..."
        folder: shortcuts.home
        onAccepted: {
            console.log("File chosen: " + fileDialog.fileUrls)
            fileToDeploy = fileDialog.fileUrl
            var index = fileToDeploy.search("///")
            fileSelected.text = fileToDeploy.substring(index + 3, fileToDeploy.length)
            visible = false

            for(var i = 0; i < 16; ++i) {
                guiAI.piControl(i).setRemoteDeploySource(fileSelected.text)
            }
        }
        onRejected: {
            console.log("Canceled")
            visible = false
        }
        Component.onCompleted: visible = false
    }

    function reset() {
        networkSelect.currentIndex = -1
        rasPiIpSelect.currentIndex = -1
        piDestSelect.currentIndex = -1
    }
}
