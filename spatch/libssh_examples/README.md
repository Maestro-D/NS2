# `libssh` examples

## What is this?
This document aims to explain how to compile and use the `libssh` client and
server examples available in this directory

## How to compile?
### Requirements
- You need to have `libssh` installed on your system
- Tested on Linux with `gcc`

### Sample ssh client
```
$ cd sample_ssh_client
$ make
```
- Executable file is `sample_ssh_client`

### Sample ssh server
```
$ cd sample_ssh_server
$ mkdir build
$ cd build
$ cmake ..
$ make VERBOSE=1
```
- Executable file is `sample_sshd`

## How to use?
### Requirements
- You need to have `openssh` installed on your system

### Sample ssh client
#### Run a classic ssh server
For my tests I commented almost everything in `/etc/ssh/sshd_config`, what remains
in the file is
```sh
HostKey /etc/ssh/ssh_host_rsa_key
HostKey /etc/ssh/ssh_host_dsa_key
HostKey /etc/ssh/ssh_host_ecdsa_key
HostKey /etc/ssh/ssh_host_ed25519_key

AuthorizedKeysFile	.ssh/authorized_keys
ChallengeResponseAuthentication yes
UsePAM yes
```

Then launch the server
```
$ sudo systemctl start sshd.socket # or sshd.service
$ sudo systemctl status sshd.socket # or sshd.service
● sshd.socket
   Loaded: loaded (/usr/lib/systemd/system/sshd.socket; disabled; vendor preset: disabled)
   Active: active (listening) since Sun 2017-02-05 19:46:00 CET; 1s ago
   Listen: [::]:22 (Stream)
 Accepted: 4; Connected: 0

Feb 05 19:46:00 Nico-PC systemd[1]: Listening on sshd.socket.
```

#### Run sample client
```
$ cd sample_ssh_client
$ ./sample_ssh_client
ssh_new() success!
ssh_options_set() success!
[2017/02/05 19:49:59.133738, 2] ssh_connect:  libssh 0.7.3 (c) 2003-2014 Aris Adamantiadis, Andreas Schneider, and libssh contributors. Distributed under the LGPL, please refer to COPYING file for information about your rights, using threading threads_noop
[2017/02/05 19:49:59.133943, 2] ssh_socket_connect:  Nonblocking connection socket: 3
[2017/02/05 19:49:59.133952, 2] ssh_connect:  Socket connecting, now waiting for the callbacks to work
[2017/02/05 19:49:59.133972, 1] socket_callback_connected:  Socket connection callback: 1 (0)
[2017/02/05 19:49:59.138119, 1] ssh_client_connection_callback:  SSH server banner: SSH-2.0-OpenSSH_7.4
[2017/02/05 19:49:59.138143, 1] ssh_analyze_banner:  Analyzing banner: SSH-2.0-OpenSSH_7.4
[2017/02/05 19:49:59.138158, 1] ssh_analyze_banner:  We are talking to an OpenSSH client version: 7.4 (70400)
[2017/02/05 19:49:59.184527, 2] ssh_packet_dh_reply:  Received SSH_KEXDH_REPLY
[2017/02/05 19:49:59.186507, 2] ssh_client_curve25519_reply:  SSH_MSG_NEWKEYS sent
[2017/02/05 19:49:59.186525, 2] ssh_packet_newkeys:  Received SSH_MSG_NEWKEYS
[2017/02/05 19:49:59.186754, 2] ssh_packet_newkeys:  Signature verified and valid
ssh_connect() success!
Password:
```
- The client should automatically connects with the server
- The server should take the default local account as default username
- Type you password
- The client should print the result of an `ls` command in your Shell

### Sample ssh server
#### Run sample server
```
$ ./generate_host_keys.bash
$ cd sample_ssh_server/build
$ ./sample_sshd -v -r ../../host_keys/ssh_host_rsa_key -d ../../host_keys/ssh_host_dsa_key -p 42010 localhost
Started sample libssh sshd on port 42010
You can login as the user libssh with the password libssh

```

#### Connect classic ssh client to server
At the same time in another console

```
$ ssh libssh@localhost -p 42010


Keyboard-Interactive Fancy Authentication

Please enter your real name and your password
Real name:
Password:


Keyboard-Interactive Fancy Authentication

OK, this is not YOUR name, but it's a reference to the HGTG...
The main character's full name:
Password:


Keyboard-Interactive Fancy Authentication

OK, this is not YOUR name, but it's a reference to the HGTG...
The main character's full name:
Password:
libssh@localhost's password:
PTY allocation request failed on channel 0


ls
ls
sdcsdv
sdcsdv
allo?
allo?
exit
exit
^CKilled by signal 2.

```
- Press `ENTER` each time some input is prompted
- Except for `libssh@localhost's password: ` type `libssh`
- After that you enter in an echo mode with server
- You can exit via `^D` or `^C`

*Note: You need to be quick while entering the needed input or the server might
close the connection*
