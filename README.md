- [1. Introduction](#1-introduction)
- [2. Current State](#2-current-state)
  - [2.1. Code sanity](#21-code-sanity)
  - [2.2. Code Credit](#22-code-credit)
  - [2.3. Features](#23-features)
- [3. Building and installing](#3-building-and-installing)
  - [3.1. Get latest stable code](#31-get-latest-stable-code)
  - [3.2. Prerequisites and dependencies](#32-prerequisites-and-dependencies)
  - [3.3. Building](#33-building)
  - [3.4. Installing](#34-installing)
  - [3.5. Configuring](#35-configuring)
- [4. Contributing](#4-contributing)
  - [4.1. Work Flow](#41-work-flow)
- [5. Version History](#5-version-history)
  - [5.1. Version 0.3](#51-version-03)
  - [5.2. Version 0.2](#52-version-02)
  - [5.3. Version 0.1](#53-version-01)


# 1. Introduction
This is a port of G4KLX Jonathan Naylor's [ircddbGateway](https://github.com/g4klx/ircDDBGateway). It is wxWidgets free and has minimal dependencies to boost (header libs only), libconfig++ and libcurl

# 2. Current State
## 2.1. Code sanity
The current code is working, yet ugly IMHO as it is a mix of C and C++ of various ages.

The code has also been amended to no longer rely on compiler defines for paths like log or data. These can be set in configuration file.

Quite a few classes are more or less copy/paste from each other some sanitization by using base classes or template classes would greatly improve code maintainibility.
## 2.2. Code Credit
- Jonathan Naylor G4KLX (The original author of [ircddbGateway](https://github.com/g4klx/ircDDBGateway))
- Thomas A. Early N7TAE (Code taken from his [smart-group](https://github.com/n7tae/smart-group-server) software)
- Geoffrey Merck F4FXL / KC3FRA [That's me !](https://github.com/F4FXL/)
## 2.3. Features
All the features found in ircddbGateway are supposed to be working. I have mixed feelings about putting these back in or not.

Features that where left out :
- CCS: is still being used? I always considered this as trojan horse to push some DMR Agenda into DStar an more or les a burdain to use. Call sign routing is by far more flexible and superior.
- Starnet: You might consider running [Smart Group Server XL](https://github.com/F4FXL/smart-group-server-xl) from a dedicated computer instead.
- Announcement: same can be achieved using transmitd.
- APRSGateway capability: I would prefer to have some sort of TCP "APRS-IS proxy" program sitting between the program and the APRS server, thus keeping the ability to directly connect to APRS-IS or not.
- Mobile APRS: Code has been ported, yet I am targeting repeaters so low priority.

# 3. Building and installing
## 3.1. Get latest stable code
From inside the already cloned repository run following commands
```
git fetch --tags
latestTag=$(git describe --tags `git rev-list --tags --max-count=1`)
git checkout $latestTag
```

## 3.2. Prerequisites and dependencies
Before first time building you need to install dependencies and prerequisites
```
apt install build-essential libconfig++-dev libcurl4-openssl-dev libboost-dev
```
## 3.3. Building
```
make
```
## 3.4. Installing
The program is meant to run as a systemd service. All bits an pieces are provided.
```
sudo make install
```
## 3.5. Configuring
After installing you have to edit the configuration file. If you went with default paths, the config file is located in `/usr/local/etc/dstargateway.cfg`

The configuration format is quite straight forward. It is organised in sections and key/value pairs.
The order of the sections or key/values pairs inside the sections does not matter nor does casing.
Boolean values can be set using true, false, 1 or 0
Floating point values must use . (point) as decimal separatorsensitive.

When done with configuration, the daemon will be started automatically on boot. To manual start and stop it use the usual systemd commands
```
sudo systemctl start dstargateway.service
sudo systemctl stop dstargateway.service
```
# 4. Contributing
## 4.1. Work Flow
I Use [Git flow](https://danielkummer.github.io/git-flow-cheatsheet/) as my workflow. PR are welcome and shall be done against the develop branch and follow the Git Flow branch naming rules.

# 5. Version History
## 5.1. Version 0.3
- [Improvement] Get ride of libcongif++ dependency. When upgrading from earlier version you need to manualy delete the config file before reinstalling.
## 5.2. Version 0.2
- [bugfix] ircDDBFreeze when repeater not found ([#1](https://github.com/F4FXL/DStarGateway/issues/1))
- Code sanitization
## 5.3. Version 0.1
First working version