smart-group-server
==================
## Introduction

This smart-group-server is based on an original idea by John Hays K7VE for a routing group server he called **STARnet Digital**. This idea was first coded by Jonathan G4KLX and he called the resulting program **StarNetServer**. The smart-group-server is derrived from Jonathan's code and still contains his original copyrights and GPLV#2 license. This new implementation of a group routing server has many improvements and new features compared to its predecessor. The main features for the end-user is that Smart Groups allow a user to "listen first" before transmitting and also be able to see the status of the Smart Groups and users. The smart-group-server can now also handle connections from mobile clients (hotspots that get their internet connection from a cellphone). The most useful feature for provider is that a single smart-group-server can serve both DCS- **and** DExtra-linked groups and only the required UDP ports are created. In addtion, by using the remote control application, Smart Groups can be unlinked and linked dynamically, freeing and reallocating resources as required.  It was designed expressly for QuadNet. The smart-group-server interact with QuadNet using new IRC messages to provide additional information that will typically be display on the ROUTING GROUPS web page at openquad.net. The smart-group-server may not function proplerly on other IRCDDB networks.

### What's New

* **V# 180407** A bug has been fixed where if you use the "LOGOFF in the text field" method of logging off a smart-group-server, the ROUTING GROUPS page was not being updated properly.

* **V# 180401** In some situations, the smart-group-server does not "following" a user if he switches repeaters. It was clear that the *last repeater used* cache was not being updated properly. This has been fixed by cleaning up how the user cache is used in the CGroupHandler class.

* **V# 180322** The smart-group-server is now compatible with mobile hotspots! You should be able to route to any Smart Group from a smart-phone-tethered hotspot. Thanks goes to Colby Ross, W1BSB for helping with this very important new capability!

* **V# 180218** The CRepeaterHandler class has been removed from the project, along with the CDDDataHandler and DCCSHandler classes. A crash bug, where someone would try to link to a Smart Group module, has been fixed.

* **V# 180205** There was a benign bug causing some linked groups not to properly receive polls from X-reflectors, causing these groups to do an unnecessary re-link at the end of each poll inactivity timer. This has been fixed. I introduced this bug when I did major modifications to the C-Handler, C-ProtocolHandler and C-ProtcolHandlerPool classes. The C-ProtocolHandlerPool classes now use a std::list to keep the C-ProtcolHandler instances, instead of a static array. I consider this release the first public release compareable to what some would call *Version 1.0.0*.

* **V# 180203** A buffer overflow error has been fixed in CConnectData. I introduced this bug when replacing wxWidgets. (I wish the standard library had a format or sprint class method for std::string! It's probably the only thing that wxWidgets has over the C++11 standard library.) Now it's fixed.

* **V# 180118** Smart Groups can now be linked and unlinked by the sgsremote program. See the README of my sgs-remote git repository. You need to unlink before you link and once you unlink a Smart Group you can link it to either an XRF or a DCS reflector. Also **the format of the configuration file has changed**. The callsign and address parameters have been moved from the ircddb section to a new section called gateway. See the example.cfg file for more information. Finally, the install section of the Makefile has been separated into two pieces, one to get the latest Host*.txt files and another to install the smart-group-server program and configuration files, see below.

* **V# 180103** The smart-group-server now supports linking both DExtra and DCS reflectors to different channels *in the same server instance*. The compile time switches for DEXTRA_LINK and DCS_LINK are gone. If you need an unlinked channel, don't define a *reflector* parameter in the configuration.

* **V# 180101** There is no hard limit on how many channels you can have running on a single smart-group-server. There is a practical limit. For instance, you could run out of ports for DExtra or DCS linking. There is also a performance limit when there are so many channels, servicing a single time slice takes longer than a D-Star frame. I don't know when this will happen. Resource allocation is much more efficient. DExtra and DCS resource are only allocated for the channels defined in the configuration file.

* **Original Version** The underlying IRCDDB version has been upgraded to 2.0.0 and supports new IRC Messages that the smart-group-server uses to communicate the channel states to the Quadnet Servers. The dependancy on wxWidgets is gone!

## Server OS Requirements

The smart-group-server requires a modern OS to compile and run. At least Debian 8 or Ubuntu 16.10, or equivilent. The command
```
g++ --version
```
must return at least Version 4.9. The latest Debian and Ubuntu will be far above this. Unlike the StarNetServer, smart-group-server does not use wxWidgits. Modern C++ calls to the standard library (c++11 standard) are used instead of wxWidgets: std::string replaces wxString, std::future replaces wxThreads and standard std::map, std::list, std::queue and std::vector replace the older wx containers. The only external library used is libconfig++. The smart-group-server is significantly improved regarding resource utiliztion compared to the ancestral StarNetServer. The smart-group-server only creates resources for the channel you define in your configuration file. Also, there is no theoretical limit to the number of channels you can create. Of course there is a practical limit based on the underlying hardware.

The smart-group-server is installed as a systemd service. If you want to run this on a system without systemd, you are on your own. I am done dealing with init.d scripts in SysVInit!

## Adminstrative Requirements

This Smart Group Server should have a unique IP address when it logs into QuadNet. That means you probably won't be able to run it from your home if you also have an ircddb gateway running from home. You probably shouldn't run it from your home anyway. The computer your Smart Group Server is running on should have reliable, 24/7 internet access and reliable, 24/7 power. It should also be properly protected from hackers. There are plenty of companies that provide virtual severs that easily fulfill these requirements for verly little money. (You don't need much horse-power for a typical Smart Group Server. For example, a $5/month server on Amazon Lightsail works fine.)

Also the Smart Group Server needs to have a unique callsign in QuadNet, one that will not be used by another client on QuadNet. Ideally, you should use a Club callsign, see the Configuring section below.

## Building

These instructions are for a Debian-based OS. Begin by downloading this git repository:
```
git clone git://github.com/n7tae/smart-group-server.git
```
Install the only needed development library:
```
sudo apt-get install libconfig++-dev
```
Change to the smart-group-server directory and type `make`. This should make the executable, `sgs` without errors or warnings. By default, you will have a group server that can link groups to X-Reflectors or DCS-Reflectors. Of course you can declare an unlinked channel by simply not defining a *reflector* parameter for that channel.

## Configuring

Before you install the group server, you need to create a configuration file called `sgs.cfg`. There is an example configuration file: `example.cfg`. The smart-group-server supports an unlimited number of channels. However there will be a practical limit based on you hardware capability. Also remember that a unique port is created for each DExtra or DCS link on a running smart-group-server. At some point you system will simply run out of connections. Be sure you look and the "StarNet Groups" tab on the openquad.net web page to be sure your new channel callsigns and logoff callsigns are not already in use! Each channel you define requires a band letter. Bands can be shared between channels. Choose any uppercase letter from 'A' to 'Z'. Each channel will have a group logon callsign and a group logoff callsign. The logon and logoff will differ only in the last letter of the callsign. PLEASE DON'T CHOOSE a channel callsign beginning in "REF", "XRF", "XLX", "DCS" or "CCS". While it is possible, it's really confusing for new-comers on QuadNet. Also, avoid subscribe and unsubscribe callsigns that end in "U". Jonathan's ircddbgateway will interpret this as an unlink command and never send it to the smart-group-server.

Your callsign parameter in the ircddb section of your configuration file is the callsign that will be used for logging into QuadNet. THIS NEEDS TO BE A UNIQUE CALLSIGN on QuadNet. Don't use your callsign if you are already using it for a repeater or a hot-spot. Ideally, you should use a Club callsign. Check with your club to see if you can use your club's callsign. Of course, don't do this if your club hosts a D-Star repeater with this callsign. If your club callsign is not available, either apply to be a trustee for a new callsign from you club, or get together with three of your friends and start a club. All the information you need is at arrl.org or w5yi.org. It's not difficult to do, and once you file your application, you'll get your new Club Callsign very quickly.

## Installing and Uninstalling

To install and start the smart-group-server, first type `make newhostfiles`. This will download the latest DCS and DExtra host files and install them. (This command downloads the files to the build directory and then moves them to /usr/local/etc with `sudo`, so it may prompt you for your password.) Then type `sudo make install`. This will put all the executable and the sgs.cfg configuration file the in /usr/local and then start the server. See the Makefile for more information. A very useful way to start it is:
```
sudo make install && sudo journalctl -u sgs.service -f
```
This will allow you to view the smart-group-server log file while it's booting up. When you are satisfied it's running okay you can Control-C to end the journalctl session. To uninstall it, type `sudo make uninstall` and `sudo make removehostfiles`. This will stop the server and remove all files from /usr/local. You can then delete the build directory to remove every trace of the smart-group-server.

73

Tom
n7tae (at) arrl (dot) net
