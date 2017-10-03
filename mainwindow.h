#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "midiplayer.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:

    void on_btn_AddSF_clicked();

    void on_btn_Play_clicked();

    void on_btn_Stop_clicked();

    void on_btn_AddMidi_clicked();

    void on_lst_SF2_itemSelectionChanged();

    void on_lst_MIDI_itemSelectionChanged();



private:
    Ui::MainWindow *ui;

    bool loadMidiFromFile(const std::string &filename);
    void stop();
    void play();

    QString shortenString(const QString &filename);
    QString getCurrentSF();
    QString getCurrentSong();
    void updateStatus();

    void scanFolders();
    void scanMidiFolder();
    void scanSFFolder();

    void addMidi(const QString& filename);
    void addSoundFont(const QString& filename);


    // add repeat
    // add shuffle sf2
    // add shuffle midi
    // add pause
    // add playback control (such as current time of the song and total length of song)

    std::string midi;
    std::vector<QString> midi_filenames;
    std::vector<QString> sf_filenames;

    MidiPlayer _midi_player;

};

#endif // MAINWINDOW_H
