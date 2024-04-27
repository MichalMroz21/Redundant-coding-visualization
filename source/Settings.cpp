#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QLoggingCategory>

#include "Settings.hpp"

Settings::Settings(QObject *parent) : QObject{parent}{
    if (!this->settings->contains("Language")) {
        // Set default language here (English)
        this->settings->setValue("Language", "0");
    }
    this->getLanguage();
    if (this->language < 0 || this->language > 1){
        //prevent other values than allowed
        this->language = 0;
        this->settings->setValue("Language", "0");
        //program runs in a default language
    }
}

void Settings::setLanguage(int value){
    if(value < 0 || value > 1){
        return;
    }
    if (value != this->language) {
        this->language = value;
        this->settings->setValue("Language", QString::number(value));
        emit this->langaugeChanged(value);
    }
}

int Settings::getLanguage(){
    this->language = this->settings->value("Language").toInt();
    return this->language;
}

void Settings::refreshLanguage(){
    qDebug() << "Odświeżam język" << this->getLanguage();
    emit this->langaugeChanged(this->getLanguage());
}
