
# Installation
```
$ sudo apt-get update && sudo apt-get upgrade -y
$ sudo apt-get install -y libjansson-dev gcc libcurl4-gnutls-dev git uuid-dev
$ git clone https://github.com/Nirvana77/DistroFiles.git
$ cd DistroFiles
$ ./compile
```

# Usage
When useing the service the host `port` needs to be oppened for the firewall. [Linux](https://www.cyberciti.biz/faq/how-to-open-firewall-port-on-ubuntu-linux-12-04-14-04-lts/), [Windows](https://learn.microsoft.com/en-us/answers/questions/291348/can39t-open-ports-in-windows-10.html)
To launched the GUI

- `Mode`: If is `deamon` or none.
- `Path`: Is the path for the booted DistroFiles. Here is where the settings file will be and the root structor.

```
$ ./bin/main <Path> <Mode>
```
# Configuration
The configuration file is in the `Path` when the DistroFiles was booted. The file name is `settings.json`.
- `version`: Definse the JSON `version` for the settings.
- `host`: Sets the `port` for the communication with the servers.
- `GUI`: Sets the `port` for the communication with the GUI.
- `Servers`: An temporary array for connection to diffrent servers. 
    - `IP`: IP to connect to the server.
    - `port`: The port number
- `autosync`: Sets if the service will autosync.
- `interval`: The interval for the autosync.
# Development
When working on an implemtention or want to debug the service.
- `DebugMode`: Changes the mode of debuging.
    - `d`: Normal debug mode. This creates the `payload_dump.txt` and `AllocatorDebug.txt` files.
    - `gdb`: GDB debug mode. Dose the same as `d` but enable the GDB. 
- `r`: Will auto run the service after compiling
```
$ git checkout <Branch>
$ ./compile <DebugMode> r
```
## Allocator File
This file will always be created then compiled with `gdb` or `d`. This file helps with what have been mallocated and free in the memory. The `AllocatorDebug.txt` is structured like this: 
"`Allocator_Event`, `SizeInByte`, `FilePath`, `FunctionCall`, `PointerInMemory`". 
Where `Allocator_Event` can be `0` for Malloc and `1` Free.

## End Print
Then the service is done it will then print if the developer has missted to freed any memory aswell as a status of memory usage. The unfreed memory is handeled and freed, but this will not only be the case when the service is in the debug mode.
```
-------------------Not freed memory----------------------
28,Libs/TCP/TCPSocket.c,5,TCPSocket_InitializePtr,0x7ffff000368a
160,Libs/DistroFiles/DistroFiles_Connection.c,9,DistroFiles_Connection_InitializePtr,0x7ffff000387a
255,Libs/Buffer.c,27,Buffer_Initialize,0x7ffff0003aea
16,Libs/EventHandler.c,33,EventHandler_Hook,0x7ffff000467a
24,Libs/LinkedList.c,233,LinkedList_CreateNode,0x7ffff000485a
---------------------------------------------------------


-------------------------Stats---------------------------
Max malloced memory: 1048b at Libs/DistroFiles/DistroFiles_Server.c,29,DistroFiles_Server_InitializePtr
Min malloced memory: 16b at Libs/EventHandler.c,29,EventHandler_Hook
Average malloced memory: 106b
Max memory: 2355b
Total malloced memory: 8090b
Number of malloced memory: 77 times
---------------------------------------------------------
```