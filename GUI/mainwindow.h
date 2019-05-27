#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *Ui;
    QString PlayStatus;
    void closeEvent(QCloseEvent *);
    QFileDialog FileDialog;

private slots:
    void on_LineEditFileName_editingFinished();
    void on_BtnPlay_clicked();
    void on_BtnPause_clicked();
    void on_BtnStop_clicked();
    void on_DialVolume_sliderMoved(int position);
    void SlotVersionInfo(QString);
    void SlotPlayStatus(bool);
    void SlotVolumeChanged(int x);
    void SlotUpdateSongInfo(QString, QString);

    void on_BtnFileBowser_clicked();
    void SlotFilesSelected ( const QStringList & selected );

signals:
    void SignalFileNameTextChanged(QString);
    void SignalPlayClicked();
    void SignalPauseClicked();
    void SignalStopClicked();
    void SignalSliderMoved(int);
    void SignalGuiClosed();
};

#endif // MAINWINDOW_H
