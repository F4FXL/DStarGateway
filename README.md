[![F4FXL](https://circleci.com/gh/F4FXL/DStarGateway.svg?style=svg)](https://app.circleci.com/pipelines/github/F4FXL/DStarGateway?filter=all)
- [1. Introduction](#1-introduction)
- [2. Current State](#2-current-state)
  - [2.1. Code sanity](#21-code-sanity)
  - [2.2. Code Credits](#22-code-credits)
  - [2.3. Thanks](#23-thanks)
  - [2.4. Features](#24-features)
    - [2.4.1. Features that where left out :](#241-features-that-where-left-out-)
    - [2.4.2. Additional Features compared to ircddbGateway:](#242-additional-features-compared-to-ircddbgateway)
- [3. Building and installing](#3-building-and-installing)
  - [3.1. Initial setup](#31-initial-setup)
  - [3.2. Get latest stable version (recommended)](#32-get-latest-stable-version-recommended)
  - [3.3. Get latest development version](#33-get-latest-development-version)
  - [3.4. Prerequisites and dependencies](#34-prerequisites-and-dependencies)
  - [3.5. Building](#35-building)
      - [3.5.0.1. Build With GPSD Support](#3501-build-with-gpsd-support)
      - [3.5.0.2. Debug Build](#3502-debug-build)
  - [3.6. Installing](#36-installing)
  - [3.7. Configuring](#37-configuring)
  - [3.8. Updating host files](#38-updating-host-files)
- [4. Contributing](#4-contributing)
  - [4.1. Work Flow](#41-work-flow)
  - [4.2. Continuous Integration](#42-continuous-integration)
- [5. Version History](#5-version-history)
  - [5.1. Version 1.0](#51-version-10)
  - [5.2. Version 0.7](#52-version-07)
  - [5.3. Version 0.6](#53-version-06)
  - [5.4. Version 0.5](#54-version-05)
  - [5.5. Version 0.4](#55-version-04)
  - [5.6. Version 0.3](#56-version-03)
  - [5.7. Version 0.2](#57-version-02)
  - [5.8. Version 0.1](#58-version-01)
- [6. Future](#6-future)


# 1. Introduction
This is a port of G4KLX Jonathan Naylor's [ircddbGateway](https://github.com/g4klx/ircDDBGateway). It is wxWidgets free and has minimal dependencies to boost (header libs only) and libcurl. I plan to ad some features in the future

# 2. Current State
## 2.1. Code sanity
The current code is working, yet it is a mix of C and C++ of various ages and there is plenty of room for rework. I realised that G4KLx started programming this over a decade ago, when C++11 was not yet a thing !

The code has also been amended to no longer rely on compiler defines for paths like log or data. These can be set in configuration file.

Quite a few classes are more or less copy/paste from each other some sanitization by using base classes or template classes would greatly improve code maintainibility. Maybe one day ;)
## 2.2. Code Credits
- Jonathan Naylor G4KLX (The original author of [ircddbGateway](https://github.com/g4klx/ircDDBGateway))
- Thomas A. Early N7TAE (Code taken from his [smart-group](https://github.com/n7tae/smart-group-server) software)
- Geoffrey Merck F4FXL / KC3FRA [That's me !](https://github.com/F4FXL/)
## 2.3. Thanks
- Cyrille F1MHV / DF1CHB for the testing
- Jonathan Naylor G4KLX for all the work ahead and the rock solid code base
## 2.4. Features
All the features found in ircddbGateway are supposed to be working. Except the ones listed below

### 2.4.1. Features that where left out :
- CCS: is still being used? I always considered this as trojan horse to push some DMR Agenda into DStar and a burdain to use. Call sign routing is by far more flexible and superior.
- Starnet: You might consider running [Smart Group Server XL](https://github.com/F4FXL/smart-group-server-xl) from a dedicated computer instead.
- Announcement: same can be achieved using VoiceTransmit.
- APRSGateway capability: I would prefer to have some sort of TCP "APRS-IS proxy" program sitting between the program and the APRS server, thus keeping the ability to directly connect to APRS-IS or not, depending on the system owner wish. I run mostly DStar Only repeaters, having an additional program to maintain is unnecessary overkill.
- CallSign Server : this is a legacy from the dead project xreflector.net, I will most probably drop it for good.

### 2.4.2. Additional Features compared to ircddbGateway:
- DPlus, DExtra and G2 NAT Traversal using ircddb network as rendez-vous server. I.e. it is not required to open firewall ports for Callsign Routing or Gateway calls. however it is still recommended to do so. But NAT Traversal will bring more flexibility when operating on CGNAT (Mobile) Networks.
- Forward RSMS1A app messages from/to  APRS-IS Network, yes you can send/receive messages to and from aprs. Yes, you can send messages to APRS stations and Vice Versa. Additionnally, part of the message is sent as Text Dat in the slow data. This allows you to read the message directly on your radio screen.
- Repeater Link status is sent to APRS-IS as a status frame

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
## 3.3. Get latest development version
```
git checkout develop
```

## 3.4. Prerequisites and dependencies
Before first time building you need to install dependencies and prerequisites
```
sudo apt install build-essential libcurl4-openssl-dev libboost-dev
```
If you are going to build with gpsd support, also install libgps-dev
```
sudo apt install libgps-dev
```
## 3.5. Building
Regular building
```
make
```
#### 3.5.0.1. Build With GPSD Support
```
make USE_GPS=1
```
#### 3.5.0.2. Debug Build
```
make ENABLE_DEBUG=1
```
Note that this will will add libl dependency. Building this way will output the stack trace in case of a crash.
## 3.6. Installing
The program is meant to run as a systemd service. All bits an pieces are provided.
```
sudo make install newhostfiles
```
## 3.7. Configuring
After installing you have to edit the configuration file. If you went with default paths, the config file is located in `/usr/local/etc/dstargateway.cfg`

The configuration format is quite straight forward. It is organised in sections and key/value pairs.
The order of the sections or key/values pairs inside the sections does not matter nor does casing.
Boolean values can be set using true, false, 1 or 0
Floating point values must use . (point) as decimal separator.

When done with configuration, the daemon will be started automatically on next boot. To manual start and stop it, use the usual systemd commands
```
sudo systemctl start dstargateway.service
sudo systemctl stop dstargateway.service
```

## 3.8. Updating host files
To update host files, from within the source code directory, run
```
sudo make newhostfiles
sudo systemctl restart dstargateway.service
```

# 4. Contributing
## 4.1. Work Flow
I Use [Git flow](https://danielkummer.github.io/git-flow-cheatsheet/) as my workflow. PR are welcome but pleasee observe following rules :
- You have tested your code thoroughly
- Compilation produces no warnings
- Code formating rules are observed (these are very lousy though)
## 4.2. Continuous Integration
I have added some basic CI using CircleCI [![F4FXL](https://circleci.com/gh/F4FXL/DStarGateway.svg?style=svg)](https://app.circleci.com/pipelines/github/F4FXL/DStarGateway?filter=all) I am trying to rewrite the code so that it can be put into some Behavior Driven Development scheme. This is a long haul task and I'll try do do it on the go while changing/adding stuff.
The testing framwework used is Google Test.

# 5. Version History
## 5.1. Version 1.0
- [**Improvement**] Improve CI to include all variants of build configurations ([#40](https://github.com/F4FXL/DStarGateway/issues/40))
- [**Bugfix**] Fix #37 Unable to transmit multiple files (DGWVoiceTransmit) ([#37](https://github.com/F4FXL/DStarGateway/issues/37))
- [**Bugfix**] Fix #36 Error1 Build fails in some environment ([#36](https://github.com/F4FXL/DStarGateway/issues/36))
- [**Bugfix**] Fix #38 g++ 13 build ([#38](https://github.com/F4FXL/DStarGateway/issues/38))
- [**Bugfix**] Add support for libgps API version greater than ([#39](https://github.com/F4FXL/DStarGateway/issues/39))
## 5.2. Version 0.7
- [**Bugfix**] Unknow repeater entries in log when using Icom Hardware ([#34](https://github.com/F4FXL/DStarGateway/issues/34))
- [**Bugfix**] Malformed callsign in some cases when using DV-G (NMEA) ([#33](https://github.com/F4FXL/DStarGateway/issues/33))
- [**Bugfix**] Crash on startup with Icom Hardware. Thanks to Josh AB9FT for reporting the issue.([#31](https://github.com/F4FXL/DStarGateway/issues/31))
- [**Improvement**] Add/Fix DPRS Object support([#28](https://github.com/F4FXL/DStarGateway/issues/28))
- [**Improvement**] Log incoming DPRS frames so they can be used in e.g. dashboards([#29](https://github.com/F4FXL/DStarGateway/issues/29))
## 5.3. Version 0.6
- [**Improvement**] Add DRats Support ([#24](https://github.com/F4FXL/DStarGateway/issues/24))
- [**Improvement**] Add call sign lists ([#22](https://github.com/F4FXL/DStarGateway/issues/22))
- [**Improvement**] Add a way to override Slow Data in VoiceTransmit ([#23](https://github.com/F4FXL/DStarGateway/issues/23))
- [**Improvement**] Add time server
- [**Improvement**] Gracefully exit on SIGINT and SIGTERM ([#21](https://github.com/F4FXL/DStarGateway/issues/21)). DStarGateway can also be run as a "forking" daemon. This might be required for distros still using sysv. Systemd can live without it.
- [**Improvement**] Add text transmit utility dgwtexttransmit ([#18](https://github.com/F4FXL/DStarGateway/issues/18))
- [**Improvement**] Add voice transmit utility dgwvoicetransmit ([#18](https://github.com/F4FXL/DStarGateway/issues/18))
## 5.4. Version 0.5
- [**Improvement**] Add remote control utility dgwremotecontrol ([#17](https://github.com/F4FXL/DStarGateway/issues/17))
- [**Bugfix**] Two simultaneous incoming G2 streams would fail to be transmitted on dual band repeaters ([#16](https://github.com/F4FXL/DStarGateway/issues/16))
- [**Improvement**] Add NAT Traversal for G2 and DExtra, using IRCDDB as a Rendez Vous server ([#5](https://github.com/F4FXL/DStarGateway/issues/5))
- [**Improvement**] Add forwarding of RS-MS1A messages to APRS-IS ([#9](https://github.com/F4FXL/DStarGateway/issues/9))
- [**Bugfix**] Failed to download XLX Hosts when URL contains a = sign ([#14](https://github.com/F4FXL/DStarGateway/issues/14))
- [**Bugfix**] Remote control connection failed ([#13](https://github.com/F4FXL/DStarGateway/issues/13))
- [**Bugfix**] Trying to connect to ghost ircDDB when no ircDDB is configured
## 5.5. Version 0.4
- [**Improvement**] Add APRS status link feature ([#8](https://github.com/F4FXL/DStarGateway/issues/8))
- [**Bugfix**] Positions received over radio were not sent to APRS-IS when GPDS connection failed. ([#7](https://github.com/F4FXL/DStarGateway/issues/7))
- [**Improvement**] Bring back GPSD support ([#6](https://github.com/F4FXL/DStarGateway/issues/6))
- [**Improvement**] Log enhancements ([#4](https://github.com/F4FXL/DStarGateway/issues/4))
## 5.6. Version 0.3
- [**Improvement**] Get rid of libconfig++ dependency. When upgrading from earlier version you need to manualy delete the config file before reinstalling.
## 5.7. Version 0.2
- [**Bugfix**] ircDDBFreeze when repeater not found ([#1](https://github.com/F4FXL/DStarGateway/issues/1))
- Code sanitization
## 5.8. Version 0.1
First working version
# 6. Future
I started this during my 2021 seasons holiday. It took me almost 8 days to get to a workable version. Here are a couple of stuff I'd like to do :
- &#9745; Better NatTraversal
  - No banging on every gateway: use ircDDB (or something else) as mitigation server to notify peer
  - Support for all protocols (G2, DExtra, DPlus) DCS does nto make sense as it was historically never used as protocol for linking repeaters
- &#9745; Send the connection status to APRS-IS as a status frame
- &#9745; Reinstantiate DRATS
- &#9745; Migrate all the "accessories" (VoiceTransmit, RemoteControl ...)
- &#9746; Automatic refresh of host files
- &#9746; Reduce ircDDB dependency, build something more P2P, maybe based on [Distributed Hashtable](https://github.com/DavidKeller/kademlia) ?
- &#9745; Forward messages from RS-MS1A to APRS and vice versa
- Everything that might come handy to make dstar the most powerful system ever :)
