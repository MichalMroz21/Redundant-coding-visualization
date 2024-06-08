#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QLoggingCategory>

#include "CMakeConfig.hpp"
#include "DebugInterceptor.hpp"
#include "HammingCode.hpp"
#include "ReedSolomonCode.hpp"
#include "Settings.hpp"
#include <boost/regex.hpp>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    app.setQuitOnLastWindowClosed(true);

    auto debugInterceptor = DebugInterceptor::getInstance();
    auto hammingCode = QSharedPointer<HammingCode>(new HammingCode());
    auto reedSolomonCode = QSharedPointer<ReedSolomonCode>(new ReedSolomonCode());
    Settings settings;

    debugInterceptor.data()->disableDebug();
    debugInterceptor.data()->enableDebug();

    QQmlApplicationEngine engine;

    //adding constants to every .qml
    engine.rootContext()->setContextProperty("ROOT_PATH", ROOT_PATH);

    //adding objects to every .qml
    engine.rootContext()->setContextProperty("hammingCode", hammingCode.data());
    engine.rootContext()->setContextProperty("reedSolomonCode", reedSolomonCode.data());

    //adding settings to every .qml
    engine.rootContext()->setContextProperty("Settings", &settings);

    const QUrl url(u"qrc:/RedundantCoding/source_gui/Main.qml"_qs);

    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
        &app, []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.load(url);

    emit settings.readFile(0);

    int value = app.exec();
    // ze względu na QtConcurrent::run bez tego po zamknięciu aplikacji zostają wątki, które nigdy się nie kończą i zostaje proces zombie w systemie
    while (!hammingCode->isFinished())
        hammingCode->quit();
    while (!reedSolomonCode->isFinished())
        reedSolomonCode->quit();
    return value;
}
