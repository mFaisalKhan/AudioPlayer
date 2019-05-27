#ifndef FACTORY_H
#define FACTORY_H
#include <QObject>
#include "GUI/mainwindow.h"
#include "AudioPlayer/AudioPlayer.h"
#include "AudioPlayer/DeviceInfo.h"

class Factory:public QObject
{
    Q_OBJECT
public:
    Factory();
    ~Factory();
    bool IsQuiting();
    void ProcessEvents();

public slots:
    void SlotGuiClosed();

private:
    void MakeConnections();
    bool Quiting;

    MainWindow *GuiWindow;
    ALSAWrapper *AlsaWrapper;
    AudioPlayer *APlayer;
    DeviceInfo *AudioDeviceInfo;
};

#endif // FACTORY_H
