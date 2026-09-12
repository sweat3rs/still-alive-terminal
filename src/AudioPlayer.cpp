#include "AudioPlayer.h"

using namespace term;

#define SECONDS_TO_MILLISECONDS(seconds) (uint64_t((seconds)*1000.0))

AudioPlayer::AudioPlayer() 
{
    m_config = ma_engine_config_init();
    m_errorState = ma_engine_init(&m_config, &m_audioEngine);
}

AudioPlayer::~AudioPlayer()
{
    if (m_errorState != MA_SUCCESS) return;
    ma_sound_stop(&m_sound);
}

void AudioPlayer::LoadSoundFromFile(const std::string& file)
{
    if (m_errorState != MA_SUCCESS) return;
    m_errorState = ma_sound_init_from_file(&m_audioEngine, file.c_str(), 0, NULL, NULL, &m_sound);
}

void AudioPlayer::StartSound(double delaySeconds)
{
    if (m_errorState != MA_SUCCESS) return;
    ma_sound_set_start_time_in_milliseconds(&m_sound, SECONDS_TO_MILLISECONDS(delaySeconds));
}

void AudioPlayer::StopSound(double delaySeconds)
{
    if (m_errorState != MA_SUCCESS) return;
    ma_sound_set_stop_time_in_milliseconds(&m_sound, SECONDS_TO_MILLISECONDS(delaySeconds));
}