
# Installation
```
$ sudo apt-get update && sudo apt-get upgrade -y
$ sudo apt-get install -y libjansson-dev gcc libcurl4-gnutls-dev git uuid-dev
$ git clone https://github.com/Nirvana77/Filesystem.git
$ cd Filesystem
$ ./compile
```
# Configuration
- `version`: definse the JSON `version` for the settings.
- `host`: Sets the `port` for the communication with the servers.
- `GUI`: Sets the `port` for the communication with the GUI.
- `Servers`: An temporary array for connection to diffrent servers. 
    - `IP`: IP to connect to the server.
    - `port`: The port number
- `autosync`: Sets if the service will autosync.
- `interval`: The interval for the autosync.

# Usage
When useing the service the host `port` needs to be oppened for the firewall. [Linux](https://www.cyberciti.biz/faq/how-to-open-firewall-port-on-ubuntu-linux-12-04-14-04-lts/), [Windows](https://learn.microsoft.com/en-us/answers/questions/291348/can39t-open-ports-in-windows-10.html)
To launched the GUI

- `Mode`: If is `deamon` or none.
- `Path`: Is the path for the filesystem.

```
$ ./bin/main <Path> <Mode>
```
# Development
When working on an implemtention or want to debug the service.
- `DebugMode`: Changes the mode of debuging.
    - `d`: Normal debug mode. This creates the `payload_dump.txt` and `AllocatorDebug.txt` files.
    - `gdb`: GDB debug mode. Dose the same as `d` but enable the GDB. 
- `r`: Will auto run the service after compiling
```
$ ./compile <DebugMode> r
```