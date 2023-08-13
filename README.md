minqlx
======
minqlx is a modification to the Quake Live Dedicated Server that extends Quake Live's dedicated server with
extra functionality and allows scripting of server behavior through an embedded Python
interpreter.

The mod has been tested on Debian 7, 8 and 9, Ubuntu 14.04 and 16.10. Supports both x64 and x86 builds of the server.

As of writing, development of the core is pretty much dead save for the occasional pull request.

If you have any questions, the IRC channel for the old bot,
[#minqlbot on Quakenet](http://webchat.quakenet.org/?channels=minqlbot),
is being used for this one as well. Feel free to drop by.

Installation
============
These instructions are for Debian 9 "Stretch". By default it will install required python 3.5 version.
If you are using Debian 7 "Wheezy" or Debian 8 "Jessie", you need to add `stretch` repository to apt.
This can be done by adding the line `deb http://ftp.debian.org/debian stretch main` to `/etc/apt/sources.list`
For Ubuntu, see the [wiki entry](https://github.com/MinoMino/minqlx/wiki/Ubuntu) for details.
You're on your own for the time being on other distros,
but feel free to add instructions to the [wiki](https://github.com/MinoMino/minqlx/wiki)
if you want to help out.

- Install Python 3.

```
sudo apt-get update
sudo apt-get -y install python3 python3-dev
```

- Make sure, that you have installed Python 3.5 or later:
```
python3 --version
```

- For Debian 7 or 8: You should remove `stretch` by commenting out or removing the line you added to `sources.list`
earlier and then do `sudo apt-get update` again to make sure you don't install any other `stretch`
packages unintentionally later.

- Now you should get Redis, Git and build utils which will be used by minqlx's plugins and for compiling minqlx:

```
sudo apt-get -y install redis-server git build-essential
```

- Clone this repository and compile minqlx

```
git clone https://github.com/MinoMino/minqlx.git
cd minqlx
make
```

- Copy everything from `minqlx/bin` into `steamcmd/steamapps/common/qlds`, or whatever other directory you might have put the
files of your server in.

- Clone the plugins repository and get/build Python dependencies. Assuming you're in
the directory with all the server files (and where you extracted the above files) do:

```
git clone https://github.com/MinoMino/minqlx-plugins.git
wget https://bootstrap.pypa.io/get-pip.py
sudo python3 get-pip.py
rm get-pip.py
export PIP_BREAK_SYSTEM_PACKAGES=1  # Workaround for Debian 12+ users
sudo python3 -m pip install -r minqlx-plugins/requirements.txt
```

**NOTE**: During the pip step, you might get a lot of warnings. You can safely ignore them.

- Redis should work right off the bat, but you might want to edit the config and make
it use UNIX sockets instead for the sake of speed. minqlx is configured through cvars,
just like you would configure the QLDS. This means it can be done either with a server.cfg
or by passing the cvars as command line arguments with `+set`. All the cvars have default
values, except for `qlx_owner`, which is your SteamID64 (there are converters out there, just google it).
Make sure you set that, otherwise you won't be able to execute any admin commands,
since it won't know you are the owner of it.

- You're almost there. Now simply edit the scripts you use to launch the server, but
make it point to `run_server_x64_minqlx.sh` instead of `run_server_x64.sh`.

Configuration
=============
minqlx is configured using cvars, like you would configure the server. All minqlx cvars
should be prefixed with `qlx_`. The following cvars are the core cvars. For plugin configuration
see the [plugins repository](https://github.com/MinoMino/minqlx-plugins).

- `qlx_owner`: The SteamID64 of the server owner. This is should be set, otherwise minqlx
can't tell who the owner is and will refuse to execute admin commands.
- `qlx_plugins`: A comma-separated list of plugins that should be loaded at launch.
  - Default: `plugin_manager, essentials, motd, permission, ban, silence, clan, names, log, workshop`.
- `qlx_pluginsPath`: The path (either relative or absolute) to the directory with the plugins.
  - Default: `minqlx-plugins`
- `qlx_database`: The default database to use. You should not change this unless you know what you're doing.
  - Default: `Redis`
- `qlx_commandPrefix`: The prefix used before command names in order to execute them.
  - Default: `!`
- `qlx_redisAddress`: The address to the Redis database. Can be a path if `qlx_redisUnixSocket` is `"1"`.
  - Default: `127.0.0.1`
- `qlx_redisDatabase`: The Redis database number.
  - Default: `0`
- `qlx_redisUnixSocket`: A boolean that determines whether or not `qlx_redisAddress` is a path to a UNIX socket.
  - Default: `0`
- `qlx_redisPassword`: The password to the Redis server, if any.
  - Default: None
- `qlx_logs`: The maximum number of logs the server keeps. 0 means no limit.
  - Default: `5`
- `qlx_logsSize`: The maximum size in bytes of a log before it backs it up and starts on a fresh file. 0 means no limit.
  - Default: `5000000` (5 MB)

Usage
=====
Once you've configured the above cvars and launched the server, you will quickly recognize if for instance
your database configuration is wrong, as it will start printing a bunch of errors in the server console
when someone connects. If you only see stuff like the following, then you know it's working like it should:

```
[minqlx.late_init] INFO: Loading preset plugins...
[minqlx.load_plugin] INFO: Loading plugin 'plugin_manager'...
[minqlx.load_plugin] INFO: Loading plugin 'essentials'...
[minqlx.load_plugin] INFO: Loading plugin 'motd'...
[minqlx.load_plugin] INFO: Loading plugin 'permission'...
[minqlx.late_init] INFO: Stats listener started on tcp://127.0.0.1:64550.
[minqlx.late_init] INFO: We're good to go!
```

To confirm minqlx recognizes you as the owner, try connecting to the server and type `!myperm` in chat.
If it tells you that you have permission level 0, the `qlx_owner` cvar has not been set properly. Otherwise
you should be good to go. As the owner, you are allowed to type commands directly into the console instead
of having to use chat. You can now go ahead and add other admins too with `!setperm`. To use commands such
as `!kick` you need to use client IDs. Look them up with `!id` first. You can also use full SteamID64s
for commands like `!ban` where the target player might not currently be connected.

[See here for a full command list.](https://github.com/MinoMino/minqlx/wiki/Command-List)

Note that the plugins repository only contains plugins maintained by me. Take a look [here](https://github.com/MinoMino/minqlx/wiki/Useful-Plugins) some of the plugins by other users that could be useful to you.

Updating
========
Since this and plugins use different repositories, they will also be updated separately. However, the latest master
branch of both repositories should always be compatible. If you want to try out the develop branch, make sure you use
the develop branch of both repositories too, otherwise you might run into issues.

To update the core, clone this repository, compile minqlx and copy `minqlx/bin` to `qlds` directory, just
like you did it, when following installing instructions.
To update the plugins, use `cd` to change the working directory to `qlds/minqlx-plugins`
and do `git pull origin` and you should be good to go. Git should not remove any untracked files, so you can have your
own custom plugins there and still keep your local copy of the repo up to date.

You can also try running [these scripts](https://gist.github.com/MinoMino/5a8c76da3edd953144ef) from your QLDS directory. It will compile the latest version from source and update plugins. The second script is the same, but using the develop branch instead.

Contribute
==========
If you'd like to contribute with code, you can fork this or the plugin repository and create pull requests for changes.
Please create pull requests into the `develop` branch and not to `master`.

If you found a bug, please open an issue here on Github and include the relevant part from either the
server's console output or from `minqlx.log` which is in your `fs_homepath`, preferably the latter as it is
more verbose. Note that `minqlx.log` by default becomes `minqlx.log.1` whenever it goes above 5 MB, and keeps doing
that until it goes to `minqlx.log.5`, at which point the 5th one gets deleted if the current one goes over
the limit again. In other words, your logs will keep the last 30 MB of data, but won't exceed that.

Both when compiling and when using binaries, the core module is in a zip file. If you want to modify
the code, simply unzip the contents of it in the same directory and then delete the zip file. minqlx will
continue to function in the same manner, but using the code that is now in the `minqlx` directory.
