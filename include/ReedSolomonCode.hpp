#ifndef REEDSOLOMONCODE_HPP
#define REEDSOLOMONCODE_HPP

#include <QObject>
#include <QBitArray>
#include <QDebug>

class ReedSolomonCode : public QObject
{
    Q_OBJECT

public:
    explicit ReedSolomonCode(QObject *parent = nullptr);

public slots:
    void setInitialData(QString data, int animationSpeed = 1000, bool infiniteWait = false);
    int correctError(bool forQML);
    void encodeDataAsync(bool forQML);
    void encodeData(bool forQML);
    void sendCode(QString sent);

    int getAnimationDelayMs() const;
    QString getDataEncoded() const;

    void setAnimationDelayMs(int delay);
    void setInfiniteWait(bool value);

    void pressButton();
signals:

    void turnBitOn(int arrIndex, int index, QString color = "");
    void turnBitOff(int arrIndex, int index);

    void pushEmptyArray(int size);
    void pushArray(QString str, bool showSymbols);

    void popArray();
    void deleteArrayAtIndex(int index);

    void setBit(int arrIndex, int index, QString bit);

    void insertEmptyBit(int arrIndex, int index);
    void insertBit(int arrIndex, int index, QString bit, bool showSymbols);

    void insertArray(int index, QString str, bool showSymbols);

    void setBelowText(QString str);
    void setBelowTextExtended(QString str);
    void setClickAllow(int arrIndex, bool isAllowed);

    void encodingEnd();
    void endErrorCorrection();

private:
    const int numData = 3, numParity = 2, numBlocks = numData + numParity;

    QString data{}, receivedCode{}, dataEncoded{};
    int animationDelayMs{};
    bool infiniteWait = false, buttonPressed = false;

    void waitForQml();
    int mod10(int x);
    int evaluatePolynomial(int x, bool forQML);
    std::vector<int> calculateSyndromes();
};

#endif // REEDSOLOMONCODE_HPP
