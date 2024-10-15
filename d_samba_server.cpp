/*!
 * \file d_samba_server.cpp
 * \brief file for the definition of the class "DSambaServer"
 * \author doga
 * \date 2024-10-10
 *
 * \details
 *
 * \copyright
 ****************************************************************************
 *        Ce logiciel est la propriete de DOGA®.
 *         -------------------------------------
 *
 *    Il ne peut etre reproduit ni communique en totalite ou partie sans
 *    son autorisation ecrite.
 *
 ****************************************************************************
 *        This software is the property of DOGA®.
 *         -------------------------------------
 *
 *    It cannot be reproduced nor disclosed entirely or partially without
 *    a written agreement.
 *
 ****************************************************************************
 */

#include <cstring>  // Pour `strncpy`

#include "d_samba_server.h"

// Constructeur de DSambaServer
DSambaServer::DSambaServer(const QString &     serverIP,
                           const QString &     share,
                           const SmbAuthInfo & authInfo)
    : m_serverIP(serverIP)
    , m_share(share)
    , m_authInfo(authInfo)
    , m_smbContext(nullptr)
{}

// Destructeur de DSambaServer
DSambaServer::~DSambaServer() {
    disconnectSource();
}

// Callback d'authentification pour fournir les informations d'identification
void authCallback(SMBCCTX *    context,
                  const char * server,
                  const char * share,
                  char *       workgroup,
                  int          maxWorkgroup,
                  char *       username,
                  int          maxUsername,
                  char *       password,
                  int          maxPassword)
{
    SmbAuthInfo * auth = static_cast<SmbAuthInfo *>(smbc_getOptionUserData(context));
    if (auth) {
        strncpy(workgroup, auth->domain.toStdString().c_str(), maxWorkgroup - 1);
        strncpy(username, auth->username.toStdString().c_str(), maxUsername - 1);
        strncpy(password, auth->password.toStdString().c_str(), maxPassword - 1);
    }
}

// Méthode pour initialiser le contexte SMB
bool DSambaServer::initSmbContext() {
    m_smbContext = smbc_new_context();
    if (!m_smbContext) {
        qCritical() << "Erreur : Impossible de créer le contexte SMB.";
        return false;
    }

    // Configurer les informations d'authentification
    smbc_setOptionUserData(m_smbContext, &m_authInfo);
    smbc_setFunctionAuthDataWithContext(m_smbContext, authCallback);

    if (! smbc_init_context(m_smbContext)) {
        qCritical() << "Erreur : Initialisation du contexte SMB échouée.";
        smbc_free_context(m_smbContext, 1);
        return false;
    }
    return true;
}

// Connecter à la source Samba
bool DSambaServer::connectSource() {
    if (initSmbContext()) {
        qDebug() << "Connexion établie au serveur Samba:" << m_serverIP;
        return true;
    }
    return false;
}

// Déconnecter de la source
void DSambaServer::disconnectSource() {
    if (m_smbContext) {
        smbc_free_context(m_smbContext, 1);
        m_smbContext = nullptr;
        qDebug() << "Déconnexion du serveur Samba.";
    }
}

// Lister le contenu d'un répertoire
QList<QString> DSambaServer::listDirectory(const QString & directoryPath) {
    QList<QString> fileList;
    if (! m_smbContext) {
        qCritical() << "Erreur : Pas de connexion active au serveur Samba.";
        return fileList;
    }

    QString fullPath = QString("smb://%1/%2/%3").arg(m_serverIP, m_share, directoryPath);

    // Ouvrir le répertoire distant
    int dirHandle = smbc_opendir(fullPath.toStdString().c_str());
    if (dirHandle < 0) {
        qCritical() << "Erreur : Impossible d'ouvrir le répertoire SMB:" << fullPath;
        return fileList;
    }

    struct smbc_dirent * dirent;
    // Lire et lister les fichiers du répertoire
    while ((dirent = smbc_readdir(dirHandle)) != nullptr) {
        fileList.append(QString::fromUtf8(dirent->name));
    }

    // Fermer le répertoire
    smbc_closedir(dirHandle);
    return fileList;
}

