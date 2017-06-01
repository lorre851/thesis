sudo apt-get install avahi-daemon avahi-dnsconfd avahi-discover avahi-utils libavahi-client-dev qt5-default ninja-build libboost-all-dev
git clone https://github.com/HBPVIS/Servus.git
mkdir Servus/build
cd Servus/build
cmake -GNinja ..
ninja
sudo ninja install
