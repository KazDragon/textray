#!/bin/bash
if [ -z ${EXTERNAL_ROOT+x} ]; then echo EXTERNAL_ROOT not set; exit 1; fi

export EXTERNAL_BUILD_ROOT=$HOME/external_build

mkdir "$EXTERNAL_BUILD_ROOT" || true

# Note: Boost and libfmt are install from apt in build.yml.

# Install Server++ dependency
if [ ! -f "$EXTERNAL_ROOT/include/serverpp-0.3.1/serverpp/version.hpp" ]; then
    cd "$EXTERNAL_BUILD_ROOT";
    wget https://github.com/KazDragon/serverpp/archive/v0.3.1.tar.gz;
    tar -xzf v0.3.1.tar.gz;
    cd serverpp-0.3.1;
    cmake -DCMAKE_INSTALL_PREFIX="$EXTERNAL_ROOT" -DCMAKE_PREFIX_PATH="$EXTERNAL_ROOT" -DSERVERPP_WITH_TESTS=False -DSERVERPP_VERSION="0.3.1" .;
    make -j2 && make install;
    cd ..;
fi

# Install Telnet++ dependency
if [ ! -f "$EXTERNAL_ROOT/include/telnetpp-4.0.0/telnetpp/version.hpp" ]; then
    cd "$EXTERNAL_BUILD_ROOT";
    wget https://github.com/KazDragon/telnetpp/archive/v4.0.0.tar.gz;
    tar -xzf v4.0.0.tar.gz;
    cd telnetpp-4.0.0;
    cmake -DCMAKE_INSTALL_PREFIX="$EXTERNAL_ROOT" -DCMAKE_PREFIX_PATH="$EXTERNAL_ROOT" -DTELNETPP_WITH_ZLIB=true -DTELNETPP_WITH_TESTS=False -DTELNETPP_VERSION="4.0.0" .;
    make -j2 && make install;
    cd ..;
fi

# Install Terminal++ dependency
if [ ! -f "$EXTERNAL_ROOT/include/terminalpp-4.0.1/terminalpp/version.hpp" ]; then
    cd "$EXTERNAL_BUILD_ROOT";
    wget https://github.com/KazDragon/terminalpp/archive/v4.0.1.tar.gz;
    tar -xzf v4.0.1.tar.gz;
    cd terminalpp-4.0.1;
    cmake -DCMAKE_INSTALL_PREFIX="$EXTERNAL_ROOT" -DCMAKE_PREFIX_PATH="$EXTERNAL_ROOT" -DTERMINALPP_WITH_TESTS=False -DTERMINALPP_VERSION="4.0.1" .;
    make -j2 && make install;
    cd ..;
fi

# Install Console++ dependency
if [ ! -f "$EXTERNAL_ROOT/include/consolepp-0.2.1/consolepp/version.hpp" ]; then
    cd "$EXTERNAL_BUILD_ROOT";
    wget https://github.com/KazDragon/consolepp/archive/v0.2.1.tar.gz;
    tar -xzf v0.2.1.tar.gz;
    cd consolepp-0.2.1;
    cmake -DCMAKE_INSTALL_PREFIX="$EXTERNAL_ROOT" -DCMAKE_PREFIX_PATH="$EXTERNAL_ROOT" -DCONSOLEPP_WITH_TESTS=False -DCONSOLEPP_VERSION="0.2.1" .;
    make -j2 && make install;
    cd ..;
fi

# Install nlohmann_json dependency
if [ ! -f "$EXTERNAL_ROOT/include/nlohmann/json.hpp" ]; then
    cd "$EXTERNAL_BUILD_ROOT";
    wget https://github.com/nlohmann/json/archive/v3.12.0.tar.gz;
    tar -xzf v3.12.0.tar.gz;
    cd json-3.12.0;
    cmake -DCMAKE_INSTALL_PREFIX="$EXTERNAL_ROOT" -DJSON_BuildTests=Off .;
    make -j2 && make install;
fi

# Install Munin dependency
if [ ! -f "$EXTERNAL_ROOT/include/munin-0.8.1/munin/version.hpp" ]; then
    cd "$EXTERNAL_BUILD_ROOT";
    wget https://github.com/KazDragon/munin/archive/v0.8.1.tar.gz;
    tar -xzf v0.8.1.tar.gz;
    cd munin-0.8.1;
    cmake -DCMAKE_INSTALL_PREFIX="$EXTERNAL_ROOT" -DCMAKE_PREFIX_PATH="$EXTERNAL_ROOT" -DMUNIN_WITH_CONSOLEPP=ON -DMUNIN_WITH_TESTS=False -DMUNIN_VERSION="0.8.1" .;
    make -j2 && make install;
    cd ..;
fi

echo Finished installing dependencies.
