#ifndef SETTINGS_HPP
#define SETTINGS_HPP

#include <QObject>
#include <QBitArray>
#include <QDebug>
#include <QSettings>

class Settings : public QObject{
    Q_OBJECT

    public:

        explicit Settings(QObject *parent = nullptr);

    public slots:
        void setLanguage(int value);
        int getLanguage();
        void refreshLanguage();

    signals:
        void langaugeChanged(int newLanguage);

    private:    
        int language = 1; //0 - English, 1 - Polish
        QSettings* settings = new QSettings("Redundant Coding", "App");
};

#endif // SETTINGS_HPP
