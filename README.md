# HEARTBEAT animation meme for Pebble Time/Time Round

Wow. a YouTube animation meme on a 10 year old smartwatch. Literally no one asked for this.
I just made this to have fun and to learn a few things.

![Basalt](screenshots/color.png) ![Chalk](screenshots/round.png)

The app makes the watch vibrate to the beat of the song. You can also toggle the subtitles by pressing select.
(subtitles are not available on PTR because there's not enough room for them)

## build instructions
Install [uv](https://docs.astral.sh/uv/) and run:

```
uv tool install pebble-tool
pebble sdk install latest
```
if you haven't already.

Clone the repo and build:

```
git clone https://github.com/nubbybubby/pebble-heartbeat-animation-meme
cd pebble-heartbeat-animation-meme
pebble build
```

the .pbw file will be in the build directory.
