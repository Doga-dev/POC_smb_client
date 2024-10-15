/*!
 * \file d_samba_client.h
 * \brief file for the definition of the class "DSambaClient"
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
#ifndef D_SAMBA_CLIENT_H
#define D_SAMBA_CLIENT_H

#include <QObject>

#include "i_smb_client.h"

class DSambaClient : public ISmbClient {
    Q_OBJECT

    // Q_PROPERTY(QStringList filesList READ filesList NOTIFY directoryReady FINAL)
public:
    explicit        DSambaClient                (QObject * parent = nullptr);


    // Méthodes pour gérer les contextes
    Q_INVOKABLE int registerContext             (const QString & address,
                                                 const QString & domain,
                                                 const QString & username,
                                                 const QString & password,
                                                 const QString & sharedFolder) Q_DECL_OVERRIDE;

public slots:
    void            listDirectory               (int contextId, const QString & relativePath);
    void            readFile                    (int contextId, const QString & relativePath);
    void            writeFile                   (int contextId, const QString & relativePath, const QByteArray & content);
    void            backupLocalFolder           (int contextId, const QString & relativeDestPath, const QString & sourcePath);

    // Implémentation des méthodes virtuelles de ISmbClient
    void            onDirectoryListed           (int contextId, const QString & fullPath, const QList<pDFileInfo> & fileList) Q_DECL_OVERRIDE;
    void            onFileRead                  (int contextId, const QString & fullPath, const QByteArray & content) Q_DECL_OVERRIDE;
    void            onFileWriten                (int contextId, const QString & fullPath, int contentSize) Q_DECL_OVERRIDE;
    void            onBackupLocalDirDone        (int contextId, const QString & fullPath, int nbFilesCopied) Q_DECL_OVERRIDE;
    void            onError                     (int contextId, const QString & error) Q_DECL_OVERRIDE;

signals:
    void            commandSent                 ();
    void            directoryReady              (const QString & fullPath, const QString & fileList);
    void            fileContentReady            (const QString & fullPath, const QString & fileContent);
    void            fileWritenDone              (const QString & fullPath, const QString & message);
    void            backupLocalDirDone          (const QString & fullPath, const QString & message);
    void            errorOccured                (const QString & message);

private:
    QString         getDuration                 () const;

    QDateTime   m_cmdStartTime;
};

#endif // D_SAMBA_CLIENT_H
