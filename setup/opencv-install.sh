sudo apt install git cmake libgtk2.0-dev pkg-config libavcodec-dev libavformat-dev libswscale-dev unzip
unzip opencv-3.1.0.zip
cd opencv-3.1.0
mkdir build
cd build
cmake -D CMAKE_BUILD_TYPE=Release -D CMAKE_INSTALL_PREFIX=/usr/local ..
make -j 4
sudo make install


