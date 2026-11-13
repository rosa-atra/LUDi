# LUDi

## Build directives

Run these commands from the repository root, where the top-level `Makefile`
is located.

### Build targets

```sh
make all
```

| Command | Description | Output |
| --- | --- | --- |
| `make all` or `make all-platforms` | Build the game for Linux, Windows, and Android. | `LUDi/build/app`, `LUDi/build/app.exe`, `LUDi/build/app.apk` |
| `make app` | Build the native Linux game. | `LUDi/build/app` |
| `make mswin` | Cross-compile the Windows game and copy its assets beside the executable. | `LUDi/build/app.exe`, `LUDi/build/LUDi/assets/` |
| `make ndk` | Build the Android debug APK, including the launcher icon from `LUDi/assets`. | `LUDi/build/app.apk` |

Run the Windows executable from `LUDi/build` so its relative asset paths resolve.

### Run targets

```sh
make run
```

`make run` builds and launches the native Linux game.

### Android installation

`make install_android` builds the debug APK and installs or updates it on an
Android device connected and authorized through ADB.

### Clean targets

| Command | Description |
| --- | --- |
| `make clean_android` | Remove Android Gradle/CMake intermediates and the copied APK and launcher icon resource. |
| `make clean_mswin` | Remove `LUDi/build/app.exe`. |
| `make clean` | Remove game and miscellaneous binaries, object files, copied Windows assets, and the copied Android build APK. |

### Build requirements

- **Linux:** GCC, Raylib development libraries, and the Linux graphics/audio
  libraries used by Raylib.
- **Windows:** `x86_64-w64-mingw32-gcc` and a Windows Raylib build under
  `.build-mswin/raylib`.
- **Android:** Android SDK and NDK, Java, and Gradle. The Makefile defaults to
  `$HOME/Android/Sdk`, `/opt/java21`, and `/opt/gradle/bin/gradle`.

## Gameplay

The main menu lets you continue a paused run, start a new game, view the high
score, or exit. Each enemy destroyed is worth 100 points; a new record is saved
to `space_high_score.txt` in the game's working directory. During play, press
Escape (or Android Back) to pause and return to the menu. Ammo is unlimited;
the HUD shows health, fuel, and score without coordinate or ammo telemetry.
On Android, tap menu options and drag horizontally during play to steer the
ship. On desktop, use A/D or the left/right arrow keys to dodge.