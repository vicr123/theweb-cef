# theWeb
Web Browser

## Build
If you're on Arch Linux, you can find theWeb as a package on the AUR. Otherwise, to build theWeb,

1. First, grab a copy of the [Chromium Embedded Framework.](http://opensource.spotify.com/cefbuilds/) Right now, theWeb is using the 2704 branch.
2. Extract the resulting tarball somewhere
3. Copy libcef.so found in the Release folder of the tarball to the location where you put theWeb's sources
4. Run the following commands to build theWeb
   ```
   qmake
   make
   ```

## Install
1. Create a directory in /opt/theWeb
2. Copy theweb to /opt/theWeb
3. Copy all the files from the CEF tarball/Release to /opt/theWeb
4. Copy all the files from the CEF tarball/Resources to /opt/theWeb
5. Copy libcef_dll_wrapper.a to /opt/theWeb
6. Give /opt/theWeb/chrome-sandbox the SUID permission, and make sure that ownership of this file is set to root.
   ```
   chmod 4755 "/opt/theWeb/chrome-sandbox"
   ```
7. Copy theweb.desktop to your Applications folder (usually /usr/share/applications)
8. Copy theweb-execscript to your binaries folder (usually /usr/bin) and rename it theweb
9. Copy icon.svg to your icons folder (usually /usr/share/icons)
