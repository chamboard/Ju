This folder contains a shell script and a c program. Both of them are syncing rtc with system datetime.
syncRTC.c is syncRTC_P code source.
syncRTC_P and syncRTC.sh use an old version of "chc" named "chanram4". 
Replace "chanram4" by "chc" in syncRTC.c to recompile.
Check RTC_DATA formats (BCD or decimal) since firwmare update. 
