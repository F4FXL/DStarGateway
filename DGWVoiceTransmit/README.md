
DGWVoiceTransmit allow you to transmit .dvtool files through your repeater.
It must be run on the same machine where DStarGateway is running.

- [1. Usage Examples](#1-usage-examples)
  - [1.1. Transmit one or more files](#11-transmit-one-or-more-files)
  - [1.2. Override or insert Text Data](#12-override-or-insert-text-data)
  - [1.3. Override or insert DPRS Data](#13-override-or-insert-dprs-data)

# 1. Usage Examples
Here are a few usage examples and explanation
## 1.1. Transmit one or more files
This wil transmit one file after the other tthoug repeater N0CALL B. You can either specify the call sign using spaces or underscores. When using spaces, make sure you put it into quotes.
```
dgwvoicetransmit N0CALL_B file1.dvtool file2.dvtool
dgwvoicetransmit "N0CALL B" file1.dvtool file2.dvtool
```
## 1.2. Override or insert Text Data
It is possible to ovveride the text slow data contained in the .dvtool file. For this, use -text flag. If no text data is present, the specified text data will be inserted into the transmsssion
```
dgwvoicetransmit N0CALL_B -text "Override text data" file1.dvtool
```

## 1.3. Override or insert DPRS Data
You cann also override or insert DPRS data. This will cause modern DStar Radio to display a popup with distance, bearing and other DPRS information etc.

You have to insert an "Icom compatible" DPRS frame. No validatin is made wether the frame is valid or not.
```
dgwvoicetransmit N0CALL_B -dprs "!4898.03N/00844.64Er/DPRS Comment" file1.dvtool
This can be combined with -text flag.
```