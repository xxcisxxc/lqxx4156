sudo apt update
sudo apt install build-essential git cmake libssl-dev autoconf libtool clang-format libcypher-parser-dev libedit-dev -y
git submodule update --init
cd external/googletest && mkdir build && cd build && cmake .. && make && sudo make install && cd ../../..
cd external/libneo4j-client && ./autogen.sh && ./configure --disable-werror && make clean check && sudo make install && cd ../../..
curl -fsSL https://get.docker.com | bash -s docker
docker run -d -p7474:7474 -p7687:7687 -e NEO4J_AUTH=neo4j/hello4156 neo4j:4.4.9
sleep 10
sudo echo "/usr/local/lib" >> /etc/ld.so.conf && sudo ldconfig
mkdir build && cd build && make
