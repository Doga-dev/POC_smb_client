/*!
 * \file d_samba_client.cpp
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
#include "d_samba_client.h"
#include "d_smb_manager.h"
#include <QDebug>
#include <QDir>

DSambaClient::DSambaClient(QObject * parent)
    : ISmbClient(parent)
{
    DSmbManager::declareClient(this);
}

int DSambaClient::registerContext(const QString & address,
                                  const QString & domain,
                                  const QString & username,
                                  const QString & password,
                                  const QString & sharedFolder)
{
    // Envoyer une demande d'enregistrement à DSmbManager
    DSmbManager * manager = DSmbManager::instance();
    return manager->registerContext(address, {username, password, domain}, sharedFolder);
}

void DSambaClient::listDirectory(int contextId, const QString & relativePath)
{
    m_cmdStartTime = QDateTime::currentDateTime();
    qDebug() << Q_FUNC_INFO << ": contextId =" << contextId << ", relativePath =" << relativePath;
    emit sigGetDirectoryFilesList(contextId, relativePath);
    emit commandSent();
}

void DSambaClient::readFile(int contextId, const QString & relativePath)
{
    m_cmdStartTime = QDateTime::currentDateTime();
    qDebug() << Q_FUNC_INFO << ": contextId =" << contextId << ", relativePath =" << relativePath;
    emit sigReadFile(contextId, relativePath);
    emit commandSent();
}

void DSambaClient::writeFile(int contextId, const QString & relativePath, const QByteArray & content)
{
    m_cmdStartTime = QDateTime::currentDateTime();
    qDebug() << Q_FUNC_INFO << ": contextId =" << contextId << ", relativePath =" << relativePath << ", content =" << content;
    emit sigWriteFile(contextId, relativePath, content);
    emit commandSent();
}

void DSambaClient::backupLocalFolder(int             contextId,
                                     const QString & relativeDestPath,
                                     const QString & sourcePath)
{
    m_cmdStartTime = QDateTime::currentDateTime();
    qDebug() << Q_FUNC_INFO << ": contextId =" << contextId << ", relativeDestPath =" << relativeDestPath << ", sourcePath =" << sourcePath;
    emit sigBackupLocalDirectory(contextId, relativeDestPath, sourcePath);
    emit commandSent();
}

void DSambaClient::onDirectoryListed(int contextId, const QString & fullPath, const QList<pDFileInfo> & fileList)
{
    QString formattedList;
    for (auto iter = fileList.begin(); iter != fileList.end(); ++iter) {
        pDFileInfo fileInfo = *iter;
        formattedList += fileInfo->getLastModificationTime().toString("yyyy-MM-dd HH:mm:ss.zzz")
                         + " | " + (fileInfo->isFolder() ? "D" : "F") + " | "
                         + QString("%1").arg(fileInfo->getFileSize(), 8) + " Bytes" + " | "
                         + fileInfo->getFileName() + "\n";
    }
    emit directoryReady(fullPath, formattedList + getDuration());
}

void DSambaClient::onFileRead(int contextId, const QString & fullPath, const QByteArray & content)
{
    emit fileContentReady(fullPath, QString(content) + getDuration());
}

void DSambaClient::onFileWriten(int contextId, const QString & fullPath, int contentSize)
{
    emit fileWritenDone(fullPath, QString("%1 bytes").arg(contentSize) + getDuration());
}

void DSambaClient::onBackupLocalDirDone(int contextId, const QString & fullPath, int nbFilesCopied)
{
    emit backupLocalDirDone(fullPath, QString("%1 files").arg(nbFilesCopied) + getDuration());
}

void DSambaClient::onError(int contextId, const QString & error)
{
    QString message = "Erreur pour le contexte " + QString::number(contextId) + ": " + error;
    qCritical() << message;
    emit errorOccured(message + getDuration());
}

QString DSambaClient::getDuration() const
{
    QString str("\n\n  duration = %1 ms");
    return str.arg(m_cmdStartTime.msecsTo(QDateTime::currentDateTime()));
}
