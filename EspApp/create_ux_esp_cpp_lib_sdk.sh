#! /bin/sh

cd ..
rm -rf build_UxEspCppLibrary
mkdir build_UxEspCppLibrary
cd build_UxEspCppLibrary
git clone ssh://git@ux-git.u-experten.de:224/Ux-Eco/UX-MOVE-E/UX-EES-ESP32CPPLIB.git -b HEM_E54_HMI_S01_V03_04
cd UX-EES-ESP32CPPLIB
git describe --tags > git-sha.txt
git log -1 --pretty=format:%H >> git-sha.txt
SHORT_SHA=`git rev-parse --short HEAD`
rm -rf .git .gitignore code_clean.sh uncrustify.cfg
cd PluginTasks
rm -rf Common
rm -rf CompWifiConnectorTask
rm -rf PluginTaskApds9930
rm -rf PluginTaskBme280
rm -rf PluginTaskGuvaS12sd
rm -rf PluginTaskHdc2010
rm -rf PluginTaskNeo6M
rm -rf PluginTaskNeoPixel
rm -rf PluginTaskPaj7620
rm -rf PluginTaskTcs34725
rm -rf PluginTaskVl53l0x

rm -rf PluginTaskAt24c16/Documentation
rm -rf PluginTaskAt24c16/TestApplication

rm -rf PluginTaskWifiConnector/Documentation
rm -rf PluginTaskWifiConnector/TestApplication

cd ../..
zip -r UX-EES-ESP32CPPLIB-${SHORT_SHA}.zip UX-EES-ESP32CPPLIB
rm -r UX-EES-ESP32CPPLIB
