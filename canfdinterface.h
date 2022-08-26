#ifndef CANFDINTERFACE_H
#define CANFDINTERFACE_H

#include <QObject>
#include <QDebug>
#include <QTimer>
#include <QDateTime>
#include <QByteArray>
class canfdinterface : public QObject
{
    Q_OBJECT
    QTimer *rxTimer;
    unsigned gDevType;
    unsigned gDevIdx ;
    unsigned subNetCnt;

    unsigned gChMask = 0;//TODO temp not use it
public:
    explicit canfdinterface(QObject *parent = nullptr);
    virtual ~canfdinterface();
private slots:
    void rxProcess();


public slots:
    void start();
    void transmitFrame(uint chn, quint32 id, QByteArray data);

signals:
    void frameRecieved(QDateTime dt, int channel, quint32 id, QByteArray data);

};

#endif // CANFDINTERFACE_H
