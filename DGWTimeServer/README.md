Time Server for DStarGateway. Installs as a systemd service.

If you did not change build settings, the configuration can be found in
```
/usr/local/etc/dgwtimeserver.cfg
```
After configuring enable and start the dgwtimeserver systemd service using :
```
sudo systemctl enable dgwtimeserver
sudo systemctl start dgwtimeserver
```