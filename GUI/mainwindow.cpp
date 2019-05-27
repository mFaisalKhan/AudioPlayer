#include "mainwindow.h"
#include "Project-Build/ui_mainwindow.h"
#include <QDebug>
#include  <qevent.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    Ui(new Ui::MainWindow)
{
    Ui->setupUi(this);
    QStringList filters;

    filters.append("*.*");
    filters.append("*.mp3");
    filters.append("*.wav");

    FileDialog.setFilters(filters);
    FileDialog.setFileMode(QFileDialog::ExistingFiles);
    connect(&FileDialog, SIGNAL(filesSelected ( const QStringList &)), this, SLOT(SlotFilesSelected(const QStringList &)));
}

MainWindow::~MainWindow()
{
    delete Ui;
}

void MainWindow::on_LineEditFileName_editingFinished()
{
      emit SignalFileNameTextChanged(Ui->LineEditFileName->text());
}

void MainWindow::on_BtnPlay_clicked()
{
    emit SignalPlayClicked();
}

void MainWindow::on_BtnPause_clicked()
{
    emit SignalPauseClicked();
}

void MainWindow::on_BtnStop_clicked()
{
     emit SignalStopClicked();
}

void MainWindow::on_DialVolume_sliderMoved(int position)
{
    emit SignalSliderMoved(position);
}

void MainWindow::SlotVersionInfo(QString version)
{
    //Ui->textEdit->setText(version);
}

void MainWindow::SlotPlayStatus(bool isPlaying)
{
   // PlayStatus += status + "\n";
   // Ui->TxtEditStatus->setPlainText(PlayStatus);

        Ui->BtnPlay->setEnabled(!isPlaying);
        Ui->BtnPause->setEnabled(isPlaying);
        Ui->BtnStop->setEnabled(isPlaying);

}

void MainWindow::SlotVolumeChanged(int x)
{
    Ui->DialVolume->setValue(x);
}

void MainWindow::SlotUpdateSongInfo(QString album , QString title)
{
    Ui->txtTrackInfo->setText("Album: "+album + "Title: "+title);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    emit SignalGuiClosed();
    event->accept();
}


void MainWindow::on_BtnFileBowser_clicked()
{
    FileDialog.show();

}

void MainWindow::SlotFilesSelected(const QStringList &selected)
{

    Ui->LineEditFileName->setText(selected.at(0));
    if(selected.count()>1)
    {
        for(int i=0; i<selected.count();i++)
        {
           // Ui->ListSongs->append();
        }
    }
}
