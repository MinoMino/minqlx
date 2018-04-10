minqlx
======
-
minqlx is a modification to the Quake Live Dedicated Server that extends Quake Live's dedicated server with
extra functionality and allows scripting of server behavior through an embedded Python
interpreter.

The mod has been tested on Debian 7 and 8, Ubuntu 14.04 and 16.10. At the moment it only officially supports the x64 
build of the server, but you can try building [this fork](https://github.com/em92/minqlx/tree/x86-support) for x86 support.

As of writing, development of the core is pretty much dead save for the occasional pull request.

If you have any questions, the IRC channel for the old bot,
[#minqlbot on Quakenet](http://webchat.quakenet.org/?channels=minqlbot),
is being used for this one as well. Feel free to drop by.

Installation
============
These instructions are for Debian 7 or 8 (use the latter if you can choose). For Ubuntu,
see the [wiki entry](https://github.com/MinoMino/minqlx/wiki/Ubuntu) for details.
You're on your own for the time being on other distros,
but feel free to add instructions to the [wiki](https://github.com/MinoMino/minqlx/wiki)
if you want to help out.

- Install Python 3.5. At the time of writing, in Debian 7 or 8 you can install it by adding the `sid` repository to apt. This can be done by adding the line `deb http://ftp.debian.org/debian sid main` to
`/etc/apt/sources.list`. You can now go ahead and install it with the following:

```
sudo apt-get update
sudo apt-get -y install python3.5 python3.5-dev
```

- You should remove `sid` by commenting out or removing the line you added to `sources.list`
earlier and then do `sudo apt-get update` again to make sure you don't install any unstable
packages unintentionally later.

- Now you should get Redis and Git, which will be used by minqlx's plugins:

```
sudo apt-get -y install redis-server git
```

- Download one of the tarballs in [releases](https://github.com/MinoMino/minqlx/releases) and extract
its contents into `steamcmd/steamapps/common/qlds`, or whatever other directory you might have put the
files of your server in.

- Clone the plugins repository and get/build Python dependencies. Assuming you're in
the directory with all the server files (and where you extracted the above files) do:

```
git clone https://github.com/MinoMino/minqlx-plugins.git
wget https://bootstrap.pypa.io/get-pip.py
sudo python3.5 get-pip.py
rm get-pip.py
sudo apt-get -y install build-essential
sudo python3.5 -m easy_install pyzmq hiredis
sudo python3.5 -m pip install -r minqlx-plugins/requirements.txt
```

**NOTE**: During the pip and easy_install steps, you might get a lot of warnings. You can safely
ignore them.

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

To update the core, just use `wget` to get the latest binary tarball and put it in your QLDS directory, then simply
extract it with `tar -xvf <tarball>`. To update the plugins, use `cd` to change the working directory to `qlds/minqlx-plugins`
and do `git pull origin` and you should be good to go. Git should not remove any untracked files, so you can have your
own custom plugins there and still keep your local copy of the repo up to date.

You can also try running [these scripts](https://gist.github.com/MinoMino/5a8c76da3edd953144ef) from your QLDS directory. It will compile the latest version from source and update plugins. The second script is the same, but using the develop branch instead.

Compiling
=========
**NOTE**: This is *not* required if you are using binaries.

It's just a makefile for now. No autoconf or anything, so you might need to edit the file in some cases.
It assumes you have GCC and that `python3.5-config` is Python 3.5's python-config. On Debian, install
`python3.5-dev` and it should compile right off the bat assuming you have all the build tools.

To compile, just do a `make` and you should get a `minqlx.so` and a `minqlx.zip` in the `bin` directory.
The `bin` directory also has launch scripts, so you can simply copy the contents of the `bin` directory
into the QLDS folder and use those scripts to launch it. If you do not want to use this with
Python, you can compile it with `make nopy` and you should get a `minqlx_nopy.so` instead.

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
