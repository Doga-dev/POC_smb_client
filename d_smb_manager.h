/*!
 * \file d_smb_manager.h
 * \brief file for the definition of the class "DSmbManager"
 * \author doga
 * \date 2024-10-11
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

#ifndef D_SMB_MANAGER_H
#define D_SMB_MANAGER_H

#include <QDateTime>
#include <QMap>
#include <QMutex>
#include <QObject>
#include <QThread>

#include <libsmbclient.h>

#include "d_file_info.h"
#include "d_operation_queue.h"
#include "i_smb_client.h"

struct SmbAuthInfo {
    QString username;
    QString password;
    QString domain;
};

struct SmbContextInfo {
    SmbContextInfo(const QString & add, const QString & fld, const SmbAuthInfo & auth) {
        address      = add;
        sharedFolder = fld;
        authInfo     = auth;
    }
    QString     address;
    QString     sharedFolder;
    SmbAuthInfo authInfo;
};

// Classe DSmbManager (singleton)
class DSmbManager : public QObject
{
    Q_OBJECT

    explicit                DSmbManager             (); // Constructeur privé pour le singleton

public:
    static DSmbManager *    instance                (); // Méthode pour obtenir l'instance du singleton
    virtual                 ~DSmbManager            ();

    static void             declareClient           (ISmbClient * client);

    // Enregistrer un contexte avec les informations d'authentification
    int                     registerContext         (const QString &     address,
                                                     const SmbAuthInfo & authInfo,
                                                     const QString &     sharedFolder);

public slots:
    // Slot pour lister le contenu d'un répertoire
    void                    onListDirectory         (int contextId, const QString & relativePath);
    void                    onReadFile              (int contextId, const QString & relativePath);
    void                    onWriteFile             (int contextId, const QString & relativePath, const QByteArray & content);
    void                    onBackupLocalDirectory  (int contextId, const QString &relativeDestPath, const QString &sourcePath);

signals:
    // Signaux pour renvoyer les résultats
    void                    directoryListed         (int contextId, const QString & fullPath, const QList<pDFileInfo> & fileList);
    void                    fileRead                (int contextId, const QString & fullPath, const QByteArray & content);
    void                    fileWriten              (int contextId, const QString & fullPath, int contentSize);
    void                    backupLocalDirProgress  (int contextId, const QString & fullPath, int progressPercent);
    void                    backupLocalDirDone      (int contextId, const QString & fullPath, int nbFilesCopied);
    void                    errorOccurred           (int contextId, const QString & error);

private:
    // Méthode d'initialisation du contexte
    SMBCCTX *               initializeContext       (SmbContextInfo * contextInfo);

    pDFileInfo              getFileInfo             (bool isFolder, const struct libsmb_file_info * fileInfo);
    QString                 copyFileToSmb           (int contextId, const QString &localFilePath, const QString &destSmbPath);
    bool                    checkConnection         (int contextId);
    bool                    attemptReconnection     (int contextId, int maxAttempts, int delayMs);
    void                    resetError              ();
    void                    emitError               (int contextId, const QString & message);

    // Fonction de rappel pour l'authentification
    static void             authCallback            (SMBCCTX *    context,
                                                     const char * srv,
                                                     const char * shr,
                                                     char *       wg,
                                                     int          wglen,
                                                     char *       un,
                                                     int          unlen,
                                                     char *       pw,
                                                     int          pwlen);

    static DSmbManager *        m_instance;        // Instance unique du singleton

    QMap<int, SMBCCTX *>        m_contexts;        // Contexte SMB par identifiant
    QMutex                      m_mutex;           // Mutex pour protéger l'accès aux contextes
    int                         m_nextContextId;   // Identifiant de contexte suivant
    DOperationQueue             m_operationQueue;
    bool                        m_onError;

    static const int            c_operationTimeout = 3000;
    static const int            c_connectAttemptsNb = 3;
    static const int            c_connectAttemptDelay = 1000;   // ms
};

#endif // D_SMB_MANAGER_H

