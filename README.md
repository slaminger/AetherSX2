# AetherSX2/AetherPurpleSX2 Source

> Since this source was leaked to the public, no one has been interested in correcting it and actually making it happen; well it's here.
--------
- ⚠️ Note that this is not the full source version of the library, just the LGPL source and modified portions of it. The LGPL does not require full source code to be released, only closed source components may be relinked/combined into LGPL components and build instructions, included as per section 4/5 of the license.

- ⚠️ Since the source of the App (apk) is not public or leaked yet, a "shell" apk is needed to make the build work.

- ⚠️ Also note that this source is very old (2021), it's the only one we have to work with, every contribution is welcome.
--------
# Things to do:

- [x] Make source buildable
- [x] Approximate git history (check 'git-his' branch)
- [ ] New contributors, PRs
- [ ] Update to the latest NDK/SDK
- [ ] Update PCSX2 code
- [ ] Add features from the latest AetherSX2
- [ ] App completely decompiled and redone (reverse engineering)
- [ ] several other things 
--------
# Dependencies and requirements
You will need:
- Android Studio/Android Studio Dependencies & SDK
- Cmake (latest or 3.22.1)
- Linux distro (Personally I recommend 22.04 LTS)
- NDK Specifically [R23c](https://dl.google.com/android/repository/android-ndk-r23c-linux.zip)
- Approximate 60GB free space and 8GB ram machine (minimum)
- Shell apk [HERE](https://drive.google.com/file/d/1FoLijHU4w9T82frNr145jE_IalYmuIDC/view?usp=drivesdk)
--------
# Build Steps
First, I recommend installing Android Studio, and installing the latest ndk and then downloading sdk 31.

Build Dependencies:
```
sudo apt install build-essential gcc g++ cmake make ninja-build git wget openssl zipalign apksigner zip unzip openjdk-19-jre-headless -y
```
- Creating libemucore.so:
1. Extract NDK to any easily accessible place and rename it to something easy like 'ndk'
2. Clone source:
```
git clone https://github.com/MrPurple666/AetherSX2 -b main
```
3. Creating necessary folders:
```
mkdir build-android && cd build-android && mkdir apk
```
4. Configure the CMAKE system (Change ndk_folder to your ndk path):
```
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=/ndk_folder/build/cmake/android.toolchain.cmake -DANDROID_PLATFORM=android-26 -DANDROID_ABI=arm64-v8a ..
```
5. Building libemucore library:
```
make -j8
```
6. Doing repack with "apk shell" and the new library:
```
cd apk
```
- Change apk_path to your shell apk file location.
```
cp /apk_path/purplesx2.apk .
````
```
mkdir -p lib/arm64-v8a && cp ../pcsx2/libemucore.so lib/arm64-v8a
```
```
zip -0 purplesx2.apk lib/arm64-v8a/libemucore.so
```

7. Align and signing apk:
```
zipalign -p 4 purplesx2.apk purplesx2-aligned.apk
```

```
keytool -genkey -v -keystore keyname.keystore -alias keyname -keyalg RSA -keysize 2048 -validity 10000
```
- change pass_keystore to the password you created when generating your signature 
```
apksigner sign --ks keyname.keystore --ks-pass "pass:pass_keystore" --ks-key-alias keyname --out purplesx2-signed.apk --verbose purplesx2-aligned.apk
```
DONE, you will get the app installable and functional.
--------
# Full credits to Talreth/Stenzek.
--------
# Special thanks:
- Jonas Angel
- Mis012 (git history approximation)
- Gamer64yt
- GiovanYCringe
- Hacobotdev
- All the people who supported me and support emulation projects, my sincere thanks ♥️💜♥️.
