# Aura C2 Server
This is a lightweight image for the [Aura C2 server](https://github.com/watersalesman/aura-botnet).

The image exposes port 41450 and has a default Django
admin user "admin" with password "pass". The admin user can be changed through
the Django admin site located at `http://your_server:your_django_port/admin`.

Example Usage
===
The following command creates a container named *c2-server1*, binds the host's
port 41450 to port 41450 in the container, and runs it in the background,
```
docker run --name c2-server1 -p 41450:41450 -d watersalesman/aura-c2
```

You can also run the container in the foreground with an interactive pseudo-TTY
to see requests being sent to the server. This is good for debugging and allows
you to kill the server with a simple CTRL-C. Start the container like so:
```
docker run --name c2-server1 -p 41450:41450 -it watersalesman/aura-c2
```
***
