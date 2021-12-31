- [1. Introduction](#1-introduction)
- [2. Current State](#2-current-state)
  - [2.1. Code sanity](#21-code-sanity)
  - [2.2. Code Credit](#22-code-credit)
  - [2.3. Features](#23-features)
- [3. Building and installing](#3-building-and-installing)
  - [3.1. Initial setup](#31-initial-setup)
  - [3.2. Get latest stable version (recommended)](#32-get-latest-stable-version-recommended)
  - [3.3. Get latest development version version](#33-get-latest-development-version-version)
  - [3.4. Prerequisites and dependencies](#34-prerequisites-and-dependencies)
  - [3.5. Building](#35-building)
      - [Build With GPSD Support](#build-with-gpsd-support)
  - [3.6. Installing](#36-installing)
  - [3.7. Configuring](#37-configuring)
- [4. Contributing](#4-contributing)
  - [4.1. Work Flow](#41-work-flow)
- [5. Version History](#5-version-history)
  - [5.1. Version 0.4](#51-version-04)
  - [5.2. Version 0.3](#52-version-03)
  - [5.3. Version 0.2](#53-version-02)
  - [5.4. Version 0.1](#54-version-01)


# 1. Introduction
This is a port of G4KLX Jonathan Naylor's [ircddbGateway](https://github.com/g4klx/ircDDBGateway). It is wxWidgets free and has minimal dependencies to boost (header libs only) and libcurl

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
- APRSGateway capability: I would prefer to have some sort of TCP "APRS-IS proxy" program sitting between the program and the APRS server, thus keeping the ability to directly connect to APRS-IS or not, depending on the system owner wish. I run mostly DStar Only repeaters, having an additional program to maintain is unnecessary burden.
- DRats : No opinion on this one, I am not using it.
- CallSign Server : this is a legacy from the dead project xreflector.net, I will most probably drop it.

# 3. Building and installing
## 3.1. Initial setup
Clone the repository (only required initally)
```
git clone https://github.com/F4FXL/DStarGateway.git
cd DStarGateway
```
## 3.2. Get latest stable version (recommended)
From inside the cloned repository run following commands to get the latest stable version
```
git pull -p
git fetch --tags
latestTag=$(git describe --tags `git rev-list --tags --max-count=1`)
git checkout $latestTag
```
## 3.3. Get latest development version version
```
git checkout develop
```

## 3.4. Prerequisites and dependencies
Before first time building you need to install dependencies and prerequisites
```
apt install build-essential libcurl4-openssl-dev libboost-dev
```
If you are going to build with gpsd support, also install libgps-dev
```
apt install libgps-dev
```
## 3.5. Building
Regular building
```
make
```
#### Build With GPSD Support
```
make USE_GPS=1
```
## 3.6. Installing
The program is meant to run as a systemd service. All bits an pieces are provided.
```
sudo make install
```
## 3.7. Configuring
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
## 5.1. Version 0.4
- [Improvement] Add APRS status link feature ([#8](https://github.com/F4FXL/DStarGateway/issues/8))
- [Bugfix] Posotions received over radio were not sent to APRS-IS when GPDS connection failed. ([#7](https://github.com/F4FXL/DStarGateway/issues/7))
- [Improvement] Bring back GPSD support (https://github.com/F4FXL/DStarGateway/issues/6)
- [Improvement] Log enhancements ([#4])(https://github.com/F4FXL/DStarGateway/issues/4)
## 5.2. Version 0.3
- [Improvement] Get ride of libcongif++ dependency. When upgrading from earlier version you need to manualy delete the config file before reinstalling.
## 5.3. Version 0.2
- [Bugfix] ircDDBFreeze when repeater not found ([#1](https://github.com/F4FXL/DStarGateway/issues/1))
- Code sanitization
## 5.4. Version 0.1
First working version