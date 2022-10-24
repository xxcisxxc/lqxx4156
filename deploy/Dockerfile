FROM gromacs/cmake-3.15.7-llvm-9-openmpi-master:latest
WORKDIR /workspaces/lqxx
RUN apt-get update &&\
    apt-get install git -y &&\
    apt-get install build-essential -y &&\
    apt-get install neo4j-client libneo4j-client-dev -y &&\
    apt-get install libssl-dev -y &&\
    apt-get install libgtest-dev -y &&\
    apt-get install libcypher-parser-dev -y &&\
    apt-get install libedit-dev -y