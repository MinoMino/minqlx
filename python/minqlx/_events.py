# minqlx - Extends Quake Live's dedicated server with extra functionality and scripting.
# Copyright (C) 2015 Mino <mino@minomino.org>

# This file is part of minqlx.

# minqlx is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# minqlx is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with minqlx. If not, see <http://www.gnu.org/licenses/>.

import minqlx
import re

_re_vote = re.compile(r"^(?P<cmd>[^ ]+)(?: \"?(?P<args>.*?)\"?)?$")

# ====================================================================
#                               EVENTS
# ====================================================================

class EventDispatcher:
    """The base event dispatcher. Each event should inherit this and provides a way
    to hook into events by registering an event handler.

    """
    no_debug = ("frame", "set_configstring", "stats", "server_command", "death", "kill", "command", "console_print", "damage")
    need_zmq_stats_enabled = False

    def __init__(self):
        self.name = type(self).name
        self.need_zmq_enabled = type(self).need_zmq_stats_enabled
        self.plugins = {}

    def dispatch(self, *args, **kwargs):
        """Calls all the handlers that have been registered when hooking this event.
        The recommended way to use this for events that inherit this class is to
        override the method with explicit arguments (as opposed to the this one's)
        and call this method by using ``super().dispatch()``.

        Handlers have several options for return values that can affect the flow:
            - minqlx.RET_NONE or None -- Continue execution normally.
            - minqlx.RET_STOP -- Stop any further handlers from being called.
            - minqlx.RET_STOP_EVENT -- Let handlers process it, but stop the event
                at the engine-level.
            - minqlx.RET_STOP_ALL -- Stop handlers **and** the event.
            - Any other value -- Passed on to :func:`self.handle_return`, which will
                by default simply send a warning to the logger about an unknown value
                being returned. Can be overridden so that events can have their own
                special return values.

        :param args: Any arguments.
        :param kwargs: Any keyword arguments.

        """
        # Allow subclasses of this to edit the arguments without having
        # to reimplement this method. Whenever an unknown return value
        # is returned, we pass it on to handle_return.
        self.args = args
        self.kwargs = kwargs
        logger = minqlx.get_logger()
        # Log the events as they come in.
        if self.name not in self.no_debug:
            dbgstr = "{}{}".format(self.name, args)
            if len(dbgstr) > 100:
                dbgstr = dbgstr[0:99] + ")"
            logger.debug(dbgstr)

        plugins = self.plugins.copy()
        self.return_value = True
        for i in range(5):
            for plugin in plugins:
                for handler in plugins[plugin][i]:
                    try:
                        res = handler(*self.args, **self.kwargs)
                        if res == minqlx.RET_NONE or res is None:
                            continue
                        elif res == minqlx.RET_STOP:
                            return True
                        elif res == minqlx.RET_STOP_EVENT:
                            self.return_value = False
                        elif res == minqlx.RET_STOP_ALL:
                            return False
                        else: # Got an unknown return value.
                            return_handler = self.handle_return(handler, res)
                            if return_handler is not None:
                                return return_handler
                    except:
                        minqlx.log_exception(plugin)
                        continue

        return self.return_value

    def handle_return(self, handler, value):
        """Handle an unknown return value. If this returns anything but None,
        the it will stop execution of the event and pass the return value on
        to the C-level handlers. This method can be useful to override,
        because of the fact that you can edit the arguments that will be
        passed on to any handler after whatever handler returned *value*
        by editing *self.args*, *self.kwargs*. Furthermore, *self.return_value*
        is the return value that will be sent to the C-level handler if the
        event isn't stopped later along the road.
        """
        logger = minqlx.get_logger()
        logger.warning("Handler '{}' returned unknown value '{}' for event '{}'"
            .format(handler.__name__, value, self.name))

    def add_hook(self, plugin, handler, priority=minqlx.PRI_NORMAL):
        """Hook the event, making the handler get called with relevant arguments
        whenever the event is takes place.

        :param plugin: The plugin that's hooking the event.
        :type plugin: minqlx.Plugin
        :param handler: The handler to be called when the event takes place.
        :type handler: callable
        :param priority: The priority of the hook. Determines the order the handlers are called in.
        :type priority: minqlx.PRI_LOWEST, minqlx.PRI_LOW, minqlx.PRI_NORMAL, minqlx.PRI_HIGH or minqlx.PRI_HIGHEST
        :raises: ValueError

        """
        if not (minqlx.PRI_HIGHEST <= priority <= minqlx.PRI_LOWEST):
            raise ValueError("'{}' is an invalid priority level.".format(priority))

        if self.need_zmq_stats_enabled and not bool(int(minqlx.get_cvar("zmq_stats_enable"))):
            raise AssertionError("{} hook requires zmq_stats_enabled cvar to have nonzero value".format(self.name))

        if plugin not in self.plugins:
            # Initialize tuple.
            self.plugins[plugin] = ([], [], [], [], []) # 5 priority levels.
        else:
            # Check if we've already registered this handler.
            for i in range(len(self.plugins[plugin])):
                for hook in self.plugins[plugin][i]:
                    if handler == hook:
                        raise ValueError("The event has already been hooked with the same handler and priority.")

        self.plugins[plugin][priority].append(handler)

    def remove_hook(self, plugin, handler, priority=minqlx.PRI_NORMAL):
        """Removes a previously hooked event.

        :param plugin: The plugin that hooked the event.
        :type plugin: minqlx.Plugin
        :param handler: The handler used when hooked.
        :type handler: callable
        :param priority: The priority of the hook when hooked.
        :type priority: minqlx.PRI_LOWEST, minqlx.PRI_LOW, minqlx.PRI_NORMAL, minqlx.PRI_HIGH or minqlx.PRI_HIGHEST
        :raises: ValueError

        """
        for hook in self.plugins[plugin][priority]:
            if handler == hook:
                self.plugins[plugin][priority].remove(handler)
                return

        raise ValueError("The event has not been hooked with the handler provided")

class EventDispatcherManager:
    """Holds all the event dispatchers and provides a way to access the dispatcher
    instances by accessing it like a dictionary using the event name as a key.
    Only one dispatcher can be used per event.

    """
    def __init__(self):
        self._dispatchers = {}

    def __getitem__(self, key):
        return self._dispatchers[key]

    def __contains__(self, key):
        return key in self._dispatchers

    def add_dispatcher(self, dispatcher):
        if dispatcher.name in self:
            raise ValueError("Event name already taken.")
        elif not issubclass(dispatcher, EventDispatcher):
            raise ValueError("Cannot add an event dispatcher not based on EventDispatcher.")

        self._dispatchers[dispatcher.name] = dispatcher()

    def remove_dispatcher(self, dispatcher):
        if dispatcher.name not in self:
            raise ValueError("Event name not found.")

        del self._dispatchers[dispatcher.name]

    def remove_dispatcher_by_name(self, event_name):
        if event_name not in self:
            raise ValueError("Event name not found.")

        del self._dispatchers[event_name]

# ====================================================================
#                          EVENT DISPATCHERS
# ====================================================================

class ConsolePrintDispatcher(EventDispatcher):
    """Event that goes off whenever the console prints something, including
    those with :func:`minqlx.console_print`.

    """
    name = "console_print"

    def dispatch(self, text):
        return super().dispatch(text)

    def handle_return(self, handler, value):
        """If a string was returned, continue execution, but we edit the
        string that's being printed along the chain of handlers.

        """
        if isinstance(value, str):
            self.args = (value,)
            self.return_value = value
        else:
            return super().handle_return(handler, value)

class CommandDispatcher(EventDispatcher):
    """Event that goes off when a command is executed. This can be used
    to for instance keep a log of all the commands admins have used.

    """
    name = "command"

    def dispatch(self, caller, command, args):
        super().dispatch(caller, command, args)

class ClientCommandDispatcher(EventDispatcher):
    """Event that triggers with any client command. This overlaps with
    other events, such as "chat".

    """
    name = "client_command"

    def dispatch(self, player, cmd):
        ret = super().dispatch(player, cmd)
        if ret is False:
            return False

        ret = minqlx.COMMANDS.handle_input(player, cmd, minqlx.ClientCommandChannel(player))
        if ret is False:
            return False

        return self.return_value

    def handle_return(self, handler, value):
        """If a string was returned, continue execution, but we edit the
        command that's being executed along the chain of handlers.

        """
        if isinstance(value, str):
            player, cmd = self.args
            self.args = (player, value)
            self.return_value = value
        else:
            return super().handle_return(handler, value)

class ServerCommandDispatcher(EventDispatcher):
    """Event that triggers with any server command sent by the server,
    including :func:`minqlx.send_server_command`. Can be cancelled.

    """
    name = "server_command"

    def dispatch(self, player, cmd):
        return super().dispatch(player, cmd)

    def handle_return(self, handler, value):
        """If a string was returned, continue execution, but we edit the
        command that's being sent along the chain of handlers.

        """
        if isinstance(value, str):
            player, cmd = self.args
            self.args = (player, value)
            self.return_value = value
        else:
            return super().handle_return(handler, value)

class FrameEventDispatcher(EventDispatcher):
    """Event that triggers every frame. Cannot be cancelled.

    """
    name = "frame"

    def dispatch(self):
        return super().dispatch()

class SetConfigstringDispatcher(EventDispatcher):
    """Event that triggers when the server tries to set a configstring. You can
    stop this event and use :func:`minqlx.set_configstring` to modify it, but a
    more elegant way to do it is simply returning the new configstring in
    the handler, and the modified one will go down the plugin chain instead.

    """
    name = "set_configstring"

    def dispatch(self, index, value):
        return super().dispatch(index, value)

    def handle_return(self, handler, value):
        """If a string was returned, continue execution, but we edit the
        configstring to the returned string. This allows multiple handlers
        to edit the configstring along the way before it's actually
        set by the QL engine.

        """
        if isinstance(value, str):
            index, old_value = self.args
            self.args = (index, value)
            self.return_value = value
        else:
            return super().handle_return(handler, value)

class ChatEventDispatcher(EventDispatcher):
    """Event that triggers with the "say" command. If the handler cancels it,
    the message will also be cancelled.

    """
    name = "chat"

    def dispatch(self, player, msg, channel):
        ret = minqlx.COMMANDS.handle_input(player, msg, channel)
        if ret is False: # Stop event if told to.
            return False

        return super().dispatch(player, msg, channel)


class UnloadDispatcher(EventDispatcher):
    """Event that triggers whenever a plugin is unloaded. Cannot be cancelled."""
    name = "unload"

    def dispatch(self, plugin):
        super().dispatch(plugin)

class PlayerConnectDispatcher(EventDispatcher):
    """Event that triggers whenever a player tries to connect. If the event
    is not stopped, it will let the player connect as usual. If it is stopped
    it will either display a generic ban message, or whatever string is returned
    by the handler.

    """
    name = "player_connect"

    def dispatch(self, player):
        return super().dispatch(player)

    def handle_return(self, handler, value):
        """If a string was returned, stop execution of event, disallow
        the player from connecting, and display the returned string as
        a message to the player trying to connect.

        """
        if isinstance(value, str):
            return value
        else:
            return super().handle_return(handler, value)

class PlayerLoadedDispatcher(EventDispatcher):
    """Event that triggers whenever a player connects *and* finishes loading.
    This means it'll trigger later than the "X connected" messages in-game,
    and it will also trigger when a map changes and players finish loading it.

    """
    name = "player_loaded"

    def dispatch(self, player):
        return super().dispatch(player)

class PlayerDisonnectDispatcher(EventDispatcher):
    """Event that triggers whenever a player disconnects. Cannot be cancelled."""
    name = "player_disconnect"

    def dispatch(self, player, reason):
        return super().dispatch(player, reason)

class PlayerSpawnDispatcher(EventDispatcher):
    """Event that triggers when a player spawns. Cannot be cancelled."""
    name = "player_spawn"

    def dispatch(self, player):
        return super().dispatch(player)

class StatsDispatcher(EventDispatcher):
    """Event that triggers whenever the server sends stats over ZMQ."""
    name = "stats"
    need_zmq_stats_enabled = True

    def dispatch(self, stats):
        return super().dispatch(stats)

class VoteCalledDispatcher(EventDispatcher):
    """Event that goes off whenever a player tries to call a vote. Note that
    this goes off even if it's a vote command that is invalid. Use vote_started
    if you only need votes that actually go through. Use this one for custom votes.

    """
    name = "vote_called"

    def dispatch(self, player, vote, args):
        return super().dispatch(player, vote, args)

class VoteStartedDispatcher(EventDispatcher):
    """Event that goes off whenever a vote starts. A vote started with Plugin.callvote()
    will have the caller set to None.

    """
    name = "vote_started"

    def __init__(self):
        super().__init__()
        self._caller = None

    def dispatch(self, vote, args):
        return super().dispatch(self._caller, vote, args)

    def caller(self, player):
        self._caller = player

class VoteEndedDispatcher(EventDispatcher):
    """Event that goes off whenever a vote either passes or fails."""
    name = "vote_ended"

    def dispatch(self, passed):
        # Check if there's a current vote in the first place.
        cs = minqlx.get_configstring(9)
        if not cs:
            minqlx.get_logger().warning("vote_ended went off without configstring 9.")
            return

        res = _re_vote.match(cs)
        vote = res.group("cmd")
        args = res.group("args") if res.group("args") else ""
        votes = (int(minqlx.get_configstring(10)), int(minqlx.get_configstring(11)))
        super().dispatch(votes, vote, args, passed)

class VoteDispatcher(EventDispatcher):
    """Event that goes off whenever someone tries to vote either yes or no."""
    name = "vote"

    def dispatch(self, player, yes):
        return super().dispatch(player, yes)

class GameCountdownDispatcher(EventDispatcher):
    """Event that goes off when the countdown before a game starts."""
    name = "game_countdown"

    def dispatch(self):
        return super().dispatch()

class GameStartDispatcher(EventDispatcher):
    """Event that goes off when a game starts."""
    name = "game_start"
    need_zmq_stats_enabled = True

    def dispatch(self, data):
        return super().dispatch(data)

class GameEndDispatcher(EventDispatcher):
    """Event that goes off when a game ends."""
    name = "game_end"
    need_zmq_stats_enabled = True

    def dispatch(self, data):
        return super().dispatch(data)

class RoundCountdownDispatcher(EventDispatcher):
    """Event that goes off when the countdown before a round starts."""
    name = "round_countdown"

    def dispatch(self, round_number):
        return super().dispatch(round_number)

class RoundStartDispatcher(EventDispatcher):
    """Event that goes off when a round starts."""
    name = "round_start"

    def dispatch(self, round_number):
        return super().dispatch(round_number)

class RoundEndDispatcher(EventDispatcher):
    """Event that goes off when a round ends."""
    name = "round_end"
    need_zmq_stats_enabled = True

    def dispatch(self, data):
        return super().dispatch(data)

class TeamSwitchDispatcher(EventDispatcher):
    """For when a player switches teams. If cancelled,
    simply put the player back in the old team.

    If possible, consider using team_switch_attempt for a cleaner
    solution if you need to cancel the event."""
    name = "team_switch"
    need_zmq_stats_enabled = True

    def dispatch(self, player, old_team, new_team):
        return super().dispatch(player, old_team, new_team)

class TeamSwitchAttemptDispatcher(EventDispatcher):
    """For when a player attempts to join a team. Prevents the player from doing it when cancelled.

    When players click the Join Match button, it sends "team a" (with the "a" being "any",
    presumably), meaning the new_team argument can also be "any" in addition to all the
    other teams.

    """
    name = "team_switch_attempt"

    def dispatch(self, player, old_team, new_team):
        return super().dispatch(player, old_team, new_team)

class MapDispatcher(EventDispatcher):
    """Event that goes off when a map is loaded, even if the same map is loaded again."""
    name = "map"

    def dispatch(self, mapname, factory):
        return super().dispatch(mapname, factory)

class NewGameDispatcher(EventDispatcher):
    """Event that goes off when the game module is initialized. This happens when new maps are loaded,
    a game is aborted, a game ends but stays on the same map, or when the game itself starts.

    """
    name = "new_game"

    def dispatch(self):
        return super().dispatch()

class KillDispatcher(EventDispatcher):
    """Event that goes off when someone is killed."""
    name = "kill"
    need_zmq_stats_enabled = True

    def dispatch(self, victim, killer, data):
        return super().dispatch(victim, killer, data)

class DeathDispatcher(EventDispatcher):
    """Event that goes off when someone dies."""
    name = "death"
    need_zmq_stats_enabled = True

    def dispatch(self, victim, killer, data):
        return super().dispatch(victim, killer, data)

class UserinfoDispatcher(EventDispatcher):
    """Event for clients changing their userinfo."""
    name = "userinfo"

    def dispatch(self, player, changed):
        return super().dispatch(player, changed)

    def handle_return(self, handler, value):
        """Takes a returned dictionary and applies it to the current userinfo."""
        if isinstance(value, dict):
            player, changed = self.args
            self.args = (player, changed)
            self.return_value = changed
        else:
            return super().handle_return(handler, value)

class KamikazeUseDispatcher(EventDispatcher):
    """Event that goes off when player uses kamikaze item."""
    name = "kamikaze_use"

    def dispatch(self, player):
        return super().dispatch(player)

class KamikazeExplodeDispatcher(EventDispatcher):
    """Event that goes off when kamikaze explodes."""
    name = "kamikaze_explode"

    def dispatch(self, player, is_used_on_demand):
        return super().dispatch(player, is_used_on_demand)

class DamageDispatcher(EventDispatcher):
    """Event that goes off when someone is inflicted with damage."""

    name = "damage"
    need_zmq_stats_enabled = False

    def dispatch(self, target, attacker, damage, dflags, mod):
        return super().dispatch(target, attacker, damage, dflags, mod)

EVENT_DISPATCHERS = EventDispatcherManager()
EVENT_DISPATCHERS.add_dispatcher(ConsolePrintDispatcher)
EVENT_DISPATCHERS.add_dispatcher(CommandDispatcher)
EVENT_DISPATCHERS.add_dispatcher(ClientCommandDispatcher)
EVENT_DISPATCHERS.add_dispatcher(ServerCommandDispatcher)
EVENT_DISPATCHERS.add_dispatcher(FrameEventDispatcher)
EVENT_DISPATCHERS.add_dispatcher(SetConfigstringDispatcher)
EVENT_DISPATCHERS.add_dispatcher(ChatEventDispatcher)
EVENT_DISPATCHERS.add_dispatcher(UnloadDispatcher)
EVENT_DISPATCHERS.add_dispatcher(PlayerConnectDispatcher)
EVENT_DISPATCHERS.add_dispatcher(PlayerLoadedDispatcher)
EVENT_DISPATCHERS.add_dispatcher(PlayerDisonnectDispatcher)
EVENT_DISPATCHERS.add_dispatcher(PlayerSpawnDispatcher)
EVENT_DISPATCHERS.add_dispatcher(KamikazeUseDispatcher)
EVENT_DISPATCHERS.add_dispatcher(KamikazeExplodeDispatcher)
EVENT_DISPATCHERS.add_dispatcher(StatsDispatcher)
EVENT_DISPATCHERS.add_dispatcher(VoteCalledDispatcher)
EVENT_DISPATCHERS.add_dispatcher(VoteStartedDispatcher)
EVENT_DISPATCHERS.add_dispatcher(VoteEndedDispatcher)
EVENT_DISPATCHERS.add_dispatcher(VoteDispatcher)
EVENT_DISPATCHERS.add_dispatcher(GameCountdownDispatcher)
EVENT_DISPATCHERS.add_dispatcher(GameStartDispatcher)
EVENT_DISPATCHERS.add_dispatcher(GameEndDispatcher)
EVENT_DISPATCHERS.add_dispatcher(RoundCountdownDispatcher)
EVENT_DISPATCHERS.add_dispatcher(RoundStartDispatcher)
EVENT_DISPATCHERS.add_dispatcher(RoundEndDispatcher)
EVENT_DISPATCHERS.add_dispatcher(TeamSwitchDispatcher)
EVENT_DISPATCHERS.add_dispatcher(TeamSwitchAttemptDispatcher)
EVENT_DISPATCHERS.add_dispatcher(MapDispatcher)
EVENT_DISPATCHERS.add_dispatcher(NewGameDispatcher)
EVENT_DISPATCHERS.add_dispatcher(KillDispatcher)
EVENT_DISPATCHERS.add_dispatcher(DeathDispatcher)
EVENT_DISPATCHERS.add_dispatcher(UserinfoDispatcher)
EVENT_DISPATCHERS.add_dispatcher(DamageDispatcher)
