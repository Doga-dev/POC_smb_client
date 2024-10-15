

#include <libsmbclient.h>
#include <iostream>
#include <cstring>  // Pour `strncpy`

// Structure pour stocker les informations d'authentification SMB
struct SmbAuthInfo {
    std::string username;
    std::string password;
    std::string domain;
};

// Fonction de rappel pour l'authentification
void auth_callback(const char *srv, const char *shr, char *wg, int wglen, char *un, int unlen, char *pw, int pwlen) {
    // Informations d'identification
    const char *username = "pi";
    const char *password = "dbd";
    const char *workgroup = "WORKGROUP";

    // Remplir les champs avec les informations d'authentification
    if (wg && wglen > 0) {
        strncpy(wg, workgroup, wglen - 1);
        wg[wglen - 1] = '\0';  // Null-terminate
    }
    if (un && unlen > 0) {
        strncpy(un, username, unlen - 1);
        un[unlen - 1] = '\0';  // Null-terminate
    }
    if (pw && pwlen > 0) {
        strncpy(pw, password, pwlen - 1);
        pw[pwlen - 1] = '\0';  // Null-terminate
    }

    std::cout << "Authentification pour le serveur : " << srv << "\n";
    std::cout << "Partage : " << shr << "\n";
    std::cout << "Nom d'utilisateur : " << un << "\n";
    std::cout << "Mot de passe : " << pw << "\n";
    std::cout << "Domaine : " << wg << "\n";
}

// Initialisation du contexte SMB avec `smbc_init`
bool initializeSmbClient() {
    // Appeler `smbc_init` avec la fonction de rappel d'authentification
    if (smbc_init(auth_callback, 1) != 0) {
        std::cerr << "Erreur : Impossible d'initialiser le contexte SMB." << std::endl;
        return false;
    }

    std::cout << "Contexte SMB initialisé avec succès." << std::endl;
    return true;
}

int main2() {
    // Initialiser la bibliothèque SMB
    if (!initializeSmbClient()) {
        return 1;
    }
    // smbc_setDebug(1);

    // Chemin d'accès au répertoire distant SMB
    const char *url = "smb://192.168.1.81/PiShare/OF";

    // Ouvrir le répertoire distant avec `smbc_opendir`
    int dirHandle = smbc_opendir(url);
    if (dirHandle < 0) {
        std::cerr << "Erreur : Impossible d'ouvrir le répertoire SMB : " << url << std::endl;
        return 1;
    }

    std::cout << "Répertoire SMB ouvert avec succès : " << url << std::endl;

    // Lire le contenu du répertoire
    struct smbc_dirent *dirent;
    while ((dirent = smbc_readdir(dirHandle)) != nullptr) {
        std::cout << "Fichier : " << dirent->name << std::endl;
    }

    // Fermer le répertoire
    smbc_closedir(dirHandle);
    return 0;
}
