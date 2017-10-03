#ifndef MIDIPLAYER_H
#define MIDIPLAYER_H

#include "fluidsynth.h"
#include <QString>

class MidiPlayer
{
public:
    MidiPlayer();
    ~MidiPlayer();
    bool loadMidiFromFile(const std::string &filename);
    bool loadSFFromFile(const std::string &filename);
    void stop();
    void play();

    void addMidi(const QString& filename);
    void addSoundFont(const QString& filename);

    bool getPlayingStatus();

private:
    std::string midi;
    std::vector<QString> midi_filenames;
    std::vector<QString> sf_filenames;

    QString shortenString(const QString &filename);

    bool _is_playing;

    fluid_settings_t* settings;
    fluid_synth_t* synth;
    fluid_player_t* player;
    fluid_audio_driver_t* adriver;
};

#endif // MIDIPLAYER_H
