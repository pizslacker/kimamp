# k!Mamp

Simple fun little project where I try to make a **Winamp** clone in `C/SDL2` on Linux.

    For now it is pretty much barebones and hardcoded, i.e: no widgets, file dialogues, settings, etc.
    Since it utilizes SDL2 it works on pretty much any distro, or *nix for that matter :P

It needs the following assets in the same directory to function:
- `base.wsz` (any classic Winamp 2.x skin renamed to this)
- `test.mp3` for audio playback

## Required:
```bash
sudo apt install build-essential libsdl2-dev libsdl2-mixer-dev
```

## Compile:
```bash
make
```
