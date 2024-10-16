/*!
 * \file i_smb_client.h
 * \brief file for the definition of the interface "ISmbClient"
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
#ifndef I_SMB_CLIENT_H
#define I_SMB_CLIENT_H

#include <QList>
#include <QObject>
#include <QString>
#include <QVariantMap>

#include "d_file_info.h"


// Interface pour les clients de DSmbManager
class ISmbClient : public QObject {
    Q_OBJECT

public:
    explicit        ISmbClient              (QObject * parent = nullptr)
        : QObject(parent)
    {}
    virtual         ~ISmbClient             () {}

    virtual int     registerContext         (const QString & address,
                                             const QString & domain,
                                             const QString & username,
                                             const QString & password,
                                             const QString & sharedFolder) = 0;

public slots:
    // Méthodes virtuelles pures à implémenter par les classes clientes
    virtual void    onDirectoryListed       (int contextId, const QString & fullPath, const QList<pDFileInfo> & fileList) = 0;
    virtual void    onFileRead              (int contextId, const QString & fullPath, const QByteArray & content) = 0;
    virtual void    onFileWriten            (int contextId, const QString & fullPath, int contentSize) = 0;
    virtual void    onBackupLocalDirProgress(int contextId, const QString & fullPath, int progressPercent) = 0;
    virtual void    onBackupLocalDirDone    (int contextId, const QString & fullPath, int nbFilesCopied) = 0;
    virtual void    onError                 (int contextId, const QString & error) = 0;

signals:
    // Signaux émis par le client pour interagir avec le DSmbManager
    void            sigGetDirectoryFilesList(int contextId, const QString & relativePath);
    void            sigReadFile             (int contextId, const QString & relativePath);
    void            sigWriteFile            (int contextId, const QString & relativePath, const QByteArray & content);
    void            sigBackupLocalDirectory (int             contextId,
                                            const QString & relativeDestPath,
                                            const QString & sourcePath);

};

#endif // I_SMB_CLIENT_H
