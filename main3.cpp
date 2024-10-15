#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "d_samba_client.h"
#include "d_smb_manager.h"

int main3(int argc, char *argv[]) {
    // Initialiser l'application Qt
    QGuiApplication app(argc, argv);

    // Créer le gestionnaire SMB (singleton)
    DSmbManager * smbManager = DSmbManager::instance();

    // Créer un client DSambaClient
    DSambaClient sambaClient;

    // Enregistrer la classe DSambaClient pour une utilisation directe dans QML
    qmlRegisterType<DSambaClient>("SambaLib", 1, 0, "DSambaClient");

    // Initialiser l'interface QML
    QQmlApplicationEngine engine;

    // Exposer l'instance de DSambaClient au contexte QML
    engine.rootContext()->setContextProperty("sambaClient", &sambaClient);

    // Charger le fichier QML principal
    const QUrl url(QStringLiteral("qrc:/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
                         if (!obj && url == objUrl)
                             QCoreApplication::exit(-1);
                     }, Qt::QueuedConnection);
    engine.load(url);

    // Exécuter l'application Qt
    return app.exec();
}
