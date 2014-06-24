#Step by Step Instructions for Changing VGA2USB Permissions Automatically

None of this needs to be done if the vga2usbPermissions.sh file is run with all files downloaded from the repository.

##Script Setup

Open Terminal and navigate to PAOL-CODE folder.
```
$ cd
$ cd paol-code/scripts/install/vga2usbPermissions/
```

Create a file named "chmodBash" and place the following lines in it
```
#!/bin/bash

sudo chmod 777 /dev/vga2usb*
```

Change permissions on that file:
```
$ sudo chmod 001 chmodBash
```

##Rule Assignment

Create a file named 85-paol_rule.rules and put the following in it:
```
ACTION=="add", \
SUBSYSTEM=="usb", \
ATTRS{idVendor}=="5555", \
RUN+="/home/paol/paol-code/scripts/install/vga2usbPermissions/chmodBash"
```

Copy the rules file to the correct location and set it running:
```
$ sudo cp 85-paol_rule.rules /etc/udev/rules.d/
$ sudo /etc/init.d/udev restart
```

##Test

Plug in or replug in a vga2usb device with active video feed in. Check permissions on the file:
```
$ ls -l /dev
```

