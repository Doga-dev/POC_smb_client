import QtQuick 2.15
import QtQuick.Controls 2.15

import SambaLib 1.0

ApplicationWindow {
    id: rootWindow
    visible: true
    width: 1280
    height: 800
    title: qsTr("Samba Client Interface")

    Item {
        id: rootItem
        anchors.fill: parent
        anchors.margins: 10

        // Modèle pour les clients enregistrés
        ListModel {
            id: clientModel
        }

        Column {
            spacing: 10
            anchors.fill: parent

            // Ligne pour ajouter et supprimer des clients
            Row {
                spacing: 10

                Button {
                    text: "Ajouter Client"
                    onClicked: clientPopup.open()
                }

                Button {
                    text: "Supprimer Client"
                    enabled: clientComboBox.currentIndex >= 0
                    onClicked: {
                        clientModel.remove(clientComboBox.currentIndex);
                    }
                }

                Item {
                    width: 40
                    height: parent.height
                }

                TextField {
                    id: backupSourcePath;
                    enabled: clientComboBox.currentIndex >= 0
                    text: "/media/internal_storage/reports"
                }

                Button {
                    text: "Backup"
                    enabled: clientComboBox.currentIndex >= 0
                    onClicked: {
                        let contextId = clientComboBox.valueAt(clientComboBox.currentIndex);
                        let sourcePath = backupSourcePath.text;
                        let destPath = folderInput.text
                        console.log("Backup folder '" + sourcePath + "' in contextId " + contextId + ", subfoldder: " + destPath);
                        sambaClient.backupLocalFolder(contextId, destPath, sourcePath);
                    }
                }

                Item {
                    width: 40
                    height: parent.height
                }

                BusyIndicator {
                    height: parent.height
                    width: height
                    running: sambaClient.busy
                }
            }

            // Ligne pour sélectionner un client et lire un répertoire
            Row {
                spacing: 10
                ComboBox {
                    id: clientComboBox
                    width: 200
                    model: clientModel
                    textRole: "display"
                    valueRole: "contextId"
                    // displayText: model.display
                }

                TextField {
                    id: folderInput
                    placeholderText: "Sous-dossier à lire"
                    width: 200
                }

                Button {
                    text: "Lister les fichiers"
                    enabled: (clientComboBox.currentIndex >= 0)
                    onClicked: {
                        let contextId = clientModel.get(clientComboBox.currentIndex).contextId;
                        if (! contextId) {
                            contextId = clientComboBox.valueAt(clientComboBox.currentIndex);
                        }
                        console.log("List files of contextId " + contextId + ", subfoldder: " + folderInput.text);
                        sambaClient.listDirectory(contextId, folderInput.text);
                    }
                }
                Button {
                    text: "Lire un Fichier"
                    enabled: clientComboBox.currentIndex >= 0
                    onClicked: {
                        let contextId = clientComboBox.valueAt(clientComboBox.currentIndex);
                        let subFolder =folderInput.text;
                        let filePath = (subFolder.length > 0 ? (subFolder + "/") : "") + "test_file.txt";
                        console.log("Read file of contextId " + contextId + ", subFolder: '" + subFolder + "', filePath: '" + filePath + "'.");
                        sambaClient.readFile(contextId, filePath);
                    }
                }
                Button {
                    text: "Ajouter un Fichier"
                    enabled: clientComboBox.currentIndex >= 0
                    onClicked: {
                        let contextId = clientComboBox.valueAt(clientComboBox.currentIndex);
                        let subFolder =folderInput.text;
                        let filePath = (subFolder.length > 0 ? (subFolder + "/") : "") + "test_file.txt";
                        console.log("Read file of contextId " + contextId + ", subFolder: '" + subFolder + "', filePath: '" + filePath + "'.");
                        let fileContent = "Un peu de texte pour vérifier la bonne prise en compte des caractères spéciaux.\n" +
                                          "Mais également l'écriture sur plusieurs lignes.";
                        sambaClient.writeFile(contextId, filePath, fileContent);
                    }
                }
            }

            // Zone de texte pour afficher les résultats
            TextArea {
                id: outputArea
                readOnly: true
                width: parent.width
                height: parent.height - 150
            }
        }
    }

    // Popup pour ajouter un nouveau client
    Dialog {
        id: clientPopup
        modal: true
        title: "Ajouter un nouveau client"
        anchors.centerIn: parent
        width: 400
        height: 400

        Column {
            spacing: 10
            anchors.centerIn: parent

            TextField { id: addressInput;       placeholderText: "Adresse IP"               ; text: "192.168.1.81"}
            TextField { id: domainInput;        placeholderText: "Domaine (ex: WORKGROUP)"  ; text: "WORKGROUP" }
            TextField { id: usernameInput;      placeholderText: "Nom d'utilisateur"        ; text: "pi" }
            TextField { id: passwordInput;      placeholderText: "Mot de passe"             ; text: "dbd"; echoMode: TextInput.Password }
            TextField { id: sharedFolderInput;  placeholderText: "Dossier partagé"          ; text: "PiShare" }

            Button {
                text: "Ajouter"
                onClicked: {
                    let contextId = sambaClient.registerContext(
                        addressInput.text, domainInput.text, usernameInput.text,
                        passwordInput.text, sharedFolderInput.text
                    );
                    if (contextId >= 0) {
                        clientModel.append({contextId: contextId, display: addressInput.text});
                        clientPopup.close();
                    }
                }
            }
        }
    }

    // Liaison entre le client Qt et le QML
    DSambaClient {
        id: sambaClient
        onCommandSent       : function () {
            outputArea.text = "";
        }
        onDirectoryReady    : function (fullPath, fileList) {
            outputArea.text = "Directory '" + fullPath + "' content =\n" + fileList;
        }
        onFileContentReady  : function (fullPath, fileContent) {
            outputArea.text = "File '" + fullPath + "' content =\n" + fileContent;
        }
        onFileWritenDone    : function (fullPath, message) {
            outputArea.text = "Writen file '" + fullPath + "' size =\n" + message;
        }
        onBackupLocalDirProgress: function (fullPath, message) {
            outputArea.text = "Backup local folder to directory '" + fullPath + "', progress =\n" + message;
        }
        onBackupLocalDirDone: function (fullPath, message) {
            outputArea.text = "Backup local folder to directory '" + fullPath + "', number of files copied =\n" + message;
        }
        onErrorOccured      : function (message) {
            outputArea.text = message;
        }
    }
}
