/*!
 * \file d_file_info.cpp
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
#include "d_file_info.h"

DFileInfo::DFileInfo(const QVariantMap & data)
    : QObject{nullptr}
    , m_data(data) {}

pDFileInfo DFileInfo::createFromSMBC(bool              isFolder,
                                     const QString &   fileName,
                                     quint64           fileSize,
                                     const QDateTime & lastModificationTime)
{
    QVariantMap data{{RN_FILE_NAME    , fileName},
                     {RN_FILE_SIZE    , fileSize},
                     {RN_IS_FOLDER    , isFolder},
                     {RN_LAST_MDF_TIME, lastModificationTime}};
    pDFileInfo pointer = pDFileInfo(new DFileInfo(data));
    return pointer;
}

QVariant DFileInfo::property(const QString & name) const
{
    return m_data.value(name);
}

void DFileInfo::clear()
{
    m_data.clear();
    emit sigDataChanged();
}
