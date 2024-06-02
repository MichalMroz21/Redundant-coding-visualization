#include "ReedSolomonCode.hpp"

#include <QtConcurrent>
#include <QThread>
#include <QtMath>

ReedSolomonCode::ReedSolomonCode(QObject *parent) : QObject{parent}, gf(fieldPower) {
}

Poly ReedSolomonCode::getRemainder(QString data)
{
    this->data = data;
    for (int i = 0; i < data.length(); i++)
    {
        this->dataArr[i] = data.at(i).toLatin1() - '0';
    }
    Poly msg(this->k, this->dataArr);
    Poly generator, remainder;
    this->createGenerator(&generator, true);
    Poly_Pad(&msg, 0, nsym);
    Poly_Div(nullptr, nullptr, &remainder, &msg, &generator, &this->gf);
    return remainder;
}

void ReedSolomonCode::setInitialData(QString data, int animationDelay, bool infiniteWait)
{
    assert(data.length() == this->k);
    this->animationDelayMs = animationDelay;
    this->infiniteWait = infiniteWait;
    this->data = data;
    for (int i = 0; i < data.length(); i++)
    {
        this->dataArr[i] = data.at(i).toLatin1() - '0';
    }
}

void ReedSolomonCode::encodeData(bool forQML)
{
    if(forQML)
    {
        this->finished = false;
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
    Poly msg(this->k, this->dataArr);
    Poly generator, remainder;
    this->createGenerator(&generator, forQML);
    if(forQML)
    {
        emit pushEmptyArray(this->total);
        for (int i = 0; i < this->k; i++)
        {
            emit setBit(0, i, this->data.at(i));
        }
        for (int i = k; i < total; i++)
        {
            emit setBit(0, i, QChar('a' + i - k));
        }
        //emit setBelowText("Przepisanie danych wejściowych");
        emit setBelowTextTranslation(9, {""});
        emit setBelowTextExtended("");
        this->waitForQml();
    }
    Poly_Pad(&msg, 0, nsym);
    Poly_Div(nullptr, nullptr, &remainder, &msg, &generator, &this->gf);
    if (forQML) {
        Poly msgRev;
        Poly_Reverse(&msgRev, &msg);
        Poly genRev;
        Poly_Reverse(&genRev, &generator);
        //emit setBelowText(QString("wiadomość / wielomian generujący = ? + (bx + a)"));
        emit setBelowTextExtended(QString("(%1) / (%2) = ? + (%3)")
                                      .arg(msgRev.toString())
                                      .arg(genRev.toString())
                                      .arg(remainder.toString()));
        emit setBelowTextTranslation(10, {""});
        this->waitForQml();
        for (int i = k; i < total; i++)
        {
            emit setBit(0, i, QChar('0' + remainder.coef[i - k]));
            this->waitForQml();
        }
    }
    memcpy(this->dataEncodedArr, this->dataArr, sizeof(int) * k);
    memcpy(this->dataEncodedArr + k, remainder.coef, sizeof(int) * remainder.n);


    this->dataEncoded = {};
    for (int i = 0; i < this->total; i++)
    {
        this->dataEncoded.append(QChar(this->dataEncodedArr[i] + '0'));
    }

    if (forQML) emit encodingEnd();
    this->finished = true;
}

void ReedSolomonCode::correctErrorQml() {
    this->finished = false;
    static_cast<void>(QtConcurrent::run([=, this](){
        correctError(true);
    }));
}

int ReedSolomonCode::getRet(int n, int* coef)
{
    Poly synd;
    Poly msg(n, coef);
    this->calcSyndromes(&synd, &msg, false);

    int ret;

    Poly fsynd, errLoc;
    this->forneySyndromes(&fsynd, &synd, k + nsym);
    bool canLocate1 = this->findErrorLocator(&errLoc, &fsynd, false);

    std::vector<unsigned int> pos;
    bool canLocate2 = this->findErrors(&pos, &errLoc, k + nsym, false);
    //for_each(pos.begin(), pos.end(), [](unsigned int e) {qInfo() << (int)e << ", "; });

    if (pos.size())
    {
        ret = pos[0];
    }

    bool success = this->correctErrata(&msg, &synd, &pos, false);

    return ret;
}

int ReedSolomonCode::correctError(bool forQML)
{
    Poly synd;
    Poly msg(k + nsym, this->receivedCodeArr);

    this->calcSyndromes(&synd, &msg, forQML);
    int ret;
    if (this->checkSyndromes(&synd))
    {
        if (forQML)
        {
            //emit setBelowText("Wszystkie syndromy są równe 0 - nie znaleziono błędu");
            emit setBelowTextTranslation(11, {""});
        }
        ret = -1;
    }
    else
    {
        if (forQML)
        {
            //emit setBelowText("Nie wszystkie syndromy są równe 0 - występują błędy");
            emit setBelowTextTranslation(12, {""});
            this->waitForQml();
        }
        qInfo() << "Występują błędy, wyszukiwanie pozycji";
        Poly fsynd, errLoc;
        this->forneySyndromes(&fsynd, &synd, k + nsym);
        bool canLocate = this->findErrorLocator(&errLoc, &fsynd, forQML);
        if (!canLocate)
        {
            qInfo() << "Zbyt dużo błędów do znalezienia!";
            if (forQML)
            {
                //emit setBelowText("Zbyt dużo błędów do znalezienia!");
                emit setBelowTextTranslation(13, {""});
                emit endErrorCorrection();
            }
            return -1;
        }
        if (forQML)
        {
            Poly revErrLoc;
            Poly_Reverse(&revErrLoc, &errLoc);
            //emit setBelowText("Wielomian wykrycia błędów znaleziony algorytmem Berlekamp-Masseya");

            emit setBelowTextTranslation(14, {""});
            emit setBelowTextExtended(revErrLoc.toString());
            this->waitForQml();
        }
        std::vector<unsigned int> pos;
        canLocate = this->findErrors(&pos, &errLoc, k + nsym, forQML);
        if (!canLocate || !(pos.size()))
        {
            qInfo() << "Nie udało się znaleźć błędów!";
            if (forQML)
            {
                //emit setBelowText("Nie udało się znaleźć błędów!");
                emit setBelowTextTranslation(15, {""});
                emit endErrorCorrection();
            }
            return -1;
        }
        if (pos.size())
        {
            qInfo() << "Dodatkowe błędy znalezione na pozycjach: ";
            for_each(pos.begin(), pos.end(), [](unsigned int e) {qInfo() << (int)e << ", "; });
            ret = pos[0];
            if (forQML)
            {
                //emit setBelowText(QString("Pozycja błędu znaleziona przez algorytm Chien: %1").arg(ret + 1));
                emit setBelowTextTranslation(16, {QString::number(ret + 1)});
                emit setBelowTextExtended("");
                this->waitForQml();
            }
        }
        bool success = this->correctErrata(&msg, &synd, &pos, forQML);
        if (!success)
        {
            qInfo() << "Nie udało się zdekodować!";
            if (forQML)
            {
                //emit setBelowText("Nie udało się zdekodować!");
                emit setBelowTextTranslation(17, {""});
                emit endErrorCorrection();
            }
            return -1;
        }
        QString poprawione = {};
        for (int i = 0; i < this->total; i++)
        {
            poprawione.append(QChar(msg.coef[i] + '0'));
        }
        if (forQML)
        {
            //emit setBelowText(QString("Znaleziono błąd na pozycji %1").arg(ret + 1));
            //emit setBelowTextExtended(QString("Poprawiona wiadomość: %1").arg(poprawione));

            emit setBelowTextTranslation(18, {QString::number(ret + 1)});
            emit setBelowTextExtendedTranslation(19, {poprawione});
        }
        qInfo() << "Poprawiono błędy";
    }
    if(forQML) emit endErrorCorrection();
    this->finished = true;
    return ret;
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

void ReedSolomonCode::quit() {
    this->animationDelayMs = 0;
    this->shouldQuit = true;
}

bool ReedSolomonCode::isFinished() {
    return this->finished;
}

void ReedSolomonCode::waitForQml()
{
    if (this->infiniteWait)
    {
        while (!this->buttonPressed && this->infiniteWait && !this->shouldQuit);
        this->buttonPressed = false;
    }
    else
        QThread::currentThread()->msleep(this->animationDelayMs);
}

void ReedSolomonCode::pressButton()
{
    this->buttonPressed = true;
}

void ReedSolomonCode::sendCode(QString sent)
{
    this->receivedCode = sent;
    for (int i = 0; i < sent.length(); i++)
    {
        this->receivedCodeArr[i] = sent.at(i).toLatin1() - '0';
    }
}

QString ReedSolomonCode::getDataEncoded() const
{
    return this->dataEncoded;
}

QString toString(int* arr, int size)
{
    QString result{};
    bool plus = false;
    for (int i = size - 1; i >= 0; i--)
    {
        if (arr[i] == 0) continue;
        QString x = QString("x^%1 ").arg(i);
        if (i == 1) x = "x ";
        if (i == 0) x = "";
        result.append(QString("%1%2%3")
                          .arg(plus ? "+ " : "")
                          .arg(QChar('0' + arr[i]))
                          .arg(x)
                      );
        plus = true;
    }
    return result;
}


std::vector<unsigned int> toVector(QString s)
{
    std::vector<unsigned int> result(s.length());
    for (QChar c : s)
    {
        result.push_back(c.toLatin1() - '0');
    }
    return result;
}

void ReedSolomonCode::createGenerator(Poly* out, bool forQML)
{
    out->setCopy(1, nullptr);
    out->coef[0] = 1;
    Poly factor(2, nullptr);
    factor.coef[0] = 1;
    for (int i = 0; i < this->nsym; i++)
    {
        factor.coef[1] = this->gf.powTable[i];
        Poly_Mult(out, out, &factor, &this->gf);
    }
    if (forQML)
    {
        Poly genRev;
        Poly_Reverse(&genRev, out);
        //emit setBelowText("Wielomian generujący");
        emit setBelowTextTranslation(20, {""});
        emit setBelowTextExtended(genRev.toString());
        this->waitForQml();
    }
}

void ReedSolomonCode::calcSyndromes(Poly* out, Poly* msg, bool forQML)
{
    int* temp = (int*)malloc(sizeof(int) * (nsym + 1));
    for (int i = 0; i < nsym; i++)
    {
        QString text = QString("(%1)(%2)").arg(msg->toString()).arg(this->gf.powTable[i]);
        if (forQML)
        {
            QString belowText = QString("Liczenie syndromu nr %1 - Wartość wiadomości w punkcie α%2").arg(i + 1).arg(i);
            emit setBelowTextCalcSyndromes((i+1), i);
            //emit setBelowText(belowText);
            emit setBelowTextExtended(text);
            this->waitForQml();
        }
        temp[nsym - i - 1] = Poly_Eval(msg, this->gf.powTable[i], &this->gf);
        if (forQML)
        {
            text.append(QString(" = %1").arg(temp[nsym - i -1]));
            qInfo() << "text: " << text;
            emit setBelowTextExtended(text);
            this->waitForQml();
        }
    }
    temp[nsym] = 0; //pad
    out->setRef(nsym + 1, temp);
    if (forQML)
    {
        emit setBelowText("");
        emit setBelowTextExtended("");
    }
}

bool ReedSolomonCode::checkSyndromes(Poly* synd)
{
    for (int i = 0; i < synd->n; i++)
    {
        if (synd->coef[i])
        {
            return false;
        }
    }
    return true;
}

void ReedSolomonCode::findErrataLocator(Poly* out, std::vector<unsigned int>* errPos, bool forQML)
{
    out->setCopy(1, nullptr);
    out->coef[0] = 1;
    Poly factor(2, nullptr);
    factor.coef[1] = 1;
    for (unsigned int i : *errPos)
    {
        factor.coef[0] = this->gf.powTable[i];
        Poly_Mult(out, out, &factor, &this->gf);
        if (forQML)
        {
            emit setBelowText("errata_locator = 1x + 2 ^ coef_pos");
            emit setBelowTextExtended(QString("1x + 2^%1 = %2").arg(i).arg(out->toString()));
            this->waitForQml();
        }
    }
}

void ReedSolomonCode::findErrorEvaluator(Poly* out, Poly* synd, Poly* errLoc, int nsym, bool forQML)
{

    Poly_Mult(out, synd, errLoc, &this->gf); //synd lul
    {
        emit setBelowText("syndrom * errata_locator = error_evaluator * x^2 + bx + c");
        emit setBelowTextExtended(QString("(%1) * (%2) = %3").arg(synd->toString()).arg(errLoc->toString()).arg(out->toString()));
        this->waitForQml();
    }
    Poly_Trim(out, out->n - nsym, 0);
    if (forQML)
    {
        emit setBelowText("error_evaluator");
        emit setBelowTextExtended(QString("%1").arg(out->toString()));
        this->waitForQml();
    }
}

bool ReedSolomonCode::correctErrata(Poly* msg, Poly* synd, std::vector<unsigned int>* errPos, bool forQML)
{
    if (forQML)
    {
        //emit setBelowText("Szukanie wielkości błędu");
        emit setBelowTextTranslation(23, {""});
        emit setBelowTextExtended("");
        this->waitForQml();
    }
    std::vector<unsigned int> coefPos(0);
    for (unsigned int i : *errPos)
    {
        coefPos.push_back(msg->n - 1 - i);
        if (forQML)
        {
            //emit setTopText("Szukanie wielkości błędu");
            emit setTopTextTranslation(23, {""});
            //emit setBelowText("coef_pos - pozycja błędu liczona od końca od 0");
            emit setBelowTextTranslation(24, {""});
            emit setBelowTextExtended(QString("coef_pos = %1").arg(msg->n - 1 - i));
            this->waitForQml();
        }
    }
    Poly errLoc, errEval;
    this->findErrataLocator(&errLoc, &coefPos, forQML);
    if (forQML)
    {
        qInfo() << "Errata locator: " << errLoc.toString();
    }
    this->findErrorEvaluator(&errEval, synd, &errLoc, errLoc.n, forQML);
    if (forQML)
    {
        qInfo() << "Evaluator: " << errEval.toString();
    }
    //Poly_Reverse(errEval, errEval); //reverse it for later use
    std::vector<int> x(coefPos.size());
    for (int i = 0; i < x.size(); i++)
    {
        x[i] = this->gf.powTable[coefPos[i]];
    }
    Poly e(msg->n, nullptr);
    for (int i = 0; i < x.size(); i++)
    {
        int xi = this->gf.powTable[this->gf.characteristic - coefPos[i]];
        if (forQML)
        {
            //emit setBelowText("xi = 2^(charakterystyka GF - coef_pos)");
            emit setBelowTextTranslation(25, {""});
            emit setBelowTextExtended(QString("xi = 2^(%1 - %2) = %3").arg(this->gf.characteristic).arg(coefPos[i]).arg(xi));
            this->waitForQml();
        }
        int errLocPrime = 1;
        for (int j = 0; j < x.size(); j++)
        {
            if (j != i)
            {
                errLocPrime = this->gf.mult(errLocPrime, 1 ^ this->gf.mult(xi, x[j]));
            }
        }
        if (errLocPrime == 0)
        {
            return false;
        }
        if (forQML)
        {
            emit setBelowText(QString("err_loc_prime = %1").arg(errLocPrime));
            emit setBelowTextExtended("");
            this->waitForQml();
        }
        int eval = Poly_Eval(&errEval, xi, &this->gf);
        int y = this->gf.mult(x[i], eval);
        if (forQML)
        {
            emit setBelowText(QString("y = 2 ^ coef_pos * error_evaluator").arg(errLocPrime));
            emit setBelowTextExtended(QString("y = 2^%1 * %2").arg(x[i]).arg(eval));
            this->waitForQml();
        }
        e.coef[errPos->at(i)] = this->gf.div(y, errLocPrime); //magnitude
        if (forQML)
        {
            //emit setBelowText("Wielkość błędu = y / err_loc_prime");
            //emit setBelowTextExtended(QString("Wielkość błędu = %1 / %2 = %3").arg(y).arg(errLocPrime).arg(e.coef[errPos->at(i)]));
            emit setBelowTextTranslation(26, {""});
            emit setBelowTextExtendedTranslation(27, {QString::number(y), " / " , QString::number(errLocPrime), " = ", QString::number(e.coef[errPos->at(i)])});
            this->waitForQml();
        }
    }
    QString tmp = msg->toString();
    Poly_Add(msg, msg, &e);
    if (forQML)
    {
        //emit setBelowText("Poprawiona wiadomość = błędy + otrzymana wiadomość");
        emit setBelowTextTranslation(28, {""});
        emit setBelowTextExtended(QString("(%1) + (%2)").arg(e.toString()).arg(tmp));
        this->waitForQml();
        emit setBelowTextExtended(msg->toString());
        this->waitForQml();
    }
    return true;
}

bool ReedSolomonCode::findErrorLocator(Poly* out, Poly* synd, bool forQML)
{
    if (forQML)
    {
        //emit setTopText("Szukanie wielomianu wykrycia błędów alogrytmem Berlekamp-Masseya");
        emit setTopTextTranslation(29, {""});
    }
    //this spits out a polynomial in reverse order but i dont know why
    int init = 1;
    Poly errLoc(1, &init);
    Poly oldLoc(1, &init);
    Poly temp;
    int syndShift = 0;
    QString d1 {}, t {};
    for (int i = nsym - 1; i >= 0; i--)
    {
        int K = i + syndShift;
        int delta = synd->coef[K];
        for (int j = 1; j < errLoc.n; j++)
        {
            delta ^= this->gf.mult(errLoc.coef[errLoc.n - j - 1], synd->coef[K + j]);
        }
        if (forQML)
        {
            if (errLoc.n <= 1)
            {
                //emit setBelowText(QString("delta1 = syndrom nr 1"));
                emit setBelowTextTranslation(30, {""});
                emit setBelowTextExtended(QString("delta1 = %1").arg(delta));
            }
            else
            {
                //emit setBelowText(QString("delta2 = syndrom nr 2 + delta1 * syndrom nr 1").arg(i + 1).arg(synd->coef[K + 1]));
                emit setBelowTextTranslation(31, {""});
                emit setBelowTextExtended(QString("delta2 = %1 + %2 * %3 = %4").arg(synd->coef[K]).arg(errLoc.coef[errLoc.n - 2]).arg(synd->coef[K + 1]).arg(delta));
            }
            this->waitForQml();
        }
        Poly_Pad(&oldLoc, 0, 1);
        if (delta != 0)
        {
            if (oldLoc.n > errLoc.n)
            {
                Poly_Scale(&temp, &oldLoc, delta, &this->gf);
                Poly_Scale(&oldLoc, &errLoc, this->gf.inv(delta), &this->gf);
                errLoc.setCopy(temp.n, temp.coef);
                if (forQML)
                {
                    d1 = errLoc.toString();
                    t = oldLoc.toString();
                    emit setBelowText("temp = 1/delta1");
                    emit setBelowTextExtended(QString("temp = 1/%2 = %1").arg(oldLoc.toString()).arg(d1));
                    this->waitForQml();
                }
            }
            Poly_Scale(&temp, &oldLoc, delta, &this->gf);
            Poly_Add(&errLoc, &errLoc, &temp);
            if (forQML && i == 0)
            {
                Poly revErrLoc;
                Poly_Reverse(&revErrLoc, &errLoc);
                //emit setBelowText("Lokator = (delta1 + temp * delta2)x + 1");
                emit setBelowTextTranslation(32, {""});
                emit setBelowTextExtended(QString("(%1 + %3 * %2)x + 1 = %4").arg(d1).arg(delta).arg(t).arg(revErrLoc.toString()));
                this->waitForQml();
            }
        }
    }
    int leading = 0;
    for (; errLoc.coef[leading] == 0; leading++);
    Poly_Trim(&errLoc, leading, 0);
    int errs = errLoc.n - 1;
    out->setCopy(errLoc.n, errLoc.coef);
    if (errs * 2 > nsym)
    {
        return false;
    }
    return true;
}

bool ReedSolomonCode::findErrors(std::vector<unsigned int>* out, Poly* errLoc, int n, bool forQML)
{
    if (forQML)
    {
        //emit setBelowText("Szukanie pozycji błędu");
        emit setBelowTextTranslation(33, {""});
        emit setBelowTextExtended("");
        this->waitForQml();
    }
    int errs = errLoc->n - 1;
    Poly revErrLoc;
    Poly_Reverse(&revErrLoc, errLoc);
    Poly_ChienSearch(out, &revErrLoc, n, &this->gf, forQML);

    if (out->size() != errs)
    {
        // Too many (or few) errors found by Chien Search for the errata locator polynomial!
        return false;
    }
    //map to string pos
    for (int i = 0; i < out->size(); i++)
    {
        if (out->at(i) >= n) //clearly something messed up
        {
            return false;
        }
        (*out)[i] = n - out->at(i) - 1;
    }
    return true;
}

void ReedSolomonCode::forneySyndromes(Poly* out, Poly* synd, int n)
{
    Poly fsynd(synd->n - 1, synd->coef);
    out->setCopy(fsynd.n, fsynd.coef);
}


int GaloisField::multNoLUT(int a, int b)
{
    int ret = 0;
    while (b > 0)
    {
        if (b & 1) //if odd
        {
            ret ^= a;
        }
        b >>= 1;
        a <<= 1;
        if (a > this->characteristic)
        {
            a ^= this->primitivePoly;
        }
    }
    return ret;
}

inline int GaloisField::mult(int a, int b)
{
    return (a == 0 || b == 0) ? 0 : this->powTable[this->logTable[a] + this->logTable[b]];
}
inline int GaloisField::div(int a, int b)
{
    return a == 0 ? 0 : (b == 0) ? -1 : this->powTable[this->logTable[a] - this->logTable[b] + this->characteristic];
}

inline int GaloisField::pow(int x, int power)
{
    return this->powTable[(this->logTable[x] * power) % this->characteristic];
}

inline int GaloisField::inv(int x)
{
    return this->powTable[this->characteristic - this->logTable[x]];
}

inline int GaloisField::sqrt(int x)
{
    return logTable[x] % 2 ? this->powTable[(logTable[x] + this->characteristic) / 2] : this->powTable[logTable[x] / 2];
}

Poly::Poly()
{
    this->init();
}

Poly::Poly(int n, int* data)
{
    this->init();
    this->setCopy(n, data);
}

Poly::~Poly()
{
    if (this->coef)
    {
        free(this->coef);
    }
}

void Poly::init()
{
    this->n = 0;
    this->coef = nullptr;
}

void Poly::setCopy(int n, int* coef)
{
    if (n > this->n)
    {
        if (this->coef)
        {
            free(this->coef);
        }
        this->coef = (int*)malloc(sizeof(int) * n);
    }
    this->n = n;
    if (coef)
    {
        memcpy(this->coef, coef, sizeof(int) * n);
    } else
    {
        memset(this->coef, 0, sizeof(int) * n);
    }
}

void Poly::setRef(int n, int* coef)
{
    if (this->coef)
    {
        free(this->coef);
    }
    this->n = n;
    this->coef = coef;
}

QString Poly::toString()
{
    return ::toString(this->coef, this->n);
}


Poly* Poly_Create(int n, int* coef)
{
    Poly* poly = (Poly*)malloc(sizeof(Poly));
    poly->init();
    poly->setCopy(n, coef);
    return poly;
}

void Poly_Free(Poly* poly)
{
    free(poly->coef);
    free(poly);
}

void Poly_Add(Poly* out, Poly* a, Poly* b)
{
    int n = std::max(a->n, b->n);
    int* temp = (int*)malloc(sizeof(int) * n);
    memset(temp, 0, sizeof(int) * n);
    for (int i = 0; i < a->n; i++)
    {
        temp[i + n - a->n] = a->coef[i];
    }
    for (int i = 0; i < b->n; i++)
    {
        temp[i + n - b->n] ^= b->coef[i];
    }
    out->setRef(n, temp);
}

void Poly_Scale(Poly* out, Poly* in, int scale, GaloisField* gf)
{
    if (out == in)
    {
        for (int i = 0; i < in->n; i++)
        {
            in->coef[i] = gf->mult(in->coef[i], scale);
        }
    } else
    {
        int* temp = (int*)malloc(sizeof(int) * in->n);
        for (int i = 0; i < in->n; i++)
        {
            temp[i] = gf->mult(in->coef[i], scale);
        }
        out->setRef(in->n, temp);
    }
}

void Poly_Mult(Poly* out, Poly* a, Poly* b, GaloisField* gf)
{
    int n = a->n + b->n - 1;
    int* temp = (int*)malloc(sizeof(int) * n);
    memset(temp, 0, sizeof(int) * n);
    for (int i = 0; i < a->n; i++)
    {
        for (int j = 0; j < b->n; j++)
        {
            temp[i + j] ^= gf->mult(a->coef[i], b->coef[j]);
        }
    }
    out->setRef(n, temp);
}

void Poly_Div(Poly* result, Poly* quotient, Poly* remainder, Poly* a, Poly* b, GaloisField* gf)
{
    int* temp = (int*)malloc(sizeof(int)* a->n);
    int normalizer = b->coef[0];
    memcpy(temp, a->coef, sizeof(int) * a->n);
    for (int i = 0; i < a->n - b->n + 1; i++)
    {
        temp[i] = gf->div(temp[i], normalizer);
        int coef = temp[i];
        if (coef != 0)
        {
            for (int j = 1; j < b->n; j++)
            {
                if (b->coef[j] != 0)
                {
                    temp[i + j] ^= gf->mult(b->coef[j], coef);
                }
            }
        }
    }
    if (result)
    {
        result->setCopy(a->n, temp);
    }
    int separator = a->n - b->n + 1;
    if (quotient)
    {
        quotient->setCopy(separator, temp);
    }
    if (remainder)
    {
        remainder->setCopy(b->n - 1, temp + separator);
    }
    free(temp);
}

int Poly_Eval(Poly* poly, int x, GaloisField* gf)
{
    int y = poly->coef[0];
    for (int i = 1; i < poly->n; i++)
    {
        y = gf->mult(y, x) ^ poly->coef[i];
    }
    return y;
}

void ReedSolomonCode::Poly_ChienSearch(std::vector<unsigned int>* out, Poly* poly, int max, GaloisField* gf, bool forQML)
{
    if (forQML)
    {
        //emit setTopText("Szukanie pozycji błędu");
        emit setTopTextTranslation(33, {""});

        //emit setBelowText(QString("Lokator ax + b - błąd na pozycji "));
        emit setBelowTextTranslation(34, {""});
    }

    //this seems unnecessary because all multiplications are performed via lookup table anyway
    int* temp = (int*)malloc(sizeof(int)* poly->n);
    memcpy(temp, poly->coef, sizeof(int) * poly->n);
    for (int i = 0; i < max; i++)
    {
        int eval = Poly_Eval(poly, gf->powTable[i], gf);
        if (eval == 0)
        {
            out->push_back(i);
        }
        if (forQML)
        {

            //emit setBelowText(QString("Wartość wielomianu w punkcie 2^%1").arg(i));
            emit setBelowTextTranslation(35, {QString::number(i)});
            emit setBelowTextExtended(QString("(%1)(%2) = %3").arg(poly->toString()).arg(gf->powTable[i]).arg(eval));
            this->waitForQml();
            if (eval == 0)
            {
                //emit setBelowText(QString("Wartość 0 - błąd na pozycji [długość wiadomości] - %1").arg(i));
                emit setBelowTextTranslation(36, {QString::number(i)});
                emit setBelowTextExtended(QString("%1 - %2 = %3").arg(max).arg(i).arg(max - i));
                this->waitForQml();
                free(temp);
                return;
            }
        }
        // int sum = 0;
        // for (int j = 0; j < poly->n; j++)
        // {
        //     sum ^= temp[j];
        //     temp[j] = gf->mult(temp[j], gf->powTable[poly->n - j - 1]);
        // }
        // if (!sum)
        // {
        //     out->push_back(i);
        // }
    }
    free(temp);
}

void Poly_Pad(Poly* poly, int left, int right)
{
    int n = poly->n + left + right;
    int* temp = (int*)malloc(sizeof(int)* n);
    memset(temp, 0, sizeof(int) * left);
    memcpy(temp + left, poly->coef, sizeof(int) * poly->n);
    memset(temp + (left + poly->n), 0, sizeof(int) * right);
    poly->setRef(n, temp);
}

void Poly_Trim(Poly* poly, int left, int right)
{
    int n = poly->n - left - right;
    int* temp = (int*)malloc(sizeof(int)* n);
    memcpy(temp, poly->coef + left, sizeof(int) * n);
    poly->setRef(n, temp);
}

void Poly_Append(Poly* out, Poly* a, Poly* b)
{
    int n = a->n + b->n;
    int* temp = (int*)malloc(sizeof(int)* n);
    memcpy(temp, a->coef, sizeof(int)* a->n);
    memcpy(temp + a->n, b->coef, sizeof(int)* b->n);
    out->setRef(n, temp);
}

void Poly_Reverse(Poly* out, Poly* in)
{
    int* temp = (int*)malloc(sizeof(int)* in->n);
    for (int i = 0; i < in->n; i++)
    {
        temp[i] = in->coef[in->n - i - 1];
    }
    out->setRef(in->n, temp);
}


unsigned int primes[] = {
    PRIM(0), PRIM(1), PRIM(2), PRIM(3),
    PRIM(4), PRIM(5), PRIM(6), PRIM(7),
    PRIM(8), PRIM(9), PRIM(10), PRIM(11),
    PRIM(12), PRIM(13), PRIM(14), PRIM(15),
    PRIM(16), PRIM(17), PRIM(18), PRIM(19),
    PRIM(20), PRIM(21), PRIM(22), PRIM(23),
    PRIM(24), PRIM(25), PRIM(26), PRIM(27),
    PRIM(28), PRIM(29), PRIM(30), PRIM(31)
};

GaloisField::GaloisField(int fieldPower)
{
    this->characteristic = ((int)1 << fieldPower) - 1;
    this->fieldPower = fieldPower;
    this->primitivePoly = primes[fieldPower];
    //init the tables
    unsigned int val = 1;
    this->powTable = (int*)malloc(sizeof(int) * this->characteristic * 2);
    this->logTable = (int*)malloc(sizeof(int) * (this->characteristic + 1));
    powTable[0] = val;
    logTable[0] = 0;
    logTable[1] = 0;
    for (int i = 1; i < this->characteristic; i++)
    {
        val <<= 1;
        if (val > this->characteristic)
        {
            val ^= this->primitivePoly;
        }
        powTable[i] = (int)val;
        logTable[(int)val] = i;
    }
    for (int i = this->characteristic; i < this->characteristic * 2; i++)
    {
        powTable[i] = powTable[i - this->characteristic];
    }
}

GaloisField::~GaloisField()
{
    free(this->powTable);
    free(this->logTable);
}

