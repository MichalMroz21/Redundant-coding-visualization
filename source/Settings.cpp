#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QLoggingCategory>
#include <QFile>
#include <QTextStream>
#include <QDir>

#include "Settings.hpp"

/**
 * @brief Create Settings object
 */
Settings::Settings(QObject *parent) : QObject{parent}{
    if (!this->settings->contains("Language")) {
        // Set default language here (English)
        this->settings->setValue("Language", "0");
    }
    this->getLanguage();
    if (this->language < 0 || this->language > Settings::availableLanguages::langNum-1){
        //prevent other values than allowed
        this->language = 0;
        this->settings->setValue("Language", "0");
        //program runs in a default language
    }
}

/**
 * @brief This function saves set language in configuration file
 * @param value set language to this
 */
void Settings::setLanguage(int value){
    if(value < 0 || value > Settings::availableLanguages::langNum-1){
        return;
    }
    if (value != this->language) {
        this->language = value;
        this->settings->setValue("Language", QString::number(value));
        emit this->langaugeChanged(value);
    }
}

/**
 * @brief Read the value of the language from the configuration file
 * @return Language from configuration file
 */
int Settings::getLanguage(){
    this->language = this->settings->value("Language").toInt();
    return this->language;
}

/**
 * @brief Refresh langugage between QML pages
 */
void Settings::refreshLanguage(){
    qDebug() << "Odświeżam język" << this->getLanguage();
    emit this->langaugeChanged(this->getLanguage());
}

/**
 * @brief Function which returns name of given language
 * @param language Value from Settings enum called "availableLanguages"
 * @return QString with specific language name
 */
QString Settings::availableLanguagesToString(int language){
    switch(language){
        case availableLanguages::en: return "en";
        case availableLanguages::pl: return "pl";
        default: throw std::invalid_argument("Unimplemented item");
    }
}

/**
 * @brief Load language translation for a given QML page
 * @param QMLpage Value of the QML page that text should be loaded
 */
void Settings::readFile(int QMLpage){

    int language = this->getLanguage();

    //filename depends on which language is choosen
    QString filename = "RedundantCoding/assets/lang/";
    filename += this->availableLanguagesToString(language);
    filename += ".lang";

    //root path
    QString executablePath = QCoreApplication::applicationDirPath();

    //root path + filename
    QString absolutePath = QDir(executablePath).absoluteFilePath(filename);

    //handle to file
    QFile file(absolutePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    //pages counter
    int currentPage = 0;
    //return variable with translation
    QString output = "";
    //temporary variable used in splitting text
    QStringList lineList;
    //input stream
    QTextStream in(&file);
    while (!in.atEnd()) {
        //read next line
        QString line = in.readLine();
        //if the line contains only new line character then it separates pages
        //readLine reads empty line ("") when it is a new line character only
        if(line == ""){
            currentPage += 1;
            if(currentPage > QMLpage) {
                if (output != ""){
                    //if there is any input then we can emit signal
                    emit this->loadedPageContent(output);
                }
                return; //no need to read further
            }

        }
        if(line != "" && currentPage == QMLpage){
            //text in the language file is in a form of dictionary so we need to extract it by splitting the input text
            lineList = line.split(u'\"');
            output += lineList[1];
            output += "\n";
        }
    }

    if (output != ""){
        emit this->loadedPageContent(output);
    }
    return;
}
