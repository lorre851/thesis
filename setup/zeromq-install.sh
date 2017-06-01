sudo apt-get install libtool autoconf automake uuid-dev unzip
unzip zeromq-4.2.1.zip
cd zeromq-4.2.1
./configure
make -j 4
sudo make install
cd ..
sudo mkdir /usr/cmake
sudo cp -i FindZeroMQ.cmake /usr/cmake/
unzip cppzmq-master.zip
cd cppzmq-master
cmake . . && make -j 2
sudo make install


