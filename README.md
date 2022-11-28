<p align="center"><img width=100% src="img/banner.png"></p>

[![GitHub all releases](https://img.shields.io/github/license/nirvana77/filesystem)](https://github.com/Nirvana77/Filesystem)
[![GitHub code size in bytes](https://img.shields.io/github/languages/code-size/nirvana77/filesystem)](https://github.com/Nirvana77/Filesystem)
[![GitHub issues](https://img.shields.io/github/issues-raw/nirvana77/filesystem)](https://github.com/Nirvana77/Filesystem/issues)

# Filesystem
Easy to use, user based file shearing system. [Doc](https://www.overleaf.com/read/wskjmkvbnwpy)

# Installation
```
$ sudo apt-get update && sudo apt-get upgrade -y
$ sudo apt-get install -y libjansson-dev gcc libcurl4-gnutls-dev git uuid-dev
$ git clone https://github.com/Nirvana77/Filesystem.git
$ cd Filesystem
$ ./compile
```

# Configuration
- `Port`: Sets the `port` for the communication with the server.
- `IP`: Sets the `IP` for the communication with the server.
- `version`: definse the JSON `version` for the settings.
- `Path`: Set the root `Path` for the system.
- `Servers`: An temporary array for connection to diffrent servers. 
    - `IP`: IP to connect to the server.
    - `port`: The port number

# Usage
When useing the service the host `port` needs to be oppen from the firewall. [Linux](https://www.cyberciti.biz/faq/how-to-open-firewall-port-on-ubuntu-linux-12-04-14-04-lts/), [Windows](https://learn.microsoft.com/en-us/answers/questions/291348/can39t-open-ports-in-windows-10.html)

- `Mode`: If is `deamon` or none.
- `Path`: Is the path for the filesystem.

```
& ./bin/main <Path> <Mode>
```

# Contribute
1. Fork the repo and create a new branch: `$ git checkout -b name_for_new_branch`.
2. Make changes and test
3. Submit Pull Request with comprehensive description of changes

# Issues or Bugs
1. Go to the [Issues](https://github.com/Nirvana77/Filesystem/issues) page in the repo.
2. Create a new issues with descriptiv title and steps for the Issues or Bug, gits [ syntax](https://docs.github.com/en/get-started/writing-on-github/getting-started-with-writing-and-formatting-on-github/basic-writing-and-formatting-syntax) can be used.

# Donations

# License
[MIT](LICENSE)