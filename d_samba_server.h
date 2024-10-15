/*!
 * \file d_samba_server.h
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
#ifndef D_SAMBA_SERVER_H
#define D_SAMBA_SERVER_H

#include "d_source.h"
#include <QObject>

#include "d_source.h"
#include <libsmbclient.h>
#include <QDebug>

// Structure pour stocker les informations d'authentification SMB
struct SmbAuthInfo
{
    QString username;
    QString password;
    QString domain;
};

// Classe DSambaServer héritant de la classe de base DSource
class DSambaServer : public DSource
{
public:
    explicit        DSambaServer        (const QString &     serverIP,
                                         const QString &     share,
                                         const SmbAuthInfo & authInfo);
    virtual         ~DSambaServer       ();

    // Implémentation des méthodes de DSource
    bool           connectSource        () override;    // Connecter à la source
    void           disconnectSource     () override; // Déconnecter de la source
    QList<QString> listDirectory        (const QString & directoryPath) override; // Lister les fichiers et répertoires

private:
    // Méthode d'initialisation du contexte SMB
    bool            initSmbContext      ();

    QString     m_serverIP;   // Adresse IP du serveur
    QString     m_share;      // Nom du partage Samba
    SmbAuthInfo m_authInfo;   // Informations d'authentification
    SMBCCTX *   m_smbContext; // Contexte de connexion SMB
};

#endif // D_SAMBA_SERVER_H
