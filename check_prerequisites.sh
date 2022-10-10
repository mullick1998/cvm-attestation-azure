#!/bin/bash

RED='\e[31m'
GREEN='\e[32m'
YELLOW='\e[33m'
BLUE='\e[34m'
MAGENTA='\e[35m'
CYAN='\e[36m'

RESET='\e[0m' # No Color

function announce {
    echo -e -n "\n\n${YELLOW}--- Tools for "
    for var in "$@"; do echo -e -n "\"$var\" "; done;
    echo -e "---${RESET}"
}

# string $1 - type of requested tool (Command / Package / ...)
# string $2 - what will be installed
# bool   $3 - check whether already present
# string $4 - command
# string $5 - short description
# string $6 - installation
# string $7 - subcommand to $4 for showing version (optional)
function _check {
    echo -e -n "\n${CYAN}Checking for $5: "
    if ! eval "$3" &> /dev/null
    then
        echo -e "${RED}$1$4 missing. Installing $2...${RESET}"
        eval "$6"
    else
        echo -e "${GREEN}$1$4 found.${RESET}"
    fi
    if (($# == 7)); then echo "$4 $7"; $4 $7; fi
}

function check_package {
    _check "Package " "it" "dpkg -l $1" "$1" "$2" "sudo apt-get install $1" $3
}

function check_package_custom {
    _check "Package " "it" "dpkg -l $1" "$1" "$2" "$3" $4
}

function check_command {
    _check "Command " "package $2" "command -v $1" "$1" "$3" "sudo apt-get install $2" $4
}

function check_other {
    _check "" "it" "command -v $1" "$1" "$2" "$3" $4
}

function check_other_non_command {
    _check "" "it" "$1" "$2" "$3" "$4" $5
}

announce "SampleAppAzureCVMAttestation"

check_package "build-essential" "Build essentials"
check_package "libcurl4-openssl-dev" "Curl 4 Library"
check_package "libjsoncpp-dev" "C++ JSON Library"
check_package "libboost-all-dev" "C++ Boost Library"
check_package "nlohmann-json3-dev" "JSON 3 Library"

check_package_custom "azguestattestation1" "Azure Guest Attestation" "
    curl -O https://packages.microsoft.com/repos/azurecore/pool/main/a/azguestattestation1/azguestattestation1_1.0.2_amd64.deb &&
    sudo dpkg -i azguestattestation1_1.0.2_amd64.deb &&
    rm azguestattestation1_1.0.2_amd64.deb"
