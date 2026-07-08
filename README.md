# <img src="screenshot-mg.png" width="28" height="28" alt="project icon"> screenshot-cpp-mg

Screenshot utility written in C++

## Usage

Get the latest build from [/releases](https://github.com/Minighost/screenshot-cpp-mg/releases/).

Download, extract, and run. Doesn't have an installer; You'll have to either add it to your start-up programs folder or have Task Scheduler launch it on login (or whenever you'd like).

Only lives as a tray icon; Right click to exit or access the settings menu.

If you encounter any bugs, please feel free to report them under [/issues](https://github.com/Minighost/screenshot-cpp-mg/issues/).

## Features

- Different modes
  - Region select (with persistent display for adjustment)
  - Fullscreen capture
  - Individual window capture
- Hotkeys
  - Ctrl+S (Save to file)
  - Ctrl+C (Copy to clipboard)
  - Ctrl+Shift+C (Open Preview window)
  - Hotkeys work on the Preview as well
  - R (Reset zoom level in Preview)
  - Shift+R (Reset Preview window size)
- Mark-up on Preview
  - Includes panning, free draw, and shape draw
  - Enables Undo (Ctrl+Z) and Redo (Ctrl+Y)
- Persistent settings
  - Saved in the program's directory as "settings.ini"

## TODO

- Add a toggle to include the cursor
- Add build instructions to README
- Add names to auto-save filenames for "source" (region, fullscreen, window)

## Limitations

- Windows only
  - I'm on Windows 11 build 26200, haven't tested it on anything else
  - Explicitly uses Window's API
- Does not account for non-uniform DPR settings
  - If you have different DPR settings (100%, 125%, 150%, etc) for each monitor, it might lead to some wonky results
  - Always uses the DPR of your primary monitor
- Doesn't work when an elevated fullscreen app is running
  - Pretty much any game with a kernel level anti-cheat
  - Valorant, Battlefield 6, and Wuthering Waves all didn't allow screenshots
  - Fix: Run program as admin

## Disclaimer

This program does not access the internet. One of the big reasons I wanted to build this was because I disliked how most screenshot utilities have some form of automated upload to an image hoster. Most of the time, when I'm taking screenshots, I need to reference some information (preview), send my friends a funny picture (copy), or save a memory for later (save).

I will not be implementing any automated upload feature.

## License

Uses GNU's GPL-3.0 license.

Some of the SVG files were downloaded from [pictoprogrammers.com](https://pictogrammers.com). [Here is their license](https://pictogrammers.com/docs/general/license/), it's "GPL friendly". I have scaled some of them up to unify the icon sizes.

I created the rest of the SVGs myself in Inkscape. As such, these are open to use for the public.

I also created the icon myself in [Photopea](https://www.photopea.com/). I got the color scheme by searching "ui color gradients" in Google Images, then selected one at random.
