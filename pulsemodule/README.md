# Pulseaudio module 
This implemenation of the equalizer in a pulseadudio module allow you to use my algorithm in your devices that support pulseaudio.

To install this module you need to build the last version of pulseaudio source and add the equalizer in the tree, to do this you can easly follow the instruction below.

## Installation
- Make sure that you have not a pulseaudio version installed, look you package manager 
  ```
  dpkg -l | grep pulseaudio (on Debian)
  pacman -Qs pulseaudio (on Arch)
  ```
  If you have pulseaudio installed you need to uninstall it

- Clone the pulseaudio repository in a directory
  ```
  git clone git://anongit.freedesktop.org/pulseaudio/pulseaudio
  ```
  
- Download and install these patch
  ```
  cd pulseaudio
  wget https://patchwork.freedesktop.org/patch/155160/raw/ -O patch1
  wget https://patchwork.freedesktop.org/patch/154921/raw/ -O patch2
  patch -p1 < patch1
  patch -p1 < patch2
  
  cd src
  wget https://raw.githubusercontent.com/andrea993/audioeqpro/master/pulsemodule/makefile.patch -O patch3
  patch -pq < patch3
  ```

- Copy the module source in the directory
  ```
  cd modules
  wget https://raw.githubusercontent.com/andrea993/audioeqpro/master/pulsemodule/module-eqpro-sink.c
  ```
- Return to main pulseaudio and install
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
To use the module load it with pactl and parameters then se it as default sink, for example:
```
pactl load-module module-eqpro-sink par="(1;1;1;1;0;-1;-1;-1;-1)" db=15
pactl set-default-sink $(SINKNUMBER)
```
Use `pactl set-default-sink` to find the sink number.
  
