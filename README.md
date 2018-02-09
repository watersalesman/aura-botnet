# Aura Botnet

[![Travis CI](https://travis-ci.org/watersalesman/aura-botnet.svg?branch=master)](https://travis-ci.org/watersalesman/aura-botnet)
[![AppVeyor](https://ci.appveyor.com/api/projects/status/8jvylxhgckob2eht/branch/master?svg=true)](https://ci.appveyor.com/project/watersalesman/aura-botnet/branch/master)
[![Docker](https://img.shields.io/docker/automated/watersalesman/aura-c2.svg)](https://hub.docker.com/r/watersalesman/aura-c2/)

C2 Server
---
The botnet's C2 server utilizes the Django framework as the backend.
It is far from the most efficient web server, but this is offset by the
following:
* Django is extremely portable and therefore good for testing/educational
purposes. The server and database are contained within the [`aura-server`](aura-server)
folder.
* Django includes a very intuitive and powerful admin site that can be used
for managing bots and commands
* The server is only handling simple POST requests and returning text
* Static files should be handled by a separate web server (local or remote) that
excels in serving static files, such as nginx

The admin site located at `http://your_server:server_port/admin` can be
accessed after setting up a superuser (see below).

Database
---
The C2 server is currently configured to use a SQLite3 database,
`bots.sqlite3`. The current configuration can be changed in [`aura-server/aura/settings.py`](aura-server/aura/settings.py).
You may wish to use MySQL, or even PostgreSQL instead; this easy to do thanks
to Django's portable database API.

Bot Clients
---
The primary client is written in C++, and can be compiled for either Linux or
Windows using CMake. Alternate clients are written in Rust, Bash, and Powershell,
but are may lack certain functionality as they are mostly unsupported. I will fix
any major bugs that come to my attention, but they will continue to lack certain
features for the time being, such as running commands in different shells.

The client will gather relevant system information and send it to the C2 server
to register the new bot. Identification is done by initially creating a file
containing random data -- referred to as the *auth file* throughout the code -- which
will then be hashed each time the client runs to identify the client and
authenticate with the C2 server. It will then install all the files in the
folder specified in the code, and initialize the system service or schedule a
task with the same privileges that the client was run with. The default settings
have the client and other files masquerading as configuration files.

Getting Started: C2 Server
---
[Read documentation here](docs/c2-server.md)

Geting Started: Bot Clients
---
[Read documentation here](docs/bot-client.md)

Other Notes
---
Because this is for testing purposes, the C2 server needs to be
hard-coded into client and web delivery files. It is currently set to
*localhost* on all the files. This is because an actual botnet would use something
like a domain generation algorithm (DGA) to sync a stream of changing domains on
the client side with a stream of disposable domains being registered -- or just
really bulletproof hosting like the original Mirai botnet.

The code is also not obfuscated nor is there any effort put toward preventing
reverse engineering; this would defeat the purpose of being a botnet for
testing and demonstrations.

The *killswitch* folder contains scripts for easy client removal when testing
on your devices.

<h2>This repo is for testing/demonstration purposes only. This is not meant to
be implemented in any real world applications except for testing on authorized
machines.</h2>
