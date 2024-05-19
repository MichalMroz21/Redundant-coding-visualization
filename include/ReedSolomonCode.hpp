/**
 * Based on https://en.wikiversity.org/wiki/Reed%E2%80%93Solomon_codes_for_coders and https://github.com/FluffyJay1/ReedSolomon
 */

#ifndef REEDSOLOMONCODE_HPP
#define REEDSOLOMONCODE_HPP

#include <QObject>
#include <QBitArray>
#include <QDebug>

#include <vector>

//defining prime polynomials in GF^x
#define PRIM(x) PRIM_ ## x
#define PRIM_0 0
#define PRIM_1 0
#define PRIM_2 0
#define PRIM_3 0xb
#define PRIM_4 0x13
#define PRIM_5 0x25
#define PRIM_6 0x43
#define PRIM_7 0x83
#define PRIM_8 0x11d
#define PRIM_9 0x211
#define PRIM_10 0x409
#define PRIM_11 0x805
#define PRIM_12 0x1053
#define PRIM_13 0x201b
#define PRIM_14 0x402b
#define PRIM_15 0x8003
#define PRIM_16 0x1002d
#define PRIM_17 0x20009
#define PRIM_18 0x40027
#define PRIM_19 0x80027
#define PRIM_20 0x100009
#define PRIM_21 0x200005
#define PRIM_22 0x400003
#define PRIM_23 0x800021
#define PRIM_24 0x100001b
#define PRIM_25 0x2000009
#define PRIM_26 0x4000047
#define PRIM_27 0x8000027
#define PRIM_28 0x10000009
#define PRIM_29 0x20000005
#define PRIM_30 0x40000053
#define PRIM_31 0x80000009

class GaloisField
{
public:
    int* powTable, *logTable;
    int fieldPower;
    int characteristic, primitivePoly;

    GaloisField(int fieldPower);
    ~GaloisField();
    int multNoLUT(int a, int b);
    inline int mult(int a, int b);
    inline int div(int a, int b);
    inline int pow(int x, int power);
    inline int inv(int x);
    inline int sqrt(int x);
};

void Init();


class Poly
{
public:
    int n;
    int* coef;
    Poly();
    Poly(int n, int* coef);
    ~Poly();

    void init();
    void setCopy(int n, int* coef);
    void setRef(int n, int* coef);
    QString toString();
};

Poly* Poly_Create(int n, int* coef);
void Poly_Free(Poly* poly);
void Poly_Add(Poly* out, Poly* a, Poly* b);
void Poly_Scale(Poly* out, Poly* in, int scale, GaloisField* gf);
void Poly_Mult(Poly* out, Poly* a, Poly* b, GaloisField* gf);
void Poly_Div(Poly* result, Poly* quotient, Poly* remainder, Poly* a, Poly* b, GaloisField* gf);
int Poly_Eval(Poly* poly, int x, GaloisField* gf);
void Poly_Pad(Poly* poly, int left, int right);
void Poly_Trim(Poly* poly, int left, int right);
void Poly_Append(Poly* out, Poly* a, Poly* b);
void Poly_Reverse(Poly* out, Poly* in);


class ReedSolomonCode : public QObject
{
    Q_OBJECT

public:
    explicit ReedSolomonCode(QObject *parent = nullptr);

public slots:
    void setInitialData(QString data, int animationSpeed = 1000, bool infiniteWait = false);
    int correctError(bool forQML);
    void correctErrorQml();
    void encodeDataAsync(bool forQML);
    void encodeData(bool forQML);
    void sendCode(QString sent);
    Poly getRemainder(QString data);
    int getRet(int n, int* coef);

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
    void setTopText(QString text);

    void insertEmptyBit(int arrIndex, int index);
    void insertBit(int arrIndex, int index, QString bit, bool showSymbols);

    void insertArray(int index, QString str, bool showSymbols);

    void setBelowText(QString str);
    void setBelowTextExtended(QString str);
    void setClickAllow(int arrIndex, bool isAllowed);

    void encodingEnd();
    void endErrorCorrection();

private:
    static const int k = 5, nsym = 2, fieldPower = 3, total = k + nsym;


    QString data{}, receivedCode{}, dataEncoded{};
    int dataArr[k], receivedCodeArr[total], dataEncodedArr[total];
    int animationDelayMs{};
    bool infiniteWait = false, buttonPressed = false;
    GaloisField gf;

    void waitForQml();
    void createGenerator(Poly* out, bool forQML);
    void calcSyndromes(Poly* out, Poly* msg, bool forQML);
    bool checkSyndromes(Poly* synd);
    void findErrataLocator(Poly* out, std::vector<unsigned int>* errorPos, bool forQML);
    void findErrorEvaluator(Poly* out, Poly* synd, Poly* errLoc, int nsym, bool forQML);
    bool correctErrata(Poly* msg, Poly* synd, std::vector<unsigned int>* errPos, bool forQML);
    bool findErrorLocator(Poly* out, Poly* synd, bool forQML);
    bool findErrors(std::vector<unsigned int>* out, Poly* errLoc, int n, bool forQML);
    void forneySyndromes(Poly* out, Poly* synd, int n);
    void Poly_ChienSearch(std::vector<unsigned int>* out, Poly* poly, int max, GaloisField* gf, bool forQML);

};

std::vector<unsigned int> toVector(QString s);
QString toString(int* arr, int size);

#endif // REEDSOLOMONCODE_HPP
