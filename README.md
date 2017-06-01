# Opvangen en analyseren van data omtrent passagiersaantallen op treinen
Deze github bevat de source code, gebruikte libraries en extra bronnen die besproken werden in mijn thesis "Opvangen en analyseren van data omtrent passagiersaantallen op treinen".
Laurent Loots, 1 juni 2017
# Libraries
De map 'setup' bevat enkele bashbestanden die de benodigde libraries voor de ontwikkelde software te draaien. Bij het uitvoeren van de scripts worden volgende libraries geïnstalleerd:
* Servus / Human Brain Project Visualization Software) - https://github.com/HBPVIS/Servus
* curlpp / jpbarette - http://www.curlpp.org/
* OpenCV 3.1.0 - http://opencv.org/
* ZeroMQ - http://zeromq.org/
Overige libraries:
* avahi-daemon
* avahi-dnsconfd
* avahi-discover
* avahi-utils
* libavahi-client-dev
* qt5-default
* ninja-build
* libboost-all-dev
* git
* cmake
* libgtk2.0-dev
* pkg-config
* libavcodec-dev
* libavformat-dev
* libswscale-dev
* libtool
* autoconf
* automake
* uuid-dev
* unzip
# Release
De map 'release' bevat de finale broncode voor camNode, camServer en de back-end.
### camNode
camNode kan als volgt (met of zonder argument) via de terminal worden gestart. Indien geen argument meegegeven wordt, zal het pad van de terminal gebruikt worden als werkmap.
```
./main [pad_naar_werkmap]
```
### camServer
camServer kan als volgt (met of zonder argument) via de terminal worden gestart. Indien geen argument meegegeven wordt, zal het pad van de terminal gebruikt worden als werkmap.
```
./camServer [pad_naar_werkmap]
```
### backend
De backend kan in zijn huidige vorm meteen gebruikt worden. Enkel de MySQL connectie in release/backend/include/connection.php moet worden gewijzigd. Verder moet het bestand /resources/database.rar worden uitgepakt en ingeladen in de databank.
# Testing
### testNode
testNode is een aangepaste versie van camNode waarmee een videobestand in een lus kan worden aangeboden aan een camServer.
testNode kan als volgt via de terminal worden gestart:
```
./main port path_to_avi [seconds_per_frame] [load_in_memory]
```
### trainframe_emu
trainframe_emu is een hulpmiddel om de boordsystemen van een trein mee te emuleren.
trainframe_emu kan zonder parameters via de terminal worden gestart.
# Resources
De map resources bevat een vooropgenomen dieptebeeld dat gebruikt kan worden in combinatie met testNode.
Daarnaast is ook een dump van de MySQL database van de back-end terug te vinden.
