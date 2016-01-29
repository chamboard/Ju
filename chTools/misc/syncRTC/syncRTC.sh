#!/bin/bash
# Indique au système que l'argument qui suit est le programme utilisé pour exécuter ce fichier
# En règle générale, les "#" servent à mettre en commentaire le texte qui suit comme ici
echo Syncing script of Chantilly RTC. 
#date 
sec=$(date +%S)
sec=$(echo $sec"/10*16 +" $sec"%10"| bc) #do division before multiplication to troncate result
sec=$(echo $sec "+2"| bc)

min=$(date +%M)
min=$(echo $min"/10*16 +" $min"%10"| bc)
hour=$(date +%H)
hour=$(echo $hour"/10*16 +" $hour"%10"| bc)
wday=$(date +%u)
wday=$(echo $wday"/10*16 +" $wday"%10"| bc)
mday=$(date +%d)
mday=$(echo $mday"/10*16 +" $mday"%10"| bc)
mon=$(date +%m)
mon=$(echo $mon"/10*16 +" $mon"%10"| bc)
year=$(date +%y)
year=$(echo $year"/10*16 +" $year"%10"| bc)

#echo $sec
#echo $min
#echo $hour
#echo $wday
#echo $mday
#echo $year
sudo ./chanram4 -a 7 -e 1 
sudo ./chanram4 -a 7 -p 0x140 -w1 -v $sec:$min:$hour:$wday:$mday:$mon:$year
sudo ./chanram4 -a 7 -e 2
#sudo ./chanram4 -a 7 -p 0x140 -r11
#myTime=$(date +%H%M)
#myDate=$(date +%Y%m%d) 
#myDateTime=$(date +%Y%m%d%H%M%S)
#echo $myTime
#echo $myDate
#echo $myDateTime
exit 0
