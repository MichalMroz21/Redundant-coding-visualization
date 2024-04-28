#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QLoggingCategory>

#include "CMakeConfig.hpp"
#include "DebugInterceptor.hpp"
#include "HammingCode.hpp"
#include "Settings.hpp"
#include <boost/regex.hpp>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    auto debugInterceptor = DebugInterceptor::getInstance();
    auto hammingCode = QSharedPointer<HammingCode>(new HammingCode());
    Settings settings;

    debugInterceptor.data()->disableDebug();
    debugInterceptor.data()->enableDebug();

    QQmlApplicationEngine engine;

    //adding constants to every .qml
    engine.rootContext()->setContextProperty("ROOT_PATH", ROOT_PATH);

    //adding objects to every .qml
    engine.rootContext()->setContextProperty("hammingCode", hammingCode.data());

    //adding settings to every .qml
    engine.rootContext()->setContextProperty("Settings", &settings);

    const QUrl url(u"qrc:/RedundantCoding/source_gui/Main.qml"_qs);

    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
        &app, []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.load(url);

    emit settings.langaugeChanged(settings.getLanguage());

    return app.exec();
}
