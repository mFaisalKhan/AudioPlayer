#ifndef DEVICEINFO_H
#define DEVICEINFO_H

#include <QApplication>
#include "AudioPlayer/ALSAWrapper/ALSAWrapper.h"
//using namespace AlsaWrapperSpace;

class DeviceInfo : public QObject
{
    Q_OBJECT
public:
    explicit DeviceInfo(QObject *parent = 0);
    void Init();
    
signals:
    void SignalVersionInfo(QString);

public slots:


private:
    //    AlsaWrapperSpace::ALSAWrapper  Alsa;
    
};

#endif // DEVICEINFO_H
