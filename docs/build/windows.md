# Setup your Keyman build environment on Windows

## Target Projects

On Windows, you can build the following projects:

* [Keyman for Android](#keyman-for-android)
* [Keyman for Windows](#keyman-for-windows)
* [Keyman Developer](#keyman-developer) (together with Keyman for Windows)
* [KeymanWeb](#keymanweb)

The following libraries can also be built:

* Keyman Core (Windows, wasm targets) (aka common/core/desktop)
* Common/Web (aka common/core/web)

The following projects **cannot** be built on Windows:

* Keyman for Linux
* Keyman for macOS
* Keyman for iOS

## System Requirements

* Minimum Windows version: Windows 10 x64

## Repository Paths

When cloning this repo for local development on a Windows machine, take care not
to place it overly deep in your file system. Some of the paths for compilation
can push character lengths around 160 characters long, while certain operations
on Windows systems may be limited to paths of 260 characters or less. For
example, [`git clean` on Windows with
msys](https://stackoverflow.com/questions/22575662/filename-too-long-in-git-for-windows/22575737#22575737)
is limited due to dependence on older Windows APIs.

Recommended filesystem layout:

```
C:\Projects\keyman\
  keyman\              this repository (https://github.com/keymanapp/keyman)
  keyboards\           https://github.com/keymanapp/keyboards
  lexical-models\      https://github.com/keymanapp/lexical-models
  CEF4Delphi_Binary\   https://github.com/keymanapp/CEF4Delphi_Binary
  sites\
    keyman.com\        https://github.com/keymanapp/keyman.com
    ...
```

Instructions and scripts in this file assume this layout; if you use a different
layout, adjust the paths accordingly.

We recommend adding an exclusion for C:\Projects to your antivirus/security
software, for performance reasons; this can also avoid build failures when
security software locks generated executables to scan them, before the build has
finished with them.

## Project Requirements

### Keyman for Windows

Dependencies:
* [Base](#base-dependencies)
* [Windows Platform](#windows-platform-dependencies)

**Note**: Keyman for Windows is currently built together with Keyman Developer.
We are working on splitting these projects. For now, you will need the Keyman
Developer dependencies as well.

Building:
* [Building Keyman for Windows](../../windows/src/README.md)

### Keyman Developer

Dependencies:
* [Base](#base-dependencies)
* [Web](#web-dependencies)
* [Windows Platform](#windows-platform-dependencies)

Building:
* [Building Keyman Developer](../../windows/src/README.md)

### KeymanWeb

Dependencies:
* [Base](#base-dependencies)
* [Web](#web-dependencies)

Building:
* [Building KeymanWeb](../../web/README.md)

### Keyman for Android

**Dependencies**:
* [Base](#base-dependencies)
* [Web](#web-dependencies)

**Additional requirements**:
* Android SDK
* Android Studio
* Ant
* Gradle
* Maven

```ps1
# Elevated PowerShell
choco install android-sdk android-studio ant gradle maven
# optionally install sdk images
sdkmanager "system-images;android-30;google_apis;armeabi-v7a"
sdkmanager --update
sdkmanager --licenses
```

* Run Android Studio once after installation to install additional components
  such as emulator images and SDK updates.

Building:
* [Building Keyman for Android](../../android/README.md)

## Prerequisites

Many dependencies are only required for specific projects.

We prefer [Chocolatey](https://chocolatey.org/install) at present for
installation of dependencies. Chocolatey should be run in an elevated
PowerShell.

### Base Dependencies

**Projects**:
* all projects

**Requirements**:
* git for Windows
* jq
* Python 3
* Meson 0.56+
* Ninja
* Rust
* Pandoc

```ps1
# Elevated PowerShell
# for *much* faster download, hide progress bar (PowerShell/PowerShell#2138)
$ProgressPreference = 'SilentlyContinue'
choco install git jq python ninja pandoc
refreshenv
# choco meson (0.55) is too old, 0.56 required:
python -m pip install meson

# choco rustup is not currently working:
Invoke-WebRequest -Uri https://static.rust-lang.org/rustup/dist/x86_64-pc-windows-msvc/rustup-init.exe -OutFile $env:TEMP\rustup-init.exe
& "$env:TEMP\rustup-init.exe"
refreshenv
```

Restart PowerShell (because `refreshenv` doesn't seem to pick up the rust environment), then
```ps1
# Elevated Powershell
rustup target add i686-pc-windows-msvc
```

**Environment variables**:
* [`KEYMAN_ROOT`](#keyman_root)
* `PATH`: add your Python scripts folder to your path: it will normally be `%appdata%\Python\Python310\Scripts`.

```bat
SET KEYMAN_ROOT=c:\Projects\keyman\keyman
SET PATH=%path%;%appdata%\Python\Python310\Scripts
```

To check whether environment variables are set, run `SET <variable>` in command
prompt.

You can use Windows Settings to add these environment variables permanently:

1. In Windows Search, type "environment" and select "Edit System Environment
   Variables"
2. Click `Environment Variables...`
3. You can add or edit variables in either User or System settings, as you
   prefer.

### Web Dependencies

**Projects**:
* Keyman Developer
* Keyman for Android
* KeymanWeb

**Requirements**:
* node.js 14+
* emscripten 2.0.23+
* wasm-pack 0.9.1+
* openjdk 8+

```ps1
# Elevated PowerShell
rustup target add i686-pc-windows-msvc
rustup target add wasm32-unknown-unknown
choco install nodejs emscripten
choco install openjdk

# for *much* faster download, hide progress bar (PowerShell/PowerShell#2138)
$ProgressPreference = 'SilentlyContinue'
Invoke-WebRequest -Uri https://github.com/rustwasm/wasm-pack/releases/download/v0.9.1/wasm-pack-init.exe -OutFile $env:TEMP\wasm-pack-init.exe
& "$env:TEMP\wasm-pack-init.exe"
```

**Environment variables**:
* `PATH`:
  * Add emscripten to path, most likely %LocalAppData%\emsdk\upstream\emscripten

### Windows Platform Dependencies

**Projects**:
* Keyman Developer
* Keyman for Windows

**Requirements**:
* Delphi 10.3 Community or Professional:
  https://www.embarcadero.com/products/delphi/starter/free-download (Delphi
  Windows Community, DUnit Unit Testing Frameworks required)

  * Note: Delphi 10.3 Community is no longer available. Delphi 10.4 Community no
    longer includes command line compilers. This change means that building
    Keyman with Delphi 10.4 Community is not really viable. This means you can
    only really use the Professional Edition, which can be used on a trial basis
    for a short time. (We are actively working to remove Delphi dependencies
    given the licensing issues with using it.)

  Start Delphi IDE once after installation as it will create various environment
  files and take you through required registration.

  * Note: It is possible to build all components that do _not_ require Delphi by
    adding the environment variable `NODELPHI=1` before starting the build.
    Currently many components are Delphi-based, but if you are working just in
    Keyman Core, the compiler, or Keyman Engine's C++ components, you may be
    able to get away without building them. In this situation, we recommend
    copying the relevant Delphi-built components into windows/bin folders from a
    compatible installed version of Keyman for testing and debugging purposes.

* Visual C++ 2019 Community or Professional

  ```ps1
  choco install visualstudio2019community visualstudio2019-workload-nativedesktop visualstudio2019buildtools
  ```

  Recommended: configure Visual Studio to use two-space tab stops:
  1. Open the options dialog: Tools > Options.
  2. Navigate to Text Editor > All Languages > Tabs.
  3. Change 'Tab size' to 2 and 'Indent size' to 2.
  4. Select 'Insert spaces'.

**Required environment variables**:
* `PATH`
  * Add the C:\Projects\keyman\keyman\windows\lib folder in the Keyman
    repository to your `PATH` environment variable. This is required for
    Keyman's design-time packages to load in Delphi.
* [`KEYMAN_CEF4DELPHI_ROOT`](#keyman_cef4delphi_root)

**Optional environment variables**:
* [`GIT_BASH_FOR_KEYMAN`](#git_bash_for_keyman)
* [`USERDEFINES`](#userdefines)

```bat
SET KEYMAN_CEF4DELPHI_ROOT=c:\Projects\keyman\CEF4Delphi_Binary
SET GIT_BASH_FOR_KEYMAN="C:\Program Files\Git\bin\bash.exe" --init-file "c:\Program Files\Git\etc\profile" -l
```

**Additional requirements for release builds**:
* [7-Zip](http://www.7-zip.org/), used for archiving build files
* [HTML Help Workshop](http://web.archive.org/web/20160201063255/http://download.microsoft.com/download/0/A/9/0A939EF6-E31C-430F-A3DF-DFAE7960D564/htmlhelp.exe) note: Microsoft no longer offer this download...
* [WiX 3.11.1](https://github.com/wixtoolset/wix3/releases/tag/wix3111rtm)
* [CEF4Delphi_Binary](https://github.com/keymanapp/CEF4Delphi_Binary) repository

```ps1
# Elevated PowerShell
choco install 7zip html-help-workshop
choco install wixtoolset --version=3.11.1
git clone https://github.com/keymanapp/CEF4Delphi_Binary C:\Projects\keyman\CEF4Delphi_Binary
```

## Notes on Environment Variables

### KEYMAN_ROOT

If you pull the entire `keyman.git` repo to `c:\keyman`, then the paths by
default will work without changes. Otherwise, you will need to set an
environment variable `KEYMAN_ROOT` to the root path of the Keyman repo. For
example:

```bat
SET KEYMAN_ROOT=c:\projects\keyman\keyman
```

### KEYMAN_CEF4DELPHI_ROOT

Keyman and Keyman Developer use Chromium Embedded Framework. The source repo is
at https://github.com/keymanapp/CEF4Delphi. In order to build the installers, we
need to source the binary files from the
https://github.com/keymanapp/CEF4Delphi_binary repo. The
`KEYMAN_CEF4DELPHI_ROOT` environment variable should be set to the root of this
repo on your local machine.

The version of CEF in use is determined by CEF_VERSION.md. This maps to a branch
prefixed with `v` e.g. `v89.0.18` in the CEF4Delphi_binary repository. During a
release build, the buildtools/cef-checkout.sh script will checkout the correct
branch of the repository automatically and extract any compressed files found in
it.

### GIT_BASH_FOR_KEYMAN

This environment variable is optional: the build will run bash in a separate
window in order to build KeymanWeb if it isn't present, but you'll lose logging
and have the annoyance of a window popping up halfway through the build. To
resolve both of those issues, set the environment variable to:

```bat
SET GIT_BASH_FOR_KEYMAN="C:\Program Files\Git\bin\bash.exe" --init-file "c:\Program Files\Git\etc\profile" -l
```

You should verify the install location of Git on your computer as it may vary.

### USERDEFINES

You can specify defines that will not be added to the git repository and will be
used in the build in the UserDefines.mak file in the root folder. This is used
mostly for code signing certificates. If not specified, a test certificate will
be used to sign executables when you build a release.

To include UserDefines.mak in the build, use the command line parameter
`-DUSERDEFINES`. You can also set an environment variable `USERDEFINES=1` to get
the same result.

## Optional Tools

* sentry-cli (optional)
  - Uploading symbols for Sentry-based error reporting

  bash:
  ```bash
  # bash
  curl -sL https://sentry.io/get-cli/ | bash
  ```
