sudo apt-get install unzip
unzip curl-7.53.1.zip
cd curl-7.53.1
./configure
sudo make install
cd ..
unzip curlpp-0.8.1.zip
cd curlpp-0.8.1
cmake . . && make -j 4
sudo make install

