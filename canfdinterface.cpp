#include "canfdinterface.h"
#include "zcan.h"
#define RX_BUFF_SIZE  1//接收的最大帧数
#define RX_WAIT_TIME  0//接收超时时间
canfdinterface::canfdinterface(QObject *parent)
    : QObject{parent}
{

    rxTimer = new QTimer(this);
    rxTimer->setInterval(1);
    rxTimer->setSingleShot(false);

    connect(rxTimer, SIGNAL(timeout()), this, SLOT(rxProcess()));

    //    gDevType = SystemConfig::readNodeInt("CanDevType", ZCAN_USBCAN2);//TODO USBCANFD型号代码:33
    //        gDevIdx = SystemConfig::readNodeInt("CanDevIndex", 0);
    //        subNetCnt = SystemConfig::readNodeInt("CanSubNetCount", 2);
    gDevType = 33;
    gDevIdx = 0;
    subNetCnt = 2;
    gChMask = 0;


}

canfdinterface::~canfdinterface()
{
    rxTimer->stop();
    uint closeflag = VCI_CloseDevice(gDevType,gDevIdx);
    if(closeflag == 0){
        qCritical() << tr("CAN设备关闭失败").toUtf8().constData();
    }else{
        qCritical() << tr("CAN设备关闭成功").toUtf8().constData();
    }
}

void canfdinterface::rxProcess()
{

    for(uint chnId=0; chnId<(uint)(subNetCnt-1);chnId++ ){
        ZCAN_FD_MSG rd;//
        uint cnt = VCI_ReceiveFD(gDevType, gDevIdx,chnId,&rd,RX_BUFF_SIZE,RX_WAIT_TIME);
        while(cnt>0){
            QByteArray rxData;
            rxData.resize(rd.hdr.len);
            for(quint8 i=0; i<rd.hdr.len;i++){
                rxData[i] = rd.dat[i];
            }
            emit frameRecieved(QDateTime::currentDateTime(),chnId,rd.hdr.id,rxData);
        }

    }

}

void canfdinterface::start()
{
    //打开can设备
    uint openflag =VCI_OpenDevice(gDevType,gDevIdx, 0);
    if( openflag == 0)
    {
        qCritical() << tr("CAN设备启动失败").toUtf8().constData();
        return;
    }

    //设置比特率
    ZCAN_INIT init;//TODO 设置波特率
    init.clk = 60000000; // clock: 60M
    init.mode = 0;
    init.aset.tseg1 = 46; // 1M
    init.aset.tseg2 = 11;
    init.aset.sjw = 3;
    init.aset.smp = 0;
    init.aset.brp = 0;
    init.dset.tseg1 = 10; // 4M
    init.dset.tseg2 = 2;
    init.dset.sjw = 2;
    init.dset.smp = 0;
    init.dset.brp = 0;
    for(uint i=0; i<(subNetCnt-1);i++){
        //是否添加通道掩码判断gChMask? if ((gChMask & (1 << i)) == 0) continue;
        if(!VCI_InitCAN(gDevType,gDevIdx,i,&init)){
            qCritical() << tr("can通道1%初始化失败").arg(i).toUtf8().constData();
            return;
        }else{
            qCritical() << tr("can通道1%初始化成功").arg(i).toUtf8().constData();
        }
        if(VCI_StartCAN(gDevType,gDevIdx,i)){
            qCritical() << tr("can通道1%开启失败").arg(i).toUtf8().constData();
            return;
        }else{
            qCritical() << tr("can通道1%开启成功").arg(i).toUtf8().constData();
        }
    }
    rxTimer->start();

}

void canfdinterface::transmitFrame(uint chn, quint32 id, QByteArray data)
{
        if(data.length() > 64)//规定发送长度
            return;
        if(chn >subNetCnt)
            return;


        ZCAN_FD_MSG fdMsg;
        fdMsg.hdr.chn = chn;
        fdMsg.hdr.id = id; // TODO 是否标记为扩展帧?id | 0x80000000
        fdMsg.hdr.len = data.length();
        fdMsg.hdr.inf.fmt = 1 ;//0-CAN帧，1-CANFD帧
        fdMsg.hdr.inf.txm = 0;//发送方式，0为正常模式，2为自发自收（仅用于自测）
        fdMsg.hdr.inf.sdf = 0;//0-数据帧，1-远程帧
        fdMsg.hdr.inf.sef = 0;//0-标准帧，1-扩展帧
        fdMsg.hdr.inf.brs = 0;//0-CANFD不加速，1-CANFD加速
        fdMsg.hdr.inf.est =0; //错误状态，0-积极错误，1-消极错误
        memcpy(fdMsg.dat,data.constData(),data.length());
        VCI_TransmitFD(gDevType, gDevIdx, chn, &fdMsg, 1);

}
