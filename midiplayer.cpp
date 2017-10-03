#include "midiplayer.h"
#include <fstream>
#include <QDebug>


MidiPlayer::MidiPlayer()
{
    settings = new_fluid_settings();
    synth = new_fluid_synth(settings);
    player = new_fluid_player(synth);
    adriver = new_fluid_audio_driver(settings, synth);
}

MidiPlayer::~MidiPlayer()
{
    /* cleanup */
    delete_fluid_audio_driver(adriver);
    delete_fluid_player(player);
    delete_fluid_synth(synth);
    delete_fluid_settings(settings);
}

bool MidiPlayer::loadMidiFromFile(const std::string &filename)
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
}

bool MidiPlayer::loadSFFromFile(const std::string &filename)
{
    if (fluid_is_soundfont(filename.c_str()))
    {
        fluid_synth_sfload(synth, filename.c_str(), 1);
        return true;
    }

    return false;
}

void MidiPlayer::stop()
{
    // seems to work. turning sound off on all channels
    fluid_synth_all_sounds_off(synth, -1);
    //fluid_synth_all_notes_off(synth, -1);

    fluid_player_stop(player);
    fluid_synth_stop(synth, 1);

    _is_playing = false;
}

void MidiPlayer::play()
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
    _is_playing = true;

      //if (fluid_player_get_status(player) != FLUID_PLAYER_DONE)
      //  SoundStream::play();


    /* play the midi files, if any */
    //fluid_player_play(player);
    /* wait for playback termination */
    //fluid_player_join(player);
}

void MidiPlayer::addMidi(const QString &filename)
{
    // verify filename before adding
    // add the real path to a private list
    // add just the name to the list (and use the index to get the real name later)
    qDebug() << filename;
    QString name = shortenString(filename);
    qDebug() << name;
    midi_filenames.push_back(filename);
}

void MidiPlayer::addSoundFont(const QString &filename)
{
    // verify filename before adding
    // add the real path to a private list
    // add just the name to the list (and use the index to get the real name later)
    qDebug() << filename;
    QString name = shortenString(filename);
    qDebug() << name;
    midi_filenames.push_back(filename);
}

bool MidiPlayer::getPlayingStatus()
{
    return _is_playing;
}

QString MidiPlayer::shortenString(const QString &filename)
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
