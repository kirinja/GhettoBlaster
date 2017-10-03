#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <fluidsynth.h>
#include <QDebug>

#include <QFileDialog>
#include <fstream>

fluid_settings_t* settings;
fluid_synth_t* synth;
fluid_player_t* player;
fluid_audio_driver_t* adriver;

bool font_loaded = false;
bool midi_loaded = false;

bool played_first_time = false;
bool isPlaying = false;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    settings = new_fluid_settings();
    synth = new_fluid_synth(settings);
    player = new_fluid_player(synth);
    adriver = new_fluid_audio_driver(settings, synth);
    scanFolders();
}


MainWindow::~MainWindow()
{
    /* cleanup */
    delete_fluid_audio_driver(adriver);
    delete_fluid_player(player);
    delete_fluid_synth(synth);
    delete_fluid_settings(settings);
    delete ui;
}

void MainWindow::play()
{
    int status = fluid_player_get_status(player);

    if (status == FLUID_PLAYER_READY)
        fluid_player_play(player);

    if (true && status == FLUID_PLAYER_DONE)
    {
        fluid_player_stop(player);
        delete_fluid_player(player);
        player = new_fluid_player(synth);
        if (player == NULL)
        {
          //setError("Failed to play MIDI.");
          return;
        }
        fluid_player_add_mem(player, midi.c_str(), midi.size());
        fluid_player_play(player);
    }
    isPlaying = true;

      //if (fluid_player_get_status(player) != FLUID_PLAYER_DONE)
      //  SoundStream::play();


    /* play the midi files, if any */
    //fluid_player_play(player);
    /* wait for playback termination */
    //fluid_player_join(player);
}

void MainWindow::stop()
{
    // seems to work. turning sound off on all channels
    fluid_synth_all_sounds_off(synth, -1);
    //fluid_synth_all_notes_off(synth, -1);

    fluid_player_stop(player);
    fluid_synth_stop(synth, 1);

    isPlaying = false;
}

// check if we're playing and do stuff accordingly
// also disable this button until we have both a soundfont and midi selected
void MainWindow::on_btn_Play_clicked()
{
    //_midi_player.play();
    play();
    played_first_time = true;

    updateStatus();
}


void MainWindow::on_btn_Stop_clicked()
{
    //_midi_player.stop();
    stop();
    played_first_time = false;

    updateStatus();
}


// Opens dialog window to load soundfonts and add to the listview (have to remove all the other shit so it's neater to look at)
void MainWindow::on_btn_AddSF_clicked()
{
    QStringList fileNames = QFileDialog::getOpenFileNames(this,
        tr("Add SoundFont"), "", tr("SF2 Files (*.sf2)"));

    for (int i = 0; i < fileNames.count(); i++)
    {
        // verify filename before adding
        // add the real path to a private list
        // add just the name to the list (and use the index to get the real name later)
        qDebug() << fileNames.at(i);
        QString name = shortenString(fileNames[i]);
        qDebug() << name;
        sf_filenames.push_back(fileNames[i]);
        ui->lst_SF2->addItem(name);


    }
}


void MainWindow::on_btn_AddMidi_clicked()
{
    QStringList fileNames = QFileDialog::getOpenFileNames(this,
        tr("Add MIDI"), "", tr("MIDI Files (*.mid *.midi)"));

    for (int i = 0; i < fileNames.count(); i++)
    {
        addMidi(fileNames.at(i));
    }
}

void MainWindow::on_lst_SF2_itemSelectionChanged()
{
    QString selectedName = sf_filenames[ui->lst_SF2->currentRow()];

    if (fluid_is_soundfont(selectedName.toStdString().c_str()))
    {
        fluid_synth_sfload(synth, selectedName.toStdString().c_str(), 1);

        font_loaded = true;
    }
    else
    {
        font_loaded = false;
    }

    if (midi_loaded && font_loaded)
    {
        ui->btn_Play->setEnabled(true);
        ui->btn_Stop->setEnabled(true);
    }

    updateStatus();
}


void MainWindow::on_lst_MIDI_itemSelectionChanged()
{
    QString selectedName = midi_filenames[ui->lst_MIDI->currentRow()];

    if (fluid_is_midifile(selectedName.toStdString().c_str()))
    {
        // have to look over this
        loadMidiFromFile(selectedName.toStdString()); // this doesnt work properly when deploying (only works in editor)
        //if (played_first_time)
        if (ui->chk_AutoPlay->isChecked())
            play();

        midi_loaded = true;
    }
    else
    {
        midi_loaded = false;
    }

    if (midi_loaded && font_loaded)
    {
        ui->btn_Play->setEnabled(true);
        ui->btn_Stop->setEnabled(true);
    }

    updateStatus();
}

// this works somewhat
// instead of loading the midi file we load the memory
// it's not complete though

bool MainWindow::loadMidiFromFile(const std::string& filename)
{
  stop();

  if (player != NULL)
    delete_fluid_player(player);

  player = new_fluid_player(synth);

  if (player == NULL)
    return false;

  // it crashed because not all dll files were included correctly
  std::ifstream file(filename.c_str(), std::ios::binary);
  if (!file.is_open())
  {
    //setError("Failed to load MIDI.");
    return false;
  }

  file.seekg(0, std::ios::end);
  unsigned int size = static_cast<unsigned int>(file.tellg());
  file.seekg(0, std::ios::beg);

  char* data = new char[size];
  if (data == NULL)
  {
    //setError("Failed to load MIDI.");
    return false;
  }

  file.read(data, size);

  midi = std::string(data, size);
  delete[] data;
  file.close();

  if (fluid_player_add_mem(player, midi.c_str(), midi.size()) != FLUID_OK)
    return false;

  return true;

  // working solution with QFile instead of ifstream
  /*
  QFile file(filename);
  if (!file.open(QIODevice::ReadOnly ))
       return false;

  unsigned int size = file.size();
  char* data = new char[size];
  if (data == NULL)
      return false;

  // seems like it's reading 0 bytes of data. Need to look into
  auto b = file.read(data, size);
  qDebug() << "qfile read " << b;
  midi = std::string(data, size);
  delete[] data;
  file.close();

  qDebug() << "printing data? " << midi.c_str();

  if (fluid_player_add_mem(player, midi.c_str(), midi.size()) !=
      FLUID_OK) {
    return false;
  }
 */
}

QString MainWindow::shortenString(const QString &filename)
{
    // start backwards and remove only keep the last part of the name (stored backwards at first, then we flip it)

    QString flippedName;

    for (int i = filename.length(); i > 0; i--)
    {
        if (filename[i] == '/' || filename[i] == '\\')
            break;
        else
            flippedName += filename[i];
    }

    // next we want to flip the name
    QString normalName;
    for (int i = flippedName.length(); i > 0; i--)
    {
        normalName += flippedName[i];
    }

    return normalName;
}

void MainWindow::addMidi(const QString &filename)
{
    // verify filename before adding
    // add the real path to a private list
    // add just the name to the list (and use the index to get the real name later)
    qDebug() << filename;
    QString name = shortenString(filename);
    qDebug() << name;
    midi_filenames.push_back(filename);
    ui->lst_MIDI->addItem(name);
}

void MainWindow::addSoundFont(const QString &filename)
{
    // verify filename before adding
    // add the real path to a private list
    // add just the name to the list (and use the index to get the real name later)
    qDebug() << filename;
    QString name = shortenString(filename);
    qDebug() << name;
    midi_filenames.push_back(filename);
    ui->lst_SF2->addItem(name);
}

QString MainWindow::getCurrentSF()
{
    QString current = "No Soundfont";
    if (ui->lst_SF2->currentItem() != nullptr)
        current = ui->lst_SF2->currentItem()->text();
    return current;
}

QString MainWindow::getCurrentSong()
{
    QString current = "No Song";
    if (ui->lst_MIDI->currentItem() != nullptr)
        current = ui->lst_MIDI->currentItem()->text();

    //if (midiSelected != -1)
    //    current = ui->lst_MIDI->item(midiSelected)->text();
    return current;
}

void MainWindow::updateStatus()
{
    QString playingStatus = _midi_player.getPlayingStatus() ? "Playing " : "Stopped ";
    ui->lbl_Status->setText(playingStatus + getCurrentSong() + " using " + getCurrentSF());
}

void MainWindow::scanFolders()
{
    scanMidiFolder();
    scanSFFolder();
}

// BUGGY
void MainWindow::scanMidiFolder()
{
    /*
    QDirIterator dirIt("/midis", QDirIterator::Subdirectories);
    while (dirIt.hasNext())
    {
        dirIt.next();
        if (QFileInfo(dirIt.filePath()).isFile())
            if (QFileInfo(dirIt.filePath()).suffix() == "mid" || QFileInfo(dirIt.filePath()).suffix() == "midi")
                qDebug()<<dirIt.filePath();
    }
     */

    QStringList nameFilter("*.mid");
    QDir directory(QDir::currentPath() + "/midis/");
    QStringList midFiles = directory.entryList(nameFilter);
    for (int i = 0; i < midFiles.size(); i++)
    {
        //qDebug() << midFiles.at(i);
        //addMidi(midFiles.at(i));
        qDebug() << midFiles.at(i);
        QString name = shortenString(midFiles.at(i));
        qDebug() << name;
        midi_filenames.push_back(midFiles.at(i));
        ui->lst_MIDI->addItem(name);
    }

    QStringList nameFilter2("*.midi");
    QStringList midiFiles = directory.entryList(nameFilter2);
    for (int i = 0; i < midiFiles.size(); i++)
    {
        //qDebug() << midiFiles.at(i);
        //addMidi(midiFiles.at(i));
        qDebug() << midFiles.at(i);
        QString name = shortenString(midFiles.at(i));
        qDebug() << name;
        midi_filenames.push_back(midFiles.at(i));
        ui->lst_MIDI->addItem(name);
    }
}

void MainWindow::scanSFFolder()
{/*
    QDirIterator dirIt("/soundfonts",QDirIterator::Subdirectories);
    while (dirIt.hasNext())
    {
        dirIt.next();
        if (QFileInfo(dirIt.filePath()).isFile())
            if (QFileInfo(dirIt.filePath()).suffix() == "sf2")
                qDebug()<<dirIt.filePath();
    }*/
/*
    QStringList nameFilter("*.sf2");
    QDir directory(QDir::currentPath() + "/soundfonts");
    QStringList sfFiles = directory.entryList(nameFilter);
    for (int i = 0; i < sfFiles.size(); i++)
    {
        QString name = shortenString(sfFiles[i]);
        qDebug() << name;
        sf_filenames.push_back(sfFiles[i]);
        ui->lst_SF2->addItem(name);
    }*/
}
