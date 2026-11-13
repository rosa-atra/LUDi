# LUDi

Build the Linux, Windows, and Android binaries from the repository root with:

```sh
make all
```

The outputs are `LUDi/build/app`, `LUDi/build/app.exe`, and
`LUDi/build/app.apk`. The Windows build includes the game assets under
`LUDi/build/LUDi/assets`, so run `app.exe` from `LUDi/build`.
The Android launcher uses the icon in `LUDi/assets`.

To build or run only the native Linux app:

```sh
make run
```

The main menu lets you continue a paused run, start a new game, view the high
score, or exit. Each enemy destroyed is worth 100 points; a new record is saved
to `space_high_score.txt` in the game's working directory. During play, press
Escape (or Android Back) to pause and return to the menu. Ammo is unlimited;
the HUD shows health, fuel, and score without coordinate or ammo telemetry.
On Android, tap menu options and drag horizontally during play to steer the
ship. On desktop, use A/D or the left/right arrow keys to dodge.