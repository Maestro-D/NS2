# Compilation
## To compile you need :
### Some packages :
* cmake
* make
* gcc
* g++
* libssh
* openssh

### Then enter these command lines :
1. `make` or `make re`
2. `./build/spatch`

> You can remove compiled files with `make clean`.

> You can remove compiled files and binary file with `make fclean`.

# Packaging

# Package installation

### Build debian package on local host :
`make local_package`

### Build debian package inside virtual environment :
`make remote_package`

### Install and enable package in virtual environment :
`make deployment`

# Configuration
You can find the config file in `./config/spatch/config.ini`.

> Please respect the configuration rules otherwise the configuration reader or the configuration controller will exit in error.

You can specify some configuration by sections like :
  - [server] :

    `port=42`
  - [local_users] :

    `username=password`
  - [endpoints] :

    `endpoint_name=ip_address|port`
  - [users_control] :

    `local_username=available_endpoint_1|available_endpoint_2|...`
  - [remote_users] :

    `local_username|endpoint_name=available_username_1|available_username_2|...`

## Example :
```
[server]
port=42

[local_users]
toto=toto
...

[endpoints]
endpoint1=10.0.2.15|22
...

[users_control]
toto=endpoint1|endpoint2
...

[remote_users]
toto|endpoint1=toto|root
...
```
# Commands

> You can display all commands on spatch by typing `help` in the ssh client.

## Command list

* list :	list all current proxified connections and users
* endpoint [name] :	list all available endpoint OR select an endpoint
* alias [name] :	list all available usernames (aliases) for selected endpoint OR select an alias
* connect :	connect to the previously selected endpoint and username alias
* help :	display this message
* exit :	terminate current connection
