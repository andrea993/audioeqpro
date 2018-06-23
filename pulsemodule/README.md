# Pulseaudio module 
This implemenation of the equalizer in a pulseadudio module allow you to use my algorithm in your devices that support pulseaudio.

To install this module you'll have to build the latest version of pulseaudio sources and add the equalizer in the tree, to do so you can easily follow the instructions below.

## Installation
- Make sure you don't have any pulseaudio packages installed, check your packages manager 
  ```
  dpkg -l | grep pulseaudio (on Debian)
  pacman -Qs pulseaudio (on Arch)
  ```
  If you have pulseaudio installed you have to uninstall it

- Clone pulseaudio repository in a directory
  ```
  git clone git://anongit.freedesktop.org/pulseaudio/pulseaudio
  ```

- Download and run 8pathces script
  ```
  cd pulseaudio
  wget https://raw.githubusercontent.com/andrea993/audioeqpro/master/pulsemodule/8patches.sh
  sh 8patches.sh
  ```
  
- Download and install this patch
  ```
  wget https://raw.githubusercontent.com/andrea993/audioeqpro/master/pulsemodule/makefile.patch -O makefile.patch
  patch -p1 < makefile.patch
  ```

- Copy the module sources in the directory
  ```
  cd src/modules
  wget https://raw.githubusercontent.com/andrea993/audioeqpro/master/pulsemodule/module-eqpro-sink.c
  ```
- Return to main pulseaudio directory and install
  ```
  cd ../..

  ./autogen.sh --prefix=/usr \
               --sysconfdir=/etc \
               --libexecdir=/usr/lib \
               --localstatedir=/var \
               --with-udev-rules-dir=/usr/lib/udev/rules.d \
               --with-database=tdb \
               --disable-hal-compat \
               --disable-tcpwrap \
               --disable-bluez4 \
               --disable-rpath \
               --disable-default-build-tests
   
   make -j4
   sudo make install bashcompletiondir=/usr/share/bash-completion/completion
   ```
   
## Usage
To use this module just load it with pactl then set it as default sink, for example:
```
pactl load-module module-eqpro-sink par="(1;1;1;1;0;0)" db=15
pactl set-default-sink $(SINKNUMBER)
```
Use `pactl list sinks` to find the sink number.
  
Possible arguments:
```
"sink_name=<name for the sink> "
"sink_properties=<properties for the sink> "
"master=<name of sink to filter> "
"rate=<sample rate> "
"channels=<number of channels> "
"channel_map=<channel map> "
"use_volume_sharing=<yes or no> "
"force_flat_volume=<yes or no> "
"db=<filter gain in decibel>"
"fmin=<central frequecy of the first band> "
"octave=<octaves between bands> "
"Nbands=<number of bands>"
"par=<equalizer levels in the form (x1;x2;...;xn) from -1 to 1>"
```
