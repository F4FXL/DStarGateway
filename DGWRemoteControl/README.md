This is the remote control software for DstarGateway.
After installation edit the config file with the detail of your gateway(s). If you did not alter build settings config file is locate in /usr/local/etc/dgwremotecontrol.cfg

Usage examples :
```
# connect repeater F4ABC B to reflector dcs208 c and reconnect to defaut reflector after 30 minutes
dgwremotecontrol F4ABC__B link 30 dcs208_c

# unlink reflector
dgwremotecontrol F4ABC__B unlink

# connect repeater F4ABC B of gateway hill_top to reflector dcs208 c and reconnect to defaut reflector after 30 minutes
dgwremotecontrol -name hill_top F4ABC__B link 30 dcs208_c

```
