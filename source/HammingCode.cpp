#include "HammingCode.hpp"

#include <QtConcurrent>
#include <QThread>
#include <QtMath>

#include <boost/range/adaptor/transformed.hpp>
#include <boost/range/numeric.hpp>
#include <boost/range/irange.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/range/combine.hpp>
#include <boost/range/adaptors.hpp>

HammingCode::HammingCode(QObject *parent) : QObject{parent}{}

void HammingCode::setInitialData(QBitArray data, bool extend, int animationDelay, bool infiniteWait){
    this->data = data;
    this->m = data.size();
    this->p = calculateP();
    this->animationDelayMs = animationDelay;
    this->encodingExtended = extend;
    this->infiniteWait = infiniteWait;
    this->receivedCode.clear();
}

void HammingCode::setInitialData(QString data, bool extend, int animationDelay, bool infiniteWait){
    QBitArray bits(data.size());

    boost::transform(data.toStdString(), const_cast<char*>(bits.bits()),
        [](QChar c) { return c.toLatin1() - '0'; }
    );

    setInitialData(bits, extend, animationDelay, infiniteWait);
}

//Simulate sending some code, it can have 1 bit error
void HammingCode::sendCode(QBitArray send){
    this->receivedCode = send;
}

void HammingCode::sendCode(QString send){

    QBitArray code(send.size());

    boost::transform(send.toStdString(), const_cast<char*>(code.bits()),
        [](QChar c) { return c.toLatin1() - '0'; }
    );

    sendCode(code);
}

int HammingCode::correctErrorExtended(bool forQML)
{
    int n = this->m + this->p, C{}, P{}, bit{};

    QString belowText{}, initialText{"<font color=\"orange\">C =</font> %1"}, addText{"<font color=\"orange\">C +=</font> %1"}, belowTextExt{};

    auto getBit = [&](int i) { return receivedCode.testBit(i) ? 1 : 0; };

    if(forQML){
        emit setBelowText(initialText.arg(C));
        this->waitForQml();

        emit setBelowText(addText.arg(C));
        this->waitForQml();
    }

    for(int i = 1; i < n; i *= 2){

        int xorVal = 0;

        xorVal ^= receivedCode[i]; //i because extended parity bit must be not used for this

        if(forQML){
            emit turnBitOn(0, i, "light green");
            this->waitForQml();
            emit turnBitOff(0, i);

            belowText = addText.arg(receivedCode[i]);
            belowTextExt = addText.arg(this->getSymbol(i));
        }

        for(int j = i + 1; j < n; j++){  //indexing +1, so <= n -> < n

            if(j & (1 << bit)){

                xorVal ^= receivedCode[j]; //indexing +1

                if(forQML){
                    emit turnBitOn(0, j, "light green");
                    belowText.append(QString(" ^ %1").arg(receivedCode[j]));
                    emit setBelowText(belowText);
                    belowTextExt.append(QString(" ^ %1").arg(this->getSymbol(j)));
                    emit setBelowTextExtended(belowTextExt);

                    this->waitForQml();
                    emit turnBitOff(0, j);
                }
            }
        }

        if(forQML){
            emit setBelowText(addText.arg(xorVal));
            this->waitForQml();

            emit setBelowText(addText.arg(QString("%1 * %2").arg(xorVal).arg(i)));
            int powerOfTwo = qFloor(qLn(i) / qLn(2.0));
            belowTextExt.append(QString(" * 2^%1").arg(powerOfTwo));
            emit setBelowTextExtended(belowTextExt);
            this->waitForQml();
        }

        C += xorVal * i;
        bit++;

        if(forQML){
            emit setBelowText(initialText.arg(C));
            emit setBelowTextExtended(QString(""));
            this->waitForQml();
        }
    }

    initialText = "<font color=\"purple\">P =</font> ";

    if(forQML){
        belowText = QString(initialText);
        belowTextExt = QString(initialText);

        emit setBelowText(belowText);
        emit setBelowTextExtended(belowTextExt);

        this->waitForQml();
    }

    if (forQML) {
        boost::for_each(boost::irange(0, n), [&](int i) {
            emit turnBitOn(0, i, "light green");
            belowText.append(QString(i == 0 ? "%1" : " ^ %1").arg(getBit(i)));
            emit setBelowText(belowText);
            belowTextExt.append(QString(i == 0 ? "%1" : " ^ %1").arg(this->getSymbol(i)));
            emit setBelowTextExtended(belowTextExt);

            this->waitForQml();
            emit turnBitOff(0, i);
        });
    }

    P = boost::accumulate(boost::irange(0, n), P, [&](int acc, int i) { return acc ^ getBit(i); });

    emit setBelowTextExtended(QString(""));
    emit setBelowText(QString("C = %1, P = %2").arg(C).arg(P));

    int ret{};

    if(C == 0){ //theory from youtube
        if(P == 1){
            qInfo() << "Error is in extended parity bit";
            emit setBelowTextExtendedTranslation(10, {""});

//            receivedCode[C] = !receivedCode[C];
            ret = C;
            this->setError(1);
        }
        else{
            qInfo() << "There is no error!";
            emit setBelowTextExtendedTranslation(11, {""});
            ret = -1;
            this->setError(0);
        }
    }
    else{
        if(P == 1){
            qInfo() << "Single error occured at the position: " + QString::number(C) + " correcting...";
            emit setBelowTextExtendedTranslation(12, {QString::number(C + 1)});
//            receivedCode[C] = !receivedCode[C];
            ret = C;
            this->setError(C + 1);
        }
        else{
            qInfo() << "There is a double error, but it can't be calculated where";
            emit setBelowTextExtendedTranslation(13, {""});
            ret = -2;
        }
    }

    if(forQML) emit endErrorCorrection(C, P);
    this->finished = true;
    return ret;
}

int HammingCode::correctErrorStandard(bool forQML){
    int n = this->m + this->p, C{}, bit{};

    QString belowText{}, initialText{"<font color=\"orange\">C =</font> %1"}, addText{"<font color=\"orange\">C +=</font> %1"}, belowTextExt{};

    if(forQML){
        emit setBelowText(initialText.arg(C));
        this->waitForQml();

        emit setBelowText(addText.arg(C));
        this->waitForQml();
    }

    for(int i = 1; i <= n; i *= 2){ //calculate parity bits and add them up with formula: p1 * 1 + p2 * 2 + p3 * 4 + ... = C

        int xorVal = 0;

        xorVal ^= receivedCode[i - 1]; //xor is same as counting 1's

        if(forQML){
            emit turnBitOn(0, i - 1, "light green");
            this->waitForQml();
            emit turnBitOff(0, i - 1);

            belowText = addText.arg(receivedCode[i - 1]);
            belowTextExt = addText.arg(this->getSymbol(i - 1));
        }

        for(int j = i + 1; j <= n; j++){

            if(j & (1 << bit)){

                xorVal ^= receivedCode[j - 1];

                if(forQML){
                    emit turnBitOn(0, j - 1, "light green");
                    belowText.append(QString(" ^ %1").arg(receivedCode[j - 1]));
                    emit setBelowText(belowText);

                    this->waitForQml();
                    emit turnBitOff(0, j - 1);
                    belowTextExt.append(QString(" ^ %1").arg(this->getSymbol(j - 1)));
                    emit setBelowTextExtended(belowTextExt);
                }
            }
        }

        if(forQML){
            emit setBelowText(addText.arg(xorVal));
            this->waitForQml();

            emit setBelowText(addText.arg(QString("%1 * %2").arg(xorVal).arg(i)));
            int powerOfTwo = qFloor(qLn(i) / qLn(2.0));
            belowTextExt.append(QString(" * 2^%1").arg(powerOfTwo));
            emit setBelowTextExtended(belowTextExt);
            this->waitForQml();
        }

        C += xorVal * i;
        bit++;

        if(forQML){
            emit setBelowText(initialText.arg(C));
            emit setBelowTextExtended(QString(""));
            this->waitForQml();
        }
    }

    this->setError(C);

    int ret{};

    emit setBelowText("C = " + QString::number(C));

    if(C == 0){ //theory from youtube
        qInfo() << "There is no error!";
        emit setBelowTextExtendedTranslation(11, {""});
        ret = -1;
    }
    else{
        qInfo() << "Error occured at the position: " << (C - 1);
        emit setBelowTextExtendedTranslation(12, {QString::number(C)});
//        receivedCode[C - 1] = !receivedCode[C - 1];
        ret = C - 1;
    }

    if(forQML) emit endErrorCorrection(C, -1);
    this->finished = true;
    return ret;
}

//Correct the 1 bit error in received code
int HammingCode::correctError(bool forQML){
    if(forQML){
        this->finished = false;
        if(this->encodingExtended){
            static_cast<void>(QtConcurrent::run([=, this](){
                correctErrorExtended(forQML);
            }));
        }

        else{
            static_cast<void>(QtConcurrent::run([=, this](){
                correctErrorStandard(forQML);
            }));
        }
    }

    else{
        if(this->encodingExtended) return correctErrorExtended(forQML);
        else return correctErrorStandard(forQML);
    }

    return -1;
}

int HammingCode::calculateP(){

    int power = 1, newP{};

    while(power < (m + newP + 1)){ //the lowest integer that satisfies the formula 2^p >= m + p + 1
        newP++;
        power <<= 1;
    }

    return newP;
}

bool HammingCode::isPowerTwo(int n){
    return (n > 0) && ((n & (n - 1)) == 0); //O(1) trick
}

void HammingCode::encodeData(bool forQML){

    if(forQML){
        this->finished = false;
        static_cast<void>(QtConcurrent::run([=, this](){
            encodeDataAsync(forQML);
        }));
    }

    else{
        encodeDataAsync(forQML);
    }
}

void HammingCode::encodeDataAsync(bool forQML){
    this->setSymbols();

    int n = this->m + this->p, dataPtr{};

    QBitArray dataEncoded(n);

    if(forQML){
        this->waitForQml();
        emit pushArray(this->getDataStr(), false);

        this->waitForQml();
        emit pushEmptyArray(n);       
    }

    for(int i = 0; i < n; i++){

        bool isParity = isPowerTwo(i + 1),
              dataBit = data[dataPtr];

        if(!isParity) dataEncoded[i] = data[dataPtr]; //copying non-parity bits

        if(forQML){
            emit turnBitOn(0, dataPtr, "light blue");

            if(!isParity) {
                emit turnBitOn(1, i, "light green");
                emit setBit(1, i, dataBit ? "1" : "0");
            }

            else {
                emit turnBitOn(1, i, "red");
                emit setBit(1, i, "0");
            }

            this->waitForQml();
            emit turnBitOff(1, i);
            emit turnBitOff(0, dataPtr);
        }

        if(!isParity) dataPtr++;
    }

    if(forQML){
        emit deleteArrayAtIndex(0);
        this->waitForQml();
    }

    int bit = 0;

    for(int i = 1; i <= n; i *= 2){ //calculating parity bits

        int xorVal = 0; //counting number of 1's for each parity bit, xor just signals even/odd count
        QString belowTextExt{};

        QStringList args = {};
        if(forQML){
            belowTextExt = QString("%1 = %1").arg(this->getSymbol(i - 1));

            emit turnBitOn(0, i - 1, "yellow");
            args << QString::number(qLn(i)/qLn(2.0)) << " =</font> " << QString::number(dataEncoded[i - 1]);
            emit setBelowTextTranslationColorized("Blue", 14, args);
            emit setBelowTextExtended(belowTextExt);

            this->waitForQml();
        }

        for(int j = i + 1; j <= n; j++){

            if(j & (1 << bit)){ //bit manipulation trick

                xorVal ^= dataEncoded[j - 1];

                if(forQML){
                    emit turnBitOn(0, j - 1, "red");
                    args << " ^ " << QString::number(dataEncoded[j - 1]);
                    emit setBelowTextTranslationColorized("Blue", 14, args);
                    belowTextExt.append(QString(" ^ %1").arg(this->getSymbol(j - 1)));
                    emit setBelowTextExtended(belowTextExt);

                    this->waitForQml();
                    emit turnBitOff(0, j - 1);
                }
            }
        }

        dataEncoded[i - 1] = xorVal; //keeping even number of 1's in the code
        bit++;

        if(forQML){

            emit setBelowTextTranslation(14, {QString::number(xorVal)});
            this->waitForQml();

            emit turnBitOn(0, i - 1, "light green");
            emit setBit(0, i - 1, xorVal ? "1" : "0");

            this->waitForQml();

            emit turnBitOff(0, i - 1);
            emit setBelowText(QString(""));
            emit setBelowTextExtended(QString(""));
        }
    }

    //putting dataEncoded into data
    if(this->encodingExtended){

        this->p++; //increase parity count if extended

        data = QBitArray(n + 1);
        QString belowTextExt{"p = p"};
        this->symbols.prepend(QString("p"));

        int xorVal{};

        QStringList args = {};

        if(forQML){
            args << "</font> " << QString::number(xorVal);
            emit setBelowTextTranslationColorized("purple", 15, args);
            emit setBelowTextExtended(belowTextExt);

            this->waitForQml();
        }

        for(int i = 0; i < n; i++){

            if(forQML){
                emit turnBitOn(0, i, "red");
                args << " ^ " << QString::number(dataEncoded[i]);
                emit setBelowTextTranslationColorized("purple", 15, args);
                belowTextExt.append(QString(" ^ %1").arg(this->getSymbol(i + 1)));
                emit setBelowTextExtended(belowTextExt);

                this->waitForQml();
                emit turnBitOff(0, i);
            }

            xorVal ^= dataEncoded[i];
        }

        data[0] = xorVal; //extended parity bit at the beginning

        if(forQML){

            emit setBelowTextTranslation(15, {QString::number(xorVal)});
            this->waitForQml();

            emit insertBit(0, 0, xorVal == 1 ? "1" : "0", true);
            emit turnBitOn(0, 0, "purple");

            this->waitForQml();

            emit turnBitOff(0, 0);
            emit setBelowText(QString(""));
            emit setBelowTextExtended(QString(""));
        }

        for (int i = 0; i < n; i++) {
            data[i + 1] = dataEncoded[i]; //copy the rest into data, doesn't need visualization
        }
    }

    else data = dataEncoded; //just copy the rest without extending the bit

    if(forQML) emit encodingEnd();
    this->finished = true;
}

QString HammingCode::getDataStr()
{
    QString ret{};

    int n = this->m;

    for(int i = 0; i < n; i++){
        ret.append(QChar(this->data[i] + '0'));
    }

    return ret;
}

int HammingCode::getAnimationDelayMs() const{
    return animationDelayMs;
}

void HammingCode::setAnimationDelayMs(int delay){
    this->animationDelayMs = delay;
}

int HammingCode::getP() const{
    return p;
}

int HammingCode::getM() const{
    return m;
}

QBitArray HammingCode::getData(){
    return this->data;
}

QString HammingCode::getReceivedCode() {
    QString ret{};
    boost::transform(std::string(receivedCode.bits()), std::back_inserter(ret), [](char bit) {
        return QChar( bit ? '1' : '0' );
    });
    return ret;
}

bool HammingCode::getEncodingExtended() const{
    return encodingExtended;
}

void HammingCode::setInfiniteWait(bool value){
    this->infiniteWait = value;
}

void HammingCode::quit() {
    this->animationDelayMs = 0;
    this->shouldQuit = true;
}

bool HammingCode::isFinished() {
    return this->finished;
}

void HammingCode::waitForQml(){
    if (this->infiniteWait) {
        while (!this->buttonPressed && this->infiniteWait && !this->shouldQuit);
        this->buttonPressed = false;
    }
    else
        QThread::currentThread()->msleep(this->animationDelayMs);
}

void HammingCode::pressButton(){
    this->buttonPressed = true;
}

// generation matrix created based on the error matrix
// each row is one bit of input, each column means whether the bit is used
// on this position in the encoded data (either included in parity bit or this position simply is this bit)
QString HammingCode::getGenerationMatrixStr()
{
    QString ret{};

    int numOfCols = this->m + this->p,
        numOfRows = this->m,
        position = 3; // position in the encoded data of non-parity bit, 1 and 2 are parity

    for(const auto i : boost::irange(numOfRows)) {
        QStringList parityRow;

        while (isPowerTwo(position)) position++; // has to be non-parity

        int n = i, parityBitCounter = 0;

        for (int j = 0; j < numOfCols; j++) {

            bool bitUsedInCurrentPosition = false,
                 isParity = isPowerTwo(j + 1);

            if(!isParity) {
                bitUsedInCurrentPosition = ((n--) == 0); // bit used at n-th non-parity position
                ret.append(QChar(bitUsedInCurrentPosition ? '1' : '0'));
            }
            else {
                bitUsedInCurrentPosition = (1 << parityBitCounter) & position;
                parityBitCounter++;
                parityRow.append(QChar(bitUsedInCurrentPosition ? '1' : '0'));
            }
        }

        ret.append(parityRow.join(""));

        if (i != numOfRows - 1) ret.append(QChar('\n'));

        position++;
    }

    return ret;
}

// columns are binary numbers in order beginning at 1
// 0 0 0 1 1 1 1
// 0 1 1 0 0 1 1
// 1 0 1 0 1 0 1
// basically, the last row is 0 then 1 then 0
// previous is two 0s, two 1s, etc, except one fewer zero at the beginning (because start at 1 not 0)
QString HammingCode::getErrorMatrixStr() {
    QString ret{};

    int numOfCols = this->m + this->p,
        numOfRows = this->p;

    for (const auto i : boost::irange(numOfRows)) {
        QString row{};

        int zeroCount = (1 << i) - 1,
            colCounter = 0;

        std::fill_n(std::back_inserter(row), zeroCount, '0');
        colCounter += zeroCount;

        bool insertZero = false;
        int howManyInARow = (1 << i);

        while (colCounter < numOfCols) {
            int remainingCols = std::min(howManyInARow, numOfCols - colCounter);
            char value = insertZero ? '0' : '1';
            std::fill_n(std::back_inserter(row), remainingCols, value);
            colCounter += remainingCols;
            insertZero = !insertZero;
        }

        if (i != 0) row.append(QChar('\n'));
        ret.prepend(row);
    }

    return ret;
}

QString HammingCode::getEncodedStr() {
    QString ret{}, error = getError(), received = getReceivedCode();

    auto combinedStrings = boost::combine(error.toStdString(), received.toStdString());

    boost::transform(combinedStrings, std::back_inserter(ret), [](const auto& pair) {
        int errorBit = boost::get<0>(pair) - '0',
            receivedBit = boost::get<1>(pair) - '0',
            xorResult = errorBit ^ receivedBit;

        return static_cast<char>(xorResult + '0');
    });

    return ret;
}

QString HammingCode::getError() {
    return this->error;
}

QString HammingCode::getSyndrome() {
    return this->syndrome;
}

void HammingCode::setError(int C){
    int syndromeLength = qCeil(qLn(this->m + this->p) / qLn(2));

    this->error = {};
    this->syndrome = QString::number(C, 2).rightJustified(syndromeLength, '0'); // converts to binary, left padded with 0s

    for (int i : boost::irange(0, this->m + this->p)) {
        this->error.append(QChar(i == C - 1 ? '1' : '0'));
    }
}

QString HammingCode::getSymbol(int index) {
    return this->symbols[index];
}

void HammingCode::setSymbols() {
    this->symbols = {};

    int parityIndex = 0, dataIndex = 0;

    for (int i : boost::irange(0, this->m + this->p)) {
        QString symbol;
        if (isPowerTwo(i + 1)) symbol = QString("p%1").arg(parityIndex++);
        else symbol = QString("d%1").arg(dataIndex++);
        this->symbols << symbol;
    }
}

QString HammingCode::getDecodedStr() {
    QString ret {}, encoded = getEncodedStr();

    auto nonParityFilter = [this](int i) {
        return !isPowerTwo(this->encodingExtended ? i : i + 1);
    };

    for (int i : boost::irange(this->encodingExtended ? 1 : 0, this->m + this->p)
                     | boost::adaptors::filtered(nonParityFilter)) {
        ret.append(encoded.at(i));
    }
    return ret;
}

