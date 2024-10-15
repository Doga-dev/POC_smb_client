/*!
 * \file %{Cpp:License:FileName}
 * \brief file for the definition of the class "%{Cpp:License:ClassName}"
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
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "d_samba_client.h"

int main(int argc, char * argv[])
{
    // Initialiser l'application Qt
    QGuiApplication app(argc, argv);

    // Créer un client DSambaClient
    DSambaClient sambaClient;

    // Enregistrer la classe DSambaClient pour une utilisation directe dans QML
    qmlRegisterType<DSambaClient>("SambaLib", 1, 0, "DSambaClient");

    // Initialiser l'interface QML
    QQmlApplicationEngine engine;

    // Exposer l'instance de DSambaClient au contexte QML
    engine.rootContext()->setContextProperty("sambaClient", & sambaClient);

    // Charger le fichier QML principal
    const QUrl url(QStringLiteral("qrc:/main.qml"));
    QObject::connect(
        & engine,
        & QQmlApplicationEngine::objectCreated,
        & app,
        [url](QObject * obj, const QUrl & objUrl) {
            if ((! obj) && url == objUrl)
                QCoreApplication::exit(-1);
        },
        Qt::QueuedConnection);
    engine.load(url);

    // Exécuter l'application Qt
    return app.exec();
}
