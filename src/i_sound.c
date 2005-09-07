// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: i_sound.c 87 2005-09-07 22:24:26Z fraggle $
//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005 Simon Howard
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
// 02111-1307, USA.
//
// $Log$
// Revision 1.17  2005/09/07 22:24:26  fraggle
// Modify the sound effect caching behaviour: sounds which are not playing
// are now marked as PU_CACHE; it is otherwise possible to run out of memory.
//
// Revision 1.16  2005/09/06 22:39:43  fraggle
// Restore -nosound, -nosfx, -nomusic
//
// Revision 1.15  2005/09/06 21:40:28  fraggle
// Setting music volume
//
// Revision 1.14  2005/09/06 21:11:23  fraggle
// Working music!
//
// Revision 1.13  2005/09/05 22:50:56  fraggle
// Add mmus2mid code from prboom.  Use 'void *' for music handles.  Pass
// length of data when registering music.
//
// Revision 1.12  2005/09/05 21:03:43  fraggle
// 16-bit sound
//
// Revision 1.11  2005/09/05 20:32:18  fraggle
// Use the system-nonspecific sound code to assign the channel number used
// by SDL.  Remove handle tagging stuff.
//
// Revision 1.10  2005/08/19 21:55:51  fraggle
// Make sounds louder.  Use the correct maximum of 15 when doing sound
// calculations.
//
// Revision 1.9  2005/08/07 19:21:01  fraggle
// Cycle round sound channels to stop reuse and conflicts of channel
// numbers.  Add debug to detect when incorrect sound handles are used.
//
// Revision 1.8  2005/08/06 17:05:51  fraggle
// Remove debug messages, send error messages to stderr
// Fix overflow when playing large sound files
//
// Revision 1.7  2005/08/05 17:53:07  fraggle
// More sensible defaults
//
// Revision 1.6  2005/08/04 21:48:32  fraggle
// Turn on compiler optimisation and warning options
// Add SDL_mixer sound code
//
// Revision 1.5  2005/07/23 21:32:47  fraggle
// Add missing errno.h, fix crash on startup when no IWAD present
//
// Revision 1.4  2005/07/23 19:17:11  fraggle
// Use ANSI-standard limit constants.  Remove LINUX define.
//
// Revision 1.3  2005/07/23 17:21:35  fraggle
// Remove step table (unused, adds dependency on pow function)
//
// Revision 1.2  2005/07/23 16:44:55  fraggle
// Update copyright to GNU GPL
//
// Revision 1.1.1.1  2005/07/23 16:20:46  fraggle
// Initial import
//
//
// DESCRIPTION:
//	System interface for sound.
//
//-----------------------------------------------------------------------------

static const char
rcsid[] = "$Id: i_sound.c 87 2005-09-07 22:24:26Z fraggle $";

#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>
#include <SDL_mixer.h>

#ifndef _WIN32
#include <unistd.h>
#endif

#include "mmus2mid.h"
#include "z_zone.h"

#include "i_system.h"
#include "i_sound.h"
#include "m_argv.h"
#include "m_misc.h"
#include "m_swap.h"
#include "w_wad.h"

#include "doomdef.h"

#define NUM_CHANNELS		16

static boolean sound_initialised = false;
static Mix_Chunk sound_chunks[NUMSFX];
static int channels_playing[NUM_CHANNELS];


// When a sound stops, check if it is still playing.  If it is not, 
// we can mark the sound data as CACHE to be freed back for other
// means.

void ReleaseSoundOnChannel(int channel)
{
    int i;
    int id = channels_playing[channel];

    if (!id)
        return;

    channels_playing[channel] = sfx_None;
    
    for (i=0; i<NUM_CHANNELS; ++i)
    {
        // Playing on this channel? if so, don't release.

        if (channels_playing[i] == id)
            return;
    }

    // Not used on any channel, and can be safely released
    
    Z_ChangeTag(sound_chunks[id].abuf, PU_CACHE);
}

// Expands the 11025Hz, 8bit, mono sound effects in Doom to
// 22050Hz, 16bit stereo

static void ExpandSoundData(byte *data, int samplerate, int length,
                            Mix_Chunk *destination)
{
    byte *expanded = (byte *) destination->abuf;
    int i;

    if (samplerate == 11025)
    {
        // need to expand to 2 channels, 11025->22050 and 8->16 bit

        for (i=0; i<length; ++i)
        {
            Uint16 sample;

            sample = data[i] | (data[i] << 8);
            sample -= 32768;

            expanded[i * 8] = expanded[i * 8 + 2]
              = expanded[i * 8 + 4] = expanded[i * 8 + 6] = sample & 0xff;
            expanded[i * 8 + 1] = expanded[i * 8 + 3]
              = expanded[i * 8 + 5] = expanded[i * 8 + 7] = (sample >> 8) & 0xff;
        }
    }
    else if (samplerate == 22050)
    {
        for (i=0; i<length; ++i)
        {
            Uint16 sample;

            sample = data[i] | (data[i] << 8);
            sample -= 32768;

            expanded[i * 4] = expanded[i * 4 + 2] = sample & 0xff;
            expanded[i * 4 + 1] = expanded[i * 4 + 3] = (sample >> 8) & 0xff;
        }
    }
    else
    {
        I_Error("Unsupported sample rate %i", samplerate);
    }
}

// Load and convert a sound effect

static void CacheSFX(int sound)
{
    int lumpnum;
    int samplerate;
    int length;
    int expanded_length;
    byte *data;

    // need to load the sound

    lumpnum = I_GetSfxLumpNum(&S_sfx[sound]);
    data = W_CacheLumpNum(lumpnum, PU_STATIC);

    samplerate = (data[3] << 8) | data[2];
    length = (data[5] << 8) | data[4];
    expanded_length = (length * 4)  * (22050 / samplerate);

    sound_chunks[sound].allocated = 1;
    sound_chunks[sound].alen = expanded_length;
    sound_chunks[sound].abuf 
        = Z_Malloc(expanded_length, PU_STATIC, &sound_chunks[sound].abuf);
    sound_chunks[sound].volume = 64;

    ExpandSoundData(data + 8, samplerate, length, &sound_chunks[sound]);

    // don't need the original lump any more
  
    Z_ChangeTag(data, PU_CACHE);
}

static Mix_Chunk *getsfx(int sound)
{
    if (sound_chunks[sound].abuf == NULL)
    {
        CacheSFX(sound);
    }
    else
    {
        // don't free the sound while it is playing!
   
        Z_ChangeTag(sound_chunks[sound].abuf, PU_STATIC);
    }

    return &sound_chunks[sound];
}

//
// SFX API
// Note: this was called by S_Init.
// However, whatever they did in the
// old DPMS based DOS version, this
// were simply dummies in the Linux
// version.
// See soundserver initdata().
//
void I_SetChannels()
{
}	

 
void I_SetSfxVolume(int volume)
{
    // Identical to DOS.
    // Basically, this should propagate
    //  the menu/config file setting
    //  to the state variable used in
    //  the mixing.
    snd_SfxVolume = volume;
}

// MUSIC API - dummy. Some code from DOS version.
void I_SetMusicVolume(int volume)
{
    // Internal state variable.
    snd_MusicVolume = volume;

    Mix_VolumeMusic((volume * MIX_MAX_VOLUME) / 15);
}


//
// Retrieve the raw data lump index
//  for a given SFX name.
//
int I_GetSfxLumpNum(sfxinfo_t* sfx)
{
    char namebuf[9];
    sprintf(namebuf, "ds%s", sfx->name);
    return W_GetNumForName(namebuf);
}
//
// Starting a sound means adding it
//  to the current list of active sounds
//  in the internal channels.
// As the SFX info struct contains
//  e.g. a pointer to the raw data,
//  it is ignored.
// As our sound handling does not handle
//  priority, it is ignored.
// Pitching (that is, increased speed of playback)
//  is set, but currently not used by mixing.
//
int
I_StartSound
( int		id,
  int           channel,
  int		vol,
  int		sep,
  int		pitch,
  int		priority )
{
    Mix_Chunk *chunk;

    if (!sound_initialised)
        return 0;

    // Release a sound effect if there is already one playing
    // on this channel

    ReleaseSoundOnChannel(channel);

    // Get the sound data

    chunk = getsfx(id);

    // play sound

    Mix_PlayChannelTimed(channel, chunk, 0, -1);

    channels_playing[channel] = id;

    // set separation, etc.
 
    I_UpdateSoundParams(channel, vol, sep, pitch);

    return channel;
}

void I_StopSound (int handle)
{
    if (!sound_initialised)
        return;

    Mix_HaltChannel(handle);

    // Sound data is no longer needed; release the
    // sound data being used for this channel

    ReleaseSoundOnChannel(handle);
}


int I_SoundIsPlaying(int handle)
{
    if (!sound_initialised) 
        return false;

    return Mix_Playing(handle);
}




// 
// Periodically called to update the sound system
//

void I_UpdateSound( void )
{
    int i;

    if (!sound_initialised)
        return;

    // Check all channels to see if a sound has finished

    for (i=0; i<NUM_CHANNELS; ++i)
    {
        if (channels_playing[i] && !I_SoundIsPlaying(i))
        {
            // Sound has finished playing on this channel,
            // but sound data has not been released to cache
            
            ReleaseSoundOnChannel(i);
        }
    }
}


// 
// This would be used to write out the mixbuffer
//  during each game loop update.
// Updates sound buffer and audio device at runtime. 
// It is called during Timer interrupt with SNDINTR.
// Mixing now done synchronous, and
//  only output be done asynchronous?
//
void
I_SubmitSound(void)
{
}



void
I_UpdateSoundParams
( int	handle,
  int	vol,
  int	sep,
  int	pitch)
{
    int left, right;

    if (!sound_initialised)
        return;

    left = ((254 - sep) * vol) / 15;
    right = ((sep) * vol) / 15;

    Mix_SetPanning(handle, left, right);
}




void I_ShutdownSound(void)
{    
    if (!sound_initialised)
        return;

    Mix_CloseAudio();
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
}



void
I_InitSound()
{ 
    int i;
    
    // No sounds yet

    for (i=0; i<NUMSFX; ++i)
    {
        sound_chunks[i].abuf = NULL;
    }

    for (i=0; i<NUM_CHANNELS; ++i)
    {
        channels_playing[i] = sfx_None;
    }
    
    // If music or sound is going to play, we need to at least
    // initialise SDL

    if (M_CheckParm("-nosound") 
     || (M_CheckParm("-nosfx") && M_CheckParm("-nomusic")))
        return;

    if (SDL_Init(SDL_INIT_AUDIO) < 0)
    {
        fprintf(stderr, "Unable to set up sound.\n");
        return;
    }

    if (Mix_OpenAudio(22050, AUDIO_S16LSB, 2, 1024) < 0)
    {
        fprintf(stderr, "Error initialising SDL_mixer: %s\n", SDL_GetError());
    }

    Mix_AllocateChannels(NUM_CHANNELS);
    
    SDL_PauseAudio(0);

    if (M_CheckParm("-nosound") || M_CheckParm("-nosfx"))
        return;

    sound_initialised = true;
}




//
// MUSIC API.
//

static int music_initialised;

void I_InitMusic(void)		
{ 
    if (M_CheckParm("-nomusic") || M_CheckParm("-nosound"))
        return;

    music_initialised = true;
}

void I_ShutdownMusic(void)	
{ 
    music_initialised = false;
}

static int	looping=0;
static int	musicdies=-1;

void I_PlaySong(void *handle, int looping)
{
    Mix_Music *music = (Mix_Music *) handle;
    int loops;

    if (!music_initialised)
        return;

    if (handle == NULL)
        return;

    if (looping)
        loops = -1;
    else
        loops = 1;

    Mix_PlayMusic(music, loops);
}

void I_PauseSong (void *handle)
{
    if (!music_initialised)
        return;

    Mix_PauseMusic();
}

void I_ResumeSong (void *handle)
{
    if (!music_initialised)
        return;

    Mix_ResumeMusic();
}

void I_StopSong(void *handle)
{
    if (!music_initialised)
        return;

    Mix_HaltMusic();
}

void I_UnRegisterSong(void *handle)
{
    Mix_Music *music = (Mix_Music *) handle;

    if (!music_initialised)
        return;

    if (handle == NULL)
        return;

    Mix_FreeMusic(music);
}

void *I_RegisterSong(void *data, int len)
{
    char filename[64];
    Mix_Music *music;
    MIDI *mididata;
    UBYTE *mid;
    int midlen;

    if (!music_initialised)
        return NULL;
    
#ifdef _WIN32
    sprintf(filename, "doom.mid");
#else
    sprintf(filename, "/tmp/doom-%i.mid", getpid());
#endif

    // Convert from mus to midi
    // Bits here came from PrBoom
  
    mididata = Z_Malloc(sizeof(MIDI), PU_STATIC, 0);
    mmus2mid(data, mididata, 89, 0);

    if (MIDIToMidi(mididata, &mid, &midlen))
    {
        // Error occurred

        fprintf(stderr, "Error converting MUS lump.\n");

        music = NULL;
    }
    else
    {
        // Write midi data to disk
       
        M_WriteFile(filename, mid, midlen);

        // Clean up
       
        free(mid);
        free_mididata(mididata);
        music = Mix_LoadMUS(filename);
        
        if (music == NULL)
        {
            // Failed to load

            fprintf(stderr, "Error loading midi\n");
        }
    }

    Z_Free(mididata);
    
    return music;
}

// Is the song playing?
int I_QrySongPlaying(void *handle)
{
    if (!music_initialised)
        return false;

    return Mix_PlayingMusic();
}



