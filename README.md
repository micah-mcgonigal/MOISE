MOISE (Musical, Open and Interactive Sequence Engine) is a music creation tool and playback engine. It's not only designed for the creation and playback of music tracks note-by-note at runtime, but also to allow highly controllable interactivity by allowing every single music command (Note on, voice change, tempo change, etc.) to be given a condition that controls if and how it is called. This is the key of MOISE. With this capability music can easily be composed with dynamic features such as seamless transitions that maintain tempo between tracks, seamless endings that can trigger at any point in a song, cahnging key and tempo independently of each other based on game conditions, changing melodies based on game conditions, synchronized music stings that override lead instruments, etc.

MOISE is very early in development but I am interested in collaborating if anyone is intersted!
 
Playing back individual notes at runtime allows for changes to music that would be completely impractical, if not impossible, to achieve with modern audio middleware such as Wwise and FMOD, as those are focused on streamed audio. Seamless transitions that could occur at the end of any measure in a track, for example, would require music files to be separated into individual measures. Changing tempo and pitch independently is impossible with streamed music without doing something drastic such as rendering every possible combination as individual streamed music files.
 
Here are some more specific examples of how MOISE could be used to create a dynamic soundtrack within the context of, for example, a personal watercraft racing game (For example, Wave Race 64):
 
* Seamlessly transitions into different parts of a song depending on lap.
  - For example, a song might loop as normal for the first two laps, but on the final lap it will seamlessly transition into a new part of the track. A transition can be composed for each measure of the base track so that the change can happen immediately regardless of current playback position.
  - With streamed music files, every measure would need to be rendered separately as would each transition into the new part of the track.
* Similarly, there could be a finale it seamlessly transitions to once the race is finished.
* Channels can fade in and out of the track depending on the player's position.
  - For example, 1st might have some extra drums that aren't heard in other tracks, while last might have an extra instrument making the music more inspiring, encouraging the player not to give up.
  - This is something that can easily be achieved with streamed music as well by fading tracks in and out.
* Some sound effects can adjust in pitch and tempo to match the key and tempo of the music.
  - For example, the sound that plays when passing a buoy might be a two note beeping sound. The key and timing of the beeps can change to match the key and timing of the currently playing music.
  - While possible with streamed music, MOISE would allow sound effects like this to be injected directly into the music track and could use readily available info from the composition to determine tempo, timing, key, etc.
* Tempo can be adjusted without changing pitch based on current power level.
  - This is extremely impractical with streamed music, moreso when combined with other interactive music techniques as the necessary amount of stems that would need to be rendered would grow exponentially as more techniques are added.
* Lead melody notes in a track can be dynamically changed into a rising scale when performing stunts.
  - This would, again, be extremely impractical with streamed music.
* Seamless transition to an exciting interlude if a player overtakes other racers after successfully taking a shortcut.
* Change the envelope of certain instruments based on how close the player is to overtaking or being overtaken by another racer.
  - Basically impossible with streamed music.

Breakdown
The project is broken down into three parts: The editor, the engine and synths. The latter two are included in this repository. The editor is where MOISE sound packages are made. The engine is where these packages are played when integrated into games. Synths receive playback info from the engine to produce audio. MOISE sound packages contain not only music track information, but transition, ending, and marker information which can be used to call game functions in sync with the music.

Inspiration
MOISE is inspired by video games from back in the 80s and 90s that took advantage of synthesizers they were limited to at the time. Since synthesizers, MIDI and similar were the standard at the time, some developers created custom systems that could alter notes on the fly. The best example of this is probably iMUSE as used in Monkey Island 2:
[iMUSE Demonstration 2 - Seamless Transitions](https://www.youtube.com/watch?v=7N41TEcjcvM)
 
[iMUSE Demonstration 1 - Seamless Endings](https://www.youtube.com/watch?v=AjtxK_WT784)
 
[iMUSE Demonstration 3 - Other Tricks](https://www.youtube.com/watch?v=-XuClagw6IQ)

[Clint Bajakian and Michael Land from LucasArts games interview](https://youtu.be/-0EqG6RYn9Y?si=ePghIS39WEbE0Xds)
 
Pinball
[ Visual Pinball X - Black Knight 2000 (Williams 1989) v2 - UnclePaulie mod - Jackpot and extra ball.
](https://www.youtube.com/watch?v=4Rn8t14axLU)

[https://www.gamedeveloper.com/audio/interactive-audio-in-black-knight-2000-the-importance-of-integration 
](https://www.gamedeveloper.com/audio/interactive-audio-in-black-knight-2000-the-importance-of-integration)

[An Interview with Brian Schmidt - GameSoundCon, Game Audio Careers, and More](https://www.youtube.com/watch?v=wI8QiladrvY&)

