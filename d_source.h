/*!
 * \file d_source.h
 * \brief file for the definition of the class "DSource"
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
#ifndef D_SOURCE_H
#define D_SOURCE_H

#include <QString>
#include <QList>
#include <QVariant>

// Classe de base abstraite pour les sources de données
class DSource {
public:
    virtual ~DSource() = default;

    // Méthodes de gestion des sources de données
    virtual bool connectSource() = 0;  // Connecter à la source
    virtual void disconnectSource() = 0;  // Déconnecter de la source
    virtual QList<QString> listDirectory(const QString &directoryPath) = 0;  // Lister les fichiers et répertoires

protected:
    QString sourceName;  // Nom de la source
    QString alias;       // Alias de la source
};

#endif // D_SOURCE_H

