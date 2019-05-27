
#include "Factory.h"
#include <QDebug>

////////////////////////////////////////////////////////////////////////////////////////////
Factory::Factory()
    :QObject()
    ,GuiWindow(NULL)
    ,AlsaWrapper(NULL)
    ,APlayer(NULL)
    ,AudioDeviceInfo(NULL)
{
    GuiWindow = new MainWindow();
    AlsaWrapper = new ALSAWrapper();
    APlayer = new AudioPlayer(AlsaWrapper);
    AudioDeviceInfo = new DeviceInfo();

    MakeConnections();

    AudioDeviceInfo->Init();
    GuiWindow->show();
}

////////////////////////////////////////////////////////////////////////////////////////////
Factory::~Factory()
{
    if(AudioDeviceInfo)
    {
        delete AudioDeviceInfo;
        AudioDeviceInfo = NULL;
    }
    if(APlayer)
    {
        delete APlayer;
        APlayer = NULL;
    }
    if(AlsaWrapper)
    {
        AlsaWrapper->exit();
        delete AlsaWrapper;
        AlsaWrapper = NULL;
    }
    if(GuiWindow)
    {
        delete GuiWindow;
        GuiWindow = NULL;
    }
}


////////////////////////////////////////////////////////////////////////////////////////////
bool Factory::IsQuiting()
{
    return Quiting;
}

////////////////////////////////////////////////////////////////////////////////////////////
void Factory::ProcessEvents()
{
    APlayer->ProcessEvents();
}

////////////////////////////////////////////////////////////////////////////////////////////
void Factory::SlotGuiClosed()
{
    APlayer->MainWindowClosing();
    Quiting = true;
}

////////////////////////////////////////////////////////////////////////////////////////////
void Factory::MakeConnections()
{
    QObject::connect(GuiWindow, SIGNAL(SignalFileNameTextChanged(QString)), APlayer, SLOT(SlotFileNameChanged(QString)));
    QObject::connect(GuiWindow, SIGNAL(SignalPlayClicked()),  APlayer, SLOT(SlotPlayClicked()));
    QObject::connect(GuiWindow, SIGNAL(SignalPauseClicked()), APlayer, SLOT(SlotPauseClicked()));
    QObject::connect(GuiWindow, SIGNAL(SignalStopClicked()),  APlayer, SLOT(SlotStopClicked()));
    QObject::connect(GuiWindow, SIGNAL(SignalSliderMoved(int)),  APlayer, SLOT(SlotSliderMoved(int)));

    QObject::connect(AudioDeviceInfo, SIGNAL(SignalVersionInfo(QString)),GuiWindow, SLOT(SlotVersionInfo(QString)));
    QObject::connect(APlayer, SIGNAL(SignalPlayStatus(bool)),  GuiWindow, SLOT(SlotPlayStatus(bool)));
    QObject::connect(AlsaWrapper, SIGNAL(VolumeChanged (int)), GuiWindow, SLOT(SlotVolumeChanged(int)));

    QObject::connect(GuiWindow, SIGNAL(SignalGuiClosed()), this, SLOT(SlotGuiClosed()));
    QObject::connect(APlayer, SIGNAL(SignalUpdateSongInfo(QString, QString)),GuiWindow, SLOT(SlotUpdateSongInfo(QString,QString)));

}
