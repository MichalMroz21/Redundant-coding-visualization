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
        QString availableLanguagesToString(int language);

    public slots:
        void setLanguage(int value);
        int getLanguage();
        void refreshLanguage();
        void readFile(int QMLpage);

    signals:
        void langaugeChanged(int newLanguage);
        void loadedPageContent(QString pageContent);

    private:

        enum availableLanguages{
            en,         // 0 - English
            pl,         // 1 - Polish

            //add more languages here (above langNum)

            langNum     // number of available languages
        };
        int language = -1; //0 - English, 1 - Polish
        QSettings* settings = new QSettings("Redundant Coding", "App");
};

#endif // SETTINGS_HPP
