# Selective Repeat TUN/TAP tunnel

This program implements a Selective Repeat algorithm at the IP level within a TUN/TAP tunnel.
A good illustration of the algorithm implemented is given here : [visualizing the go back n and the selective repeat protocol](https://www2.tkn.tu-berlin.de/teaching/rn/animations/gbn_sr/).

## 1. Copyright Information

Copyright (C) 2021
Emmanuel Lochin <emmanuel.lochin@enac.fr>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or any 
later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>. 

## 2. Installation 

Just type `make`. You can edit the Makefile and compile with the following option:

- `-DDEBUG` to add debug messages concerning the SR buffer implemented as a linked list.

## 3. Usage 

As soon as compiled, launch `bin/pss -h` for usage (see below):

```
./bin/sr_tunnel -i <ifacename> [-s|-c <serverIP>] [-p <port>]
./bin/sr_tunnel -h

-i <ifacename>: Name of interface to use (mandatory)
-s|-c <serverIP>: run in server mode (-s), or specify server address (-c <serverIP>) (mandatory)
-p <port>: port to listen on (if run in server mode) or to connect to (in client mode), default 30001
-d <#>: emulate link-layer losses by dropping sent packet, expressed in percentage of drop (from 0 to 100). Default is 0
-b <#>: average burst size. GE model for losses. Default is 1 if -d set
-t <file.out>: enable trace file. Default is stdout
-h: prints this help text
```

## 4. How to edit default SR parameters

You can change SR window and timeoity value in `include/util.h` : 
```
#define MAXWIN 8        /**< maximum SR window */
#define TIMEOUT 500     /**< timeout expressed in ms */
```

## 5. Practical usecase with TUN/TAP

You first need to create a `tun0` interface on the client:
```
sudo ip tuntap add mode tun tun0
sudo ip link set tun0 up
sudo ip addr add 192.168.10.1/24 dev tun0
```
Then same operation on the serveur side with a different IP adress for the tunnel interface:
```
sudo ip tuntap add mode tun tun0
sudo ip link set tun0 up
sudo ip addr add 192.168.10.2/24 dev tun0
```
First launch `bin/sr_tunnel -i tun0 -s` on the server side and `bin/sr_tunnel -i tun0 -c W.X.Y.Z where W.X.Y.Z is the server IP address (e.g. the IP address associated to `eth0` for instance, not the `tun0` IP address).
