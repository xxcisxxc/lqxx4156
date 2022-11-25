if [[ "$1" == "install" || "$1" == "" ]]; then
sudo apt update
sudo apt install build-essential git cmake libssl-dev autoconf libtool clang-format libcypher-parser-dev libedit-dev pkg-config nlohmann-json3-dev python3-pip -y
git submodule update --init
cd external/googletest && mkdir build && cd build && cmake .. && make && sudo make install && cd ../../..
cd external/libneo4j-client && ./autogen.sh && ./configure --disable-werror && make clean check && sudo make install && cd ../../..
if neo4j-client --version; then
    echo "Env already set"
else
    echo "Set Env"
    echo "/usr/local/lib" | sudo tee -a /etc/ld.so.conf
    sudo ldconfig
fi
if [ -x "$(command -v docker)" ]; then
    echo "Skip install docker"
else
    echo "Install docker"
    curl -fsSL https://get.docker.com | bash -s docker
fi
fi

if [ "$1" == "build" ]; then
    docker run -d -p7474:7474 -p7687:7687 -e NEO4J_AUTH=neo4j/hello4156 neo4j:4.4.9
    sleep 10
    mkdir build && cd build && cmake .. && make
fi

if [ "$1" == "run" ]; then
    docker run -d -p7474:7474 -p7687:7687 -e NEO4J_AUTH=neo4j/hello4156 neo4j:4.4.9
    sleep 10
fi
