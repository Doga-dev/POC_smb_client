/*!
 * \file main1.cpp
 * \brief file for the definition of the class "%{Cpp:License:ClassName}"
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

#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include "d_samba_server.h"

int main1(int argc, char * argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
    QGuiApplication app(argc, argv);

    SmbAuthInfo auth;
    auth.username = "pi";
    auth.password = "dbd";
    auth.domain = "WORKGROUP";

    DSambaServer sambaSource("192.168.1.81", "PiShare", auth);
    if (sambaSource.connectSource()) {
        QList<QString> files = sambaSource.listDirectory("OF");
        qDebug() << "Fichiers dans 'Documents':" << files;
    }

    QQmlApplicationEngine engine;
    const QUrl            url(QStringLiteral("qrc:/main.qml"));
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreated,
        &app,
        [url](QObject * obj, const QUrl & objUrl) {
            if (! obj && url == objUrl)
                QCoreApplication::exit(-1);
        },
        Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}
