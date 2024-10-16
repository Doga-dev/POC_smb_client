/*!
 * \file d_smb_manager.cpp
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

#include <QDebug>
#include <QDirIterator>
#include <QFile>

#include "d_operation_with_timeout.h"
#include "d_smb_manager.h"

// Initialisation du singleton
DSmbManager * DSmbManager::m_instance = nullptr;

DSmbManager * DSmbManager::instance()
{
    if (! m_instance) {
        m_instance = new DSmbManager();
    }
    return m_instance;
}

// Constructeur privé
DSmbManager::DSmbManager()
    : QObject()
    , m_contexts()
    // , m_contextInfoMap()
    , m_mutex()
    , m_nextContextId(1)
{
    // Configuration initiale du gestionnaire
}

// Destructeur
DSmbManager::~DSmbManager()
{
    // Libérer tous les contextes SMB
    QMutexLocker locker(&m_mutex);
    for (auto iter = m_contexts.begin(); iter != m_contexts.end(); ++iter) {
        SMBCCTX * ctx = iter.value();
        if (ctx) {
            SmbContextInfo * contextInfo = static_cast<SmbContextInfo *>(smbc_getOptionUserData(ctx));
            if (contextInfo) {
                free(contextInfo);
            }
        }
        smbc_free_context(ctx, 1);
    }
}

void DSmbManager::declareClient(ISmbClient * client)
{
    DSmbManager * manager = DSmbManager::instance();

    QObject::connect(manager, & DSmbManager::directoryListed       , client, & ISmbClient::onDirectoryListed       , Qt::QueuedConnection);
    QObject::connect(manager, & DSmbManager::fileRead              , client, & ISmbClient::onFileRead              , Qt::QueuedConnection);
    QObject::connect(manager, & DSmbManager::fileWriten            , client, & ISmbClient::onFileWriten            , Qt::QueuedConnection);
    QObject::connect(manager, & DSmbManager::backupLocalDirProgress, client, & ISmbClient::onBackupLocalDirProgress, Qt::QueuedConnection);
    QObject::connect(manager, & DSmbManager::backupLocalDirDone    , client, & ISmbClient::onBackupLocalDirDone    , Qt::QueuedConnection);
    QObject::connect(manager, & DSmbManager::errorOccurred         , client, & ISmbClient::onError                 , Qt::QueuedConnection);

    QObject::connect(client, & ISmbClient::sigGetDirectoryFilesList, manager, & DSmbManager::onListDirectory       , Qt::QueuedConnection);
    QObject::connect(client, & ISmbClient::sigReadFile             , manager, & DSmbManager::onReadFile            , Qt::QueuedConnection);
    QObject::connect(client, & ISmbClient::sigWriteFile            , manager, & DSmbManager::onWriteFile           , Qt::QueuedConnection);
    QObject::connect(client, & ISmbClient::sigBackupLocalDirectory , manager, & DSmbManager::onBackupLocalDirectory, Qt::QueuedConnection);
}

// Enregistrer un nouveau contexte
int DSmbManager::registerContext(const QString &     address,
                                 const SmbAuthInfo & authInfo,
                                 const QString &     sharedFolder)
{
    QMutexLocker locker(&m_mutex);

    // Initialiser un nouveau contexte SMB
    SmbContextInfo * contextInfo = new SmbContextInfo(address, sharedFolder, authInfo);
    SMBCCTX * ctx = initializeContext(contextInfo);
    if (! ctx) {
        emit errorOccurred(-1, "Erreur : Impossible de créer le contexte SMB.");
        return -1;
    }

    // Associer le contexte à un nouvel ID
    int contextId               = m_nextContextId++;
    m_contexts[contextId]       = ctx;
    // m_contextInfoMap[contextId] = contextInfo;

    qDebug() << "Contexte enregistré avec succès pour le serveur : " << address;
    return contextId;
}

// Initialiser un contexte SMB
SMBCCTX * DSmbManager::initializeContext(SmbContextInfo * contextInfo)
{
    SMBCCTX * context = smbc_new_context();
    if (! context) {
        qCritical() << "Erreur : Impossible de créer le contexte SMB.";
        return nullptr;
    }

    smbc_setOptionUserData(context, (void *) contextInfo);
    smbc_setFunctionAuthDataWithContext(context, authCallback);

    if (! smbc_init_context(context)) {
        smbc_free_context(context, 1);
        qCritical() << "Erreur : Initialisation du contexte échouée.";
        return nullptr;
    }
    return context;
}

pDFileInfo DSmbManager::getFileInfo(bool isFolder, const libsmb_file_info * fileInfo)
{
    pDFileInfo pointer;
    if (fileInfo) {
        QString fileName = fileInfo->name;
        quint64 fileSize = fileInfo->size;
        qint64 sec  = static_cast<qint64>(fileInfo->mtime_ts.tv_sec);
        qint64 nsec = static_cast<qint64>(fileInfo->mtime_ts.tv_nsec);
        QDateTime lastModificationTime = QDateTime::fromSecsSinceEpoch(sec, Qt::UTC);
        lastModificationTime = lastModificationTime.addMSecs(nsec / 1000000);
        pointer = DFileInfo::createFromSMBC(isFolder, fileName, fileSize, lastModificationTime);
    }
    return pointer;
}

QString DSmbManager::copyFileToSmb(int             contextId,
                                   const QString & localFilePath,
                                   const QString & destSmbPath)
{
    QString errMsg;
    QFile localFile(localFilePath);
    if (localFile.open(QIODevice::ReadOnly)) {
        // Ouvrir le fichier distant pour écriture
        int fileHandle = smbc_open(destSmbPath.toStdString().c_str(), O_WRONLY | O_CREAT, 0666);
        if (fileHandle >= 0) {
            // Lire et écrire par blocs de 4 ko max
            char buffer[4096];
            ssize_t bytesRead;
            while ((bytesRead = localFile.read(buffer, sizeof(buffer))) > 0) {
                ssize_t bytesWritten = smbc_write(fileHandle, buffer, bytesRead);
                if (bytesWritten != bytesRead) {
                    errMsg = "Erreur : Échec de l'écriture sur le fichier distant : " + destSmbPath;
                    break;
                }
            }
            smbc_close(fileHandle);

            if (errMsg.isEmpty()) {
                // Vérification de la taille du fichier copié
                struct stat smbFileStat;
                if (smbc_stat(destSmbPath.toStdString().c_str(), & smbFileStat) == 0) {
                    if (smbFileStat.st_size != localFile.size()) {
                        errMsg = "Erreur : Taille de fichier incorrecte pour : " + destSmbPath;
                    }
                } else {
                    errMsg = "Erreur : Impossible de vérifier le fichier distant : " + destSmbPath;
                }
            }
        } else {
            errMsg = "Erreur : Impossible de créer ou ouvrir le fichier distant : " + destSmbPath;
        }
        localFile.close();
    } else {
        errMsg = "Erreur : Impossible d'ouvrir le fichier local : " + localFilePath;
    }
    return errMsg;
}

bool DSmbManager::checkConnection(int contextId)
{
    QMutexLocker locker(& m_mutex);
    SMBCCTX * context = m_contexts.value(contextId, nullptr);
    if (context) {
        // Tentez une opération simple, comme ouvrir le répertoire racine
        smbc_set_context(context);
        SmbContextInfo * contextInfo = static_cast<SmbContextInfo *>(smbc_getOptionUserData(context));
        QString testPath = QString("smb://%1/%2").arg(contextInfo->address, contextInfo->sharedFolder);

        int dirHandle = smbc_opendir(testPath.toStdString().c_str());
        if (dirHandle >= 0) {
            smbc_closedir(dirHandle);
            return true;
        }
    }
    return false;
}

bool DSmbManager::attemptReconnection(int contextId, int maxAttempts, int delayMs)
{
    for (int attempt = 0; attempt < maxAttempts; ++attempt) {
        if (checkConnection(contextId)) {
            return true;
        }
        QThread::msleep(delayMs);
    }
    return false;
}

void DSmbManager::resetError()
{
    m_onError = false;
}

void DSmbManager::emitError(int contextId, const QString & message)
{
    m_onError = true;
    emit errorOccurred(contextId, message);
}

// Callback pour l'authentification
void DSmbManager::authCallback(SMBCCTX *    context,
                               const char * srv,
                               const char * shr,
                               char *       wg,
                               int          wglen,
                               char *       un,
                               int          unlen,
                               char *       pw,
                               int          pwlen)
{
    SmbContextInfo * contextInfo = static_cast<SmbContextInfo *>(smbc_getOptionUserData(context));
    if (contextInfo) {
        SmbAuthInfo & auth = contextInfo->authInfo;
        strncpy(wg, auth.domain  .toStdString().c_str(), wglen - 1);
        strncpy(un, auth.username.toStdString().c_str(), unlen - 1);
        strncpy(pw, auth.password.toStdString().c_str(), pwlen - 1);
    }
}

// Méthode pour lister le contenu d'un répertoire
void DSmbManager::onListDirectory(int contextId, const QString & relativePath)
{
    resetError();
    auto operation = [this, contextId, relativePath](std::function<void()> resetTimet) {
        if (! checkConnection(contextId)) {
            if (attemptReconnection(contextId, c_connectAttemptsNb, c_connectAttemptDelay)) {
                qDebug() << "Reconnexion réussie pour le contexte" << contextId;
            } else {
                emit errorOccurred(contextId, "Impossible de se reconnecter au serveur SMB");
                return;
            }
        }

        QMutexLocker locker(& m_mutex);
        QString      errMsg;
        // Récupérer la liste des fichiers du répertoire en utilisant le contexte correct
        SMBCCTX *    context = m_contexts.value(contextId, nullptr);
        if (context) {
            SmbContextInfo * contextInfo = static_cast<SmbContextInfo *>(
                smbc_getOptionUserData(context));
            if (contextInfo) {
                QString fullPath = QString("smb://%1/%2/%3")
                                       .arg(contextInfo->address,
                                            contextInfo->sharedFolder,
                                            relativePath);
                smbc_set_context(context);

                // Ouvrir le répertoire distant
                int dirHandle = smbc_opendir(fullPath.toStdString().c_str());
                if (dirHandle < 0) {
                    errMsg = "Erreur : Impossible d'ouvrir le répertoire SMB: " + fullPath;
                } else {
                    QList<pDFileInfo> fileList;

                    struct stat                     fileStat;
                    const struct libsmb_file_info * fi = nullptr;
                    // Lire et lister les fichiers du répertoire
                    while ((fi = smbc_readdirplus2(dirHandle, &fileStat)) != nullptr) {
                        bool    isDir  = S_ISDIR(fileStat.st_mode);
                        bool    isFile = S_ISREG(fileStat.st_mode);
                        bool    isLink = S_ISLNK(fileStat.st_mode);
                        QString name   = fi->name;
                        qDebug() << Q_FUNC_INFO << "file =" << name << ", isDir =" << isDir
                                 << ", isFile =" << isFile << ", isLink =" << isLink
                                 << ", size =" << fi->size;
                        if ((name != ".") && (name != "..") && (! isLink)) {
                            // Exclure les dossiers "." et ".."
                            pDFileInfo fileInfo = getFileInfo(isDir, fi);
                            if (fileInfo) {
                                fileList.append(fileInfo);
                            }
                        }
                        if (m_onError) {
                            return;
                        } else {
                            resetTimet();
                        }
                    }

                    // Fermer le répertoire
                    smbc_closedir(dirHandle);

                    emit directoryListed(contextId, fullPath, fileList);
                    return;
                }
            } else {
                errMsg = "Erreur : Echec de la récupération des données du contexte";
            }
        } else {
            errMsg = "Erreur : Echec de la récupération du contexte samba";
        }
        emit errorOccurred(contextId, errMsg);
    };

    auto * operationWithTimeout = new DOperationWithTimeout(operation, c_operationTimeout, this);
    connect(operationWithTimeout, & DOperationWithTimeout::operationCompleted, this, [this, operationWithTimeout]() {
        operationWithTimeout->deleteLater();
    });
    connect(operationWithTimeout, & DOperationWithTimeout::timeoutOccurred, this, [this, contextId, relativePath, operationWithTimeout]() {
        emitError(contextId, "Opération timeout: Listing du répertoire");
        operationWithTimeout->deleteLater();

        // Mettre l'opération en file d'attente pour réessayer plus tard
        m_operationQueue.enqueue([this, contextId, relativePath]() {
            onListDirectory(contextId, relativePath);
        });
    });
}

void DSmbManager::onReadFile(int contextId, const QString & relativePath)
{
    resetError();
    auto operation = [this, contextId, relativePath](std::function<void()> resetTimet) {
        if (! checkConnection(contextId)) {
            if (attemptReconnection(contextId, c_connectAttemptsNb, c_connectAttemptDelay)) {
                qDebug() << "Reconnexion réussie pour le contexte" << contextId;
            } else {
                emit errorOccurred(contextId, "Impossible de se reconnecter au serveur SMB");
                return;
            }
        }

        QMutexLocker locker(& m_mutex);
        QString errMsg;
        SMBCCTX * context = m_contexts.value(contextId, nullptr);
        if (context) {
            SmbContextInfo * contextInfo = static_cast<SmbContextInfo *>(smbc_getOptionUserData(context));
            if (contextInfo) {
                QString fullPath = QString("smb://%1/%2/%3").arg(contextInfo->address,
                                                                 contextInfo->sharedFolder,
                                                                 relativePath);
                smbc_set_context(context);

                // ouvrir le fichier
                int fileHandle = smbc_open(fullPath.toStdString().c_str(), O_RDONLY, 0);
                if (fileHandle >= 0) {
                    struct stat fileStat;
                    if (smbc_fstat(fileHandle, & fileStat) == 0) {
                        void * buff = calloc(sizeof(char), fileStat.st_blksize);
                        ssize_t bytesRead = smbc_read(fileHandle, buff, fileStat.st_blksize);
                        if (bytesRead >= fileStat.st_size) {
                            emit fileRead(contextId, fullPath, QByteArray(static_cast<const char *>(buff), bytesRead + 1));
                            return;
                        } else {
                            errMsg = "Erreur : Echec de lecture du fichier '" + fullPath + "', " + QString::number(bytesRead) + " octets lus sur " + QString ::number(fileStat.st_size) + " octets attendus.";
                        }
                    } else {
                        errMsg = "Erreur : Echec de la récupération des informations sur le fichier '" + fullPath + "'.";
                    }
                    if (smbc_close(fileHandle) < 0) {
                        errMsg = "Erreur : Echec de la fermeture du fichier '" + fullPath + "'.";
                    }
                } else {
                    errMsg = "Erreur : Echec de l'ouverture du fichier '" + fullPath + "'.";
                }
            } else {
                errMsg = "Erreur : Echec de la récupération des données du contexte";
            }
        } else {
            errMsg = "Erreur : Echec de la récupération du contexte samba";
        }
        emit errorOccurred(contextId, errMsg);
    };

    auto * operationWithTimeout = new DOperationWithTimeout(operation, c_operationTimeout, this);
    connect(operationWithTimeout, & DOperationWithTimeout::operationCompleted, this, [this, operationWithTimeout]() {
        operationWithTimeout->deleteLater();
    });
    connect(operationWithTimeout, & DOperationWithTimeout::timeoutOccurred, this, [this, contextId, relativePath, operationWithTimeout]() {
        emitError(contextId, "Opération timeout: Lecture du fichier");
        operationWithTimeout->deleteLater();
    });
}

void DSmbManager::onWriteFile(int                contextId,
                              const QString &    relativePath,
                              const QByteArray & content)
{
    resetError();
    auto operation = [this, contextId, relativePath, content](
                         std::function<void()> resetTimer) {
        if (! checkConnection(contextId)) {
            if (attemptReconnection(contextId, c_connectAttemptsNb, c_connectAttemptDelay)) {
                qDebug() << "Reconnexion réussie pour le contexte" << contextId;
            } else {
                emit errorOccurred(contextId, "Impossible de se reconnecter au serveur SMB");
                return;
            }
        }

        QMutexLocker locker(& m_mutex);
        QString      errMsg;
        SMBCCTX *    context = m_contexts.value(contextId, nullptr);
        if (context) {
            SmbContextInfo * contextInfo = static_cast<SmbContextInfo *>(
                smbc_getOptionUserData(context));
            if (contextInfo) {
                QString fullPath = QString("smb://%1/%2/%3")
                                       .arg(contextInfo->address,
                                            contextInfo->sharedFolder,
                                            relativePath);
                smbc_set_context(context);

                // ouvrir le fichier
                int fileHandle = smbc_open(fullPath.toStdString().c_str(), O_WRONLY | O_CREAT, 0);
                if (fileHandle >= 0) {
                    ssize_t bytesWriten = smbc_write(fileHandle,
                                                     content.toStdString().c_str(),
                                                     content.size());
                    if (bytesWriten >= content.size()) {
                        emit fileWriten(contextId, fullPath, content.size());
                        return;
                    } else {
                        errMsg = "Erreur : Echec de l'écriture du fichier '" + fullPath + "', "
                                 + QString::number(bytesWriten) + " octets écrits sur "
                                 + QString ::number(content.size()) + " octets voulus.";
                    }
                    if (smbc_close(fileHandle) < 0) {
                        errMsg = "Erreur : Echec de la fermeture du fichier '" + fullPath + "'.";
                    }
                } else {
                    errMsg = "Erreur : Echec de l'ouverture du fichier '" + fullPath + "'.";
                }
            } else {
                errMsg = "Errur : Echec de la récupération des données du contexte";
            }
        } else {
            errMsg = "Errur : Echec de la récupération du contexte samba";
        }
        emit errorOccurred(contextId, errMsg);
    };

    auto * operationWithTimeout = new DOperationWithTimeout(operation, c_operationTimeout, this);
    connect(operationWithTimeout, & DOperationWithTimeout::operationCompleted, this, [this, operationWithTimeout]() {
        operationWithTimeout->deleteLater();
    });
    connect(operationWithTimeout, & DOperationWithTimeout::timeoutOccurred, this, [this, contextId, relativePath, content, operationWithTimeout]() {
        emitError(contextId, "Opération timeout: Ecriture du fichier");
        operationWithTimeout->deleteLater();
    });
}

void DSmbManager::onBackupLocalDirectory(int             contextId,
                                         const QString & relativeDestPath,
                                         const QString & sourcePath)
{
    resetError();
    auto operation = [this, contextId, relativeDestPath, sourcePath](
                         std::function<void()> resetTimer) {
        if (! checkConnection(contextId)) {
            if (attemptReconnection(contextId, c_connectAttemptsNb, c_connectAttemptDelay)) {
                qDebug() << "Reconnexion réussie pour le contexte" << contextId;
            } else {
                emit errorOccurred(contextId, "Impossible de se reconnecter au serveur SMB");
                return;
            }
        }

        QMutexLocker locker(& m_mutex);
        QString      errMsg;
        SMBCCTX *    context = m_contexts.value(contextId, nullptr);
        if (context) {
            SmbContextInfo * contextInfo = static_cast<SmbContextInfo *>(
                smbc_getOptionUserData(context));
            if (contextInfo) {
                // Créer l'URL complet du répertoire distant
                QString destBaseUrl = QString("smb://%1/%2/%3")
                                          .arg(contextInfo->address,
                                               contextInfo->sharedFolder,
                                               relativeDestPath);
                smbc_set_context(context);

                int filesCopied = 0;
                int totalFiles  = 0;
                {
                    QDirIterator it(sourcePath,
                                    QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks,
                                    QDirIterator::Subdirectories);
                    // Compter le nombre total de fichiers
                    while (it.hasNext()) {
                        it.next();
                        if (it.fileInfo().isFile()) {
                            totalFiles++;
                        }
                    }
                }
                {
                    // Réinitialiser l'itérateur
                    QDirIterator it(sourcePath,
                                    QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks,
                                    QDirIterator::Subdirectories);

                    while (it.hasNext()) {
                        it.next();
                        QString localPath    = it.filePath();
                        QString relativePath = it.filePath().mid(
                            sourcePath.length() + 1); // Chemin relatif par rapport au sourcePath
                        QString destPath = destBaseUrl + "/" + relativePath;

                        if (it.fileInfo().isDir()) {
                            // Créer le répertoire distant
                            if (smbc_mkdir(destPath.toStdString().c_str(), 0777) != 0) {
                                if (errno != EEXIST) { // Si le répertoire n'existe pas, le créer
                                    errMsg = "Erreur : Impossible de créer le répertoire distant : "
                                             + destPath;
                                    break;
                                }
                                qDebug() << Q_FUNC_INFO << ": context =" << contextId
                                         << ", make directory:" << destPath;
                            }
                        } else if (it.fileInfo().isFile()) {
                            // Copier le fichier local vers le fichier distant
                            errMsg = copyFileToSmb(contextId, localPath, destPath);
                            if (errMsg.isEmpty()) {
                                ++filesCopied;
                                emit backupLocalDirProgress(contextId, destBaseUrl, (qRound(100.0 * filesCopied / totalFiles)));
                                // Réinitialiser le timer
                                resetTimer();
                                qDebug() << Q_FUNC_INFO << ": context =" << contextId
                                         << ", files copied =" << QString("%1 / %2").arg(filesCopied, QString::number(totalFiles).length()).arg(totalFiles)
                                         << ", sourcePath =" << localPath << ", destPath =" << destPath;
                            } else {
                                break;
                            }
                        } else {
                            qDebug() << Q_FUNC_INFO << ": context =" << contextId
                                     << ",   unmanaged item:" << localPath;
                        }
                        if (m_onError) {
                            return;
                        }
                    }
                }
                if (errMsg.isEmpty()) {
                    emit backupLocalDirDone(contextId, destBaseUrl, filesCopied);
                    return;
                }
            } else {
                errMsg = "Errur : Echec de la récupération des données du contexte";
            }
        } else {
            errMsg = "Errur : Echec de la récupération du contexte samba";
        }
        emit errorOccurred(contextId, errMsg);
    };

    auto * operationWithTimeout = new DOperationWithTimeout(operation, c_operationTimeout, this);
    connect(operationWithTimeout, & DOperationWithTimeout::operationCompleted, this, [this, operationWithTimeout]() {
        operationWithTimeout->deleteLater();
    });
    connect(operationWithTimeout, & DOperationWithTimeout::timeoutOccurred, this, [this, contextId, relativeDestPath, sourcePath, operationWithTimeout]() {
        emitError(contextId, "Opération timeout: Backup du répertoire");
        operationWithTimeout->deleteLater();

        // // Mettre l'opération en file d'attente pour réessayer plus tard
        // m_operationQueue.enqueue([this, contextId, relativeDestPath, sourcePath]() {
        //     onBackupLocalDirectory(contextId, relativeDestPath, sourcePath);
        // });
    });
}
