#include "ReedSolomonCode.hpp"

#include <QtConcurrent>
#include <QThread>
#include <QtMath>

ReedSolomonCode::ReedSolomonCode(QObject *parent) : QObject{parent}{}

void ReedSolomonCode::setInitialData(QString data, int animationDelay, bool infiniteWait)
{
    assert(data.length() == numData);
    this->animationDelayMs = animationDelay;
    this->infiniteWait = infiniteWait;
    this->data = data;
}

// the starting data reprensents some polynomial g(x) mod 10, this calculates the value of polynomial at specific point
int ReedSolomonCode::evaluatePolynomial(int x, bool forQML)
{
    QString belowTextExt = QString("f(%1) =").arg(x),
        belowText = QString("f(x) =");
    if (forQML)
    {
        for (int i = 0; i < numData; i++)
        {
            int num = (data[i].toLatin1() - '0');
            belowText.append(QString("%1 %2 * x^%3").arg(i == 0 ? " " : " +")
                                 .arg(num).arg(numData - 1 - i));
        }
        belowText.append(" mod 10");
        emit setBelowText(belowText);
        emit setBelowTextExtended("");
    }
    int result = 0;
    int xPower = 1; // x^0

    for (int i = 0; i < numData; i++)
    {
        int num = (data[i].toLatin1() - '0');
        result = mod10(result + num * xPower);
        xPower = mod10(xPower * x);
        if (forQML)
        {
            belowTextExt.append(QString("%1 %2 * %3^%4").arg(i == 0 ? " " : " +")
                                    .arg(num).arg(x).arg(numData - 1 - i));
            emit setBelowTextExtended(belowTextExt);
            this->waitForQml();
        }
    }

    if (forQML)
    {
        belowTextExt.append(QString(" = %1").arg(result));
        emit setBelowTextExtended(belowTextExt);
        this->waitForQml();
    }

    return result;
}

void ReedSolomonCode::encodeData(bool forQML)
{

    if(forQML)
    {
        static_cast<void>(QtConcurrent::run([=, this](){
            encodeDataAsync(forQML);
        }));
    }

    else
    {
        encodeDataAsync(forQML);
    }
}

void ReedSolomonCode::encodeDataAsync(bool forQML)
{
    if(forQML)
    {
        this->waitForQml();
        emit pushEmptyArray(numBlocks);
    }

    this->dataEncoded = {};

    for (int i = 0; i < numBlocks; i++)
    {
        if (forQML)
        {
            emit turnBitOn(0, i, "light blue");
        }
        int num = evaluatePolynomial(i + 1, forQML);
        QChar c = QChar(num + '0');
        dataEncoded.append(c);
        if (forQML)
        {
            emit setBit(0, i, c);
            this->waitForQml();
            emit turnBitOff(0, i);
        }
    }

    if (forQML) emit encodingEnd();
}


std::vector<int> ReedSolomonCode::calculateSyndromes() {
    std::vector<int> syndromes(numParity);
    for (int i = 0; i < numParity; ++i) {
        syndromes[i] = evaluatePolynomial(i + numData + 1, false);  // Evaluate at points 4 and 5 for redundancy checks
    }
    return syndromes;
}


int ReedSolomonCode::correctError(bool forQML)
{
    std::vector<int> syndromes = calculateSyndromes();
    // std::vector<int> corrected = received;

    // if (syndromes[0] == 0 && syndromes[1] == 0) {
    //     std::cout << "No errors detected." << std::endl;
    //     return corrected; // No errors
    // }

    // // We assume one error, find its location and value
    // // For simplicity, let's assume error is detectable and correctable easily (which would not always be the case)
    // for (int i = 0; i < received.size(); ++i) {
    //     // Create a vector of received values with one corrected element
    //     std::vector<int> testReceived = received;
    //     testReceived[i] = mod9(testReceived[i] + 1);  // Try increasing by 1

    //     if (calculateSyndromes(testReceived)[0] == 0 && calculateSyndromes(testReceived)[1] == 0) {
    //         std::cout << "Error corrected at position " << i << std::endl;
    //         return testReceived;  // Corrected vector
    //     }
    // }

    // std::cout << "Error detection/correction failed." << std::endl;
    // return corrected;  // Return the original if correction fails

}

int ReedSolomonCode::getAnimationDelayMs() const
{
    return animationDelayMs;
}

void ReedSolomonCode::setAnimationDelayMs(int delay)
{
    this->animationDelayMs = delay;
}

void ReedSolomonCode::setInfiniteWait(bool value)
{
    this->infiniteWait = value;
}

void ReedSolomonCode::waitForQml()
{
    if (this->infiniteWait)
    {
        while (!this->buttonPressed && this->infiniteWait);
        this->buttonPressed = false;
    }
    else
        QThread::currentThread()->msleep(this->animationDelayMs);
}

void ReedSolomonCode::pressButton()
{
    this->buttonPressed = true;
}

// simple % can return negative numbers
int ReedSolomonCode::mod10(int x)
{
    return ((x % 10) + 10) % 10;
}

void ReedSolomonCode::sendCode(QString sent)
{
    this->receivedCode = sent;
}

QString ReedSolomonCode::getDataEncoded() const
{
    return this->dataEncoded;
}
