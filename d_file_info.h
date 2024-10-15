/*!
 * \file d_file_info.h
 * \brief file for the definition of the class "DFileInfo"
 * \author doga
 * \date 2024-10-14
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
#ifndef D_FILE_INFO_H
#define D_FILE_INFO_H

#include <QDateTime>
#include <QObject>
#include <QSharedPointer>
#include <QVariantMap>

#define RN_FILE_NAME		"fileName"
#define RN_FILE_SIZE        "fileSize"
#define RN_IS_FOLDER        "isFolder"
#define RN_LAST_MDF_TIME    "lastModificationTime"

class DFileInfo;
typedef QSharedPointer<DFileInfo> pDFileInfo;

class DFileInfo : public QObject
{
    Q_OBJECT

    explicit            DFileInfo               (const QVariantMap & data);

public:
    static pDFileInfo   createFromSMBC          (bool              isFolder,
                                                 const QString &   fileName,
                                                 quint64           fileSize,
                                                 const QDateTime & lastModificationTime);

    QStringList			properties              () const    {	return m_data.keys();	}
    QVariant            property                (const QString & name) const;

    bool                isFolder                () const    {   return property(RN_IS_FOLDER    ).toBool();      }
    QString             getFileName             () const    {   return property(RN_FILE_NAME    ).toString();    }
    quint64             getFileSize             () const    {   return property(RN_FILE_SIZE    ).toULongLong(); }
    QDateTime           getLastModificationTime () const    {   return property(RN_LAST_MDF_TIME).toDateTime();  }

    void				clear			();

signals:
    void                sigDataChanged  ();

private:
    QVariantMap m_data;
};

#endif // D_FILE_INFO_H
