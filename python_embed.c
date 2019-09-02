#include <Python.h>
#include <structmember.h>
#include <structseq.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

#include "pyminqlx.h"
#include "quake_common.h"
#include "patterns.h"
#include "common.h"

PyObject* client_command_handler = NULL;
PyObject* server_command_handler = NULL;
PyObject* client_connect_handler = NULL;
PyObject* client_loaded_handler = NULL;
PyObject* client_disconnect_handler = NULL;
PyObject* frame_handler = NULL;
PyObject* custom_command_handler = NULL;
PyObject* new_game_handler = NULL;
PyObject* set_configstring_handler = NULL;
PyObject* rcon_handler = NULL;
PyObject* console_print_handler = NULL;
PyObject* client_spawn_handler = NULL;

PyObject* kamikaze_use_handler = NULL;
PyObject* kamikaze_explode_handler = NULL;

static PyThreadState* mainstate;
static int initialized = 0;

/*
 * If we don't do this, we'll simply get NULL from both PyRun_File*()
 * and PyRun_String*() if the module has an error in it. It's ugly as
 * fuck, but other than doing this, I have no idea how to extract the
 * traceback. The documentation or Google doesn't help much either.
*/
static const char loader[] = "import traceback\n" \
    "try:\n" \
    "  import sys\n" \
	"  sys.path.append('" CORE_MODULE "')\n" \
    "  sys.path.append('.')\n" \
	"  import minqlx\n" \
	"  minqlx.initialize()\n" \
    "  ret = True\n" \
    "except Exception as e:\n" \
    "  e = traceback.format_exc().rstrip('\\n')\n" \
    "  for line in e.split('\\n'): print(line)\n" \
    "  ret = False\n";

/*
 * The number of handlers was getting large, so instead of a bunch of
 * else ifs in register_handler, I'm using a struct to hold name-handler
 * pairs and iterate over them instead.
 */
static handler_t handlers[] = {
		{"client_command", 		&client_command_handler},
		{"server_command", 		&server_command_handler},
		{"frame", 				&frame_handler},
		{"player_connect", 		&client_connect_handler},
		{"player_loaded", 		&client_loaded_handler},
		{"player_disconnect", 	&client_disconnect_handler},
		{"custom_command", 		&custom_command_handler},
		{"new_game",			&new_game_handler},
		{"set_configstring", 	&set_configstring_handler},
        {"rcon",                &rcon_handler},
        {"console_print",       &console_print_handler},
        {"player_spawn",        &client_spawn_handler},

        {"kamikaze_use",        &kamikaze_use_handler},
        {"kamikaze_explode",    &kamikaze_explode_handler},

		{NULL, NULL}
};

/*
 * ================================================================
 *                      Struct Sequences
 * ================================================================
*/

// Players
static PyTypeObject player_info_type = {0};

static PyStructSequence_Field player_info_fields[] = {
    {"client_id", "The player's client ID."},
    {"name", "The player's name."},
    {"connection_state", "The player's connection state."},
    {"userinfo", "The player's userinfo."},
    {"steam_id", "The player's 64-bit representation of the Steam ID."},
    {"team", "The player's team."},
    {"privileges", "The player's privileges."},
    {NULL}
};

static PyStructSequence_Desc player_info_desc = {
    "PlayerInfo",
    "Information about a player, such as Steam ID, name, client ID, and whatnot.",
    player_info_fields,
    (sizeof(player_info_fields)/sizeof(PyStructSequence_Field)) - 1
};

// Player state
static PyTypeObject player_state_type = {0};

static PyStructSequence_Field player_state_fields[] = {
    {"is_alive", "Whether the player's alive or not."},
    {"position", "The player's position."},
    {"velocity", "The player's velocity."},
    {"health", "The player's health."},
    {"armor", "The player's armor."},
    {"noclip", "Whether the player has noclip or not."},
    {"weapon", "The weapon the player is currently using."},
    {"weapons", "The player's weapons."},
    {"ammo", "The player's weapon ammo."},
    {"powerups", "The player's powerups."},
    {"holdable", "The player's holdable item."},
    {"flight", "A struct sequence with flight parameters."},
    {"is_frozen", "Whether the player is frozen(freezetag)."},
    {NULL}
};

static PyStructSequence_Desc player_state_desc = {
    "PlayerState",
    "Information about a player's state in the game.",
    player_state_fields,
    (sizeof(player_state_fields)/sizeof(PyStructSequence_Field)) - 1
};

// Stats
static PyTypeObject player_stats_type = {0};

static PyStructSequence_Field player_stats_fields[] = {
    {"score", "The player's primary score."},
    {"kills", "The player's number of kills."},
    {"deaths", "The player's number of deaths."},
    {"damage_dealt", "The player's total damage dealt."},
    {"damage_taken", "The player's total damage taken."},
    {"time", "The time in milliseconds the player has on a team since the game started."},
    {"ping", "The player's ping."},
    {NULL}
};

static PyStructSequence_Desc player_stats_desc = {
    "PlayerStats",
    "A player's score and some basic stats.",
    player_stats_fields,
    (sizeof(player_stats_fields)/sizeof(PyStructSequence_Field)) - 1
};

// Vectors
static PyTypeObject vector3_type = {0};

static PyStructSequence_Field vector3_fields[] = {
    {"x", NULL},
    {"y", NULL},
    {"z", NULL},
    {NULL}
};

static PyStructSequence_Desc vector3_desc = {
    "Vector3",
    "A three-dimensional vector.",
    vector3_fields,
    (sizeof(vector3_fields)/sizeof(PyStructSequence_Field)) - 1
};

// Weapons
static PyTypeObject weapons_type = {0};

static PyStructSequence_Field weapons_fields[] = {
    {"g", NULL}, {"mg", NULL}, {"sg", NULL},
    {"gl", NULL}, {"rl", NULL}, {"lg", NULL},
    {"rg", NULL}, {"pg", NULL}, {"bfg", NULL},
    {"gh", NULL}, {"ng", NULL}, {"pl", NULL},
    {"cg", NULL}, {"hmg", NULL}, {"hands", NULL},
    {NULL}
};

static PyStructSequence_Desc weapons_desc = {
    "Weapons",
    "A struct sequence containing all the weapons in the game.",
    weapons_fields,
    (sizeof(weapons_fields)/sizeof(PyStructSequence_Field)) - 1
};

// Powerups
static PyTypeObject powerups_type = {0};

static PyStructSequence_Field powerups_fields[] = {
    {"quad", NULL}, {"battlesuit", NULL},
    {"haste", NULL}, {"invisibility", NULL},
    {"regeneration", NULL}, {"invulnerability", NULL},
    {NULL}
};

static PyStructSequence_Desc powerups_desc = {
    "Powerups",
    "A struct sequence containing all the powerups in the game.",
    powerups_fields,
    (sizeof(powerups_fields)/sizeof(PyStructSequence_Field)) - 1
};

// Flight
static PyTypeObject flight_type = {0};

static PyStructSequence_Field flight_fields[] = {
    {"fuel", NULL},
    {"max_fuel", NULL},
    {"thrust", NULL},
    {"refuel", NULL},
    {NULL}
};

static PyStructSequence_Desc flight_desc = {
    "Flight",
    "A struct sequence containing parameters for the flight holdable item.",
    flight_fields,
    (sizeof(flight_fields)/sizeof(PyStructSequence_Field)) - 1
};

/*
 * ================================================================
 *                    player_info/players_info
 * ================================================================
*/

static PyObject* makePlayerTuple(int client_id) {
    PyObject *name, *team, *priv;
    PyObject* cid = PyLong_FromLongLong(client_id);

    if (g_entities[client_id].client != NULL) {
        if (g_entities[client_id].client->pers.connected == CON_DISCONNECTED)
            name = PyUnicode_FromString("");
        else
            name = PyUnicode_DecodeUTF8(g_entities[client_id].client->pers.netname,
                strlen(g_entities[client_id].client->pers.netname), "ignore");

        if (g_entities[client_id].client->pers.connected == CON_DISCONNECTED)
            team = PyLong_FromLongLong(TEAM_SPECTATOR); // Set team to spectator if not yet connected.
        else
            team = PyLong_FromLongLong(g_entities[client_id].client->sess.sessionTeam);

        priv = PyLong_FromLongLong(g_entities[client_id].client->sess.privileges);
    }
    else {
        name = PyUnicode_FromString("");
        team = PyLong_FromLongLong(TEAM_SPECTATOR);
        priv = PyLong_FromLongLong(-1);
    }

    PyObject* state = PyLong_FromLongLong(svs->clients[client_id].state);
    PyObject* userinfo = PyUnicode_DecodeUTF8(svs->clients[client_id].userinfo, strlen(svs->clients[client_id].userinfo), "ignore");
    PyObject* steam_id = PyLong_FromLongLong(svs->clients[client_id].steam_id);

    PyObject* info = PyStructSequence_New(&player_info_type);
    PyStructSequence_SetItem(info, 0, cid);
    PyStructSequence_SetItem(info, 1, name);
    PyStructSequence_SetItem(info, 2, state);
    PyStructSequence_SetItem(info, 3, userinfo);
    PyStructSequence_SetItem(info, 4, steam_id);
    PyStructSequence_SetItem(info, 5, team);
    PyStructSequence_SetItem(info, 6, priv);

    return info;
}

static PyObject* PyMinqlx_PlayerInfo(PyObject* self, PyObject* args) {
    int i;
    if (!PyArg_ParseTuple(args, "i:player", &i))
        return NULL;

    if (i < 0 || i >= sv_maxclients->integer) {
        PyErr_Format(PyExc_ValueError,
                     "client_id needs to be a number from 0 to %d.",
                     sv_maxclients->integer);
        return NULL;

    }
    else if (allow_free_client != i && svs->clients[i].state == CS_FREE) {
        #ifndef NDEBUG
        DebugPrint("WARNING: PyMinqlx_PlayerInfo called for CS_FREE client %d.\n", i);
        #endif
        Py_RETURN_NONE;
    }

    return makePlayerTuple(i);
}

static PyObject* PyMinqlx_PlayersInfo(PyObject* self, PyObject* args) {
	PyObject* ret = PyList_New(sv_maxclients->integer);

	for (int i = 0; i < sv_maxclients->integer; i++) {
		if (svs->clients[i].state == CS_FREE) {
			if (PyList_SetItem(ret, i, Py_None) == -1)
                        return NULL;
            Py_INCREF(Py_None);
            continue;
		}

		if (PyList_SetItem(ret, i, makePlayerTuple(i)) == -1)
			return NULL;
	}

    return ret;
}

/*
 * ================================================================
 *                          get_userinfo
 * ================================================================
*/

static PyObject* PyMinqlx_GetUserinfo(PyObject* self, PyObject* args) {
    int i;
    if (!PyArg_ParseTuple(args, "i:get_userinfo", &i))
        return NULL;

    if (i < 0 || i >= sv_maxclients->integer) {
        PyErr_Format(PyExc_ValueError,
                     "client_id needs to be a number from 0 to %d.",
                     sv_maxclients->integer);
        return NULL;

    }
    else if (allow_free_client != i && svs->clients[i].state == CS_FREE)
        Py_RETURN_NONE;

    return PyUnicode_DecodeUTF8(svs->clients[i].userinfo, strlen(svs->clients[i].userinfo), "ignore");
}

/*
 * ================================================================
 *                       send_server_command
 * ================================================================
*/

static PyObject* PyMinqlx_SendServerCommand(PyObject* self, PyObject* args) {
    PyObject* client_id;
    int i;
    char* cmd;
    if (!PyArg_ParseTuple(args, "Os:send_server_command", &client_id, &cmd))
        return NULL;

    if (client_id == Py_None) {
        My_SV_SendServerCommand(NULL, "%s\n", cmd); // Send to all.
        Py_RETURN_TRUE;
    }
    else if (PyLong_Check(client_id)) {
        i = PyLong_AsLong(client_id);
        if (i >= 0 && i < sv_maxclients->integer) {
            if (svs->clients[i].state != CS_ACTIVE)
                Py_RETURN_FALSE;
            else {
                My_SV_SendServerCommand(&svs->clients[i], "%s\n", cmd);
                Py_RETURN_TRUE;
            }
        }
    }

    PyErr_Format(PyExc_ValueError,
                 "client_id needs to be a number from 0 to %d, or None.",
                 sv_maxclients->integer);
    return NULL;
}

/*
 * ================================================================
 *                          client_command
 * ================================================================
*/

static PyObject* PyMinqlx_ClientCommand(PyObject* self, PyObject* args) {
    int i;
    char* cmd;
    if (!PyArg_ParseTuple(args, "is:client_command", &i, &cmd))
        return NULL;

	if (i >= 0 && i < sv_maxclients->integer) {
		if (svs->clients[i].state == CS_FREE || svs->clients[i].state == CS_ZOMBIE)
			Py_RETURN_FALSE;
		else {
			My_SV_ExecuteClientCommand(&svs->clients[i], cmd, qtrue);
			Py_RETURN_TRUE;
		}
	}

    PyErr_Format(PyExc_ValueError,
                 "client_id needs to be a number from 0 to %d, or None.",
                 sv_maxclients->integer);
    return NULL;
}

/*
 * ================================================================
 *                         console_command
 * ================================================================
*/

static PyObject* PyMinqlx_ConsoleCommand(PyObject* self, PyObject* args) {
    char* cmd;
    if (!PyArg_ParseTuple(args, "s:console_command", &cmd))
        return NULL;

    Cmd_ExecuteString(cmd);

    Py_RETURN_NONE;
}

/*
 * ================================================================
 *                           get_cvar
 * ================================================================
*/

static PyObject* PyMinqlx_GetCvar(PyObject* self, PyObject* args) {
    char* name;
    if (!PyArg_ParseTuple(args, "s:get_cvar", &name))
        return NULL;

    cvar_t* cvar = Cvar_FindVar(name);
    if (cvar) {
    	return PyUnicode_FromString(cvar->string);
    }

    Py_RETURN_NONE;
}

/*
 * ================================================================
 *                           set_cvar
 * ================================================================
*/

static PyObject* PyMinqlx_SetCvar(PyObject* self, PyObject* args) {
    char *name, *value;
    int flags = 0;
    if (!PyArg_ParseTuple(args, "ss|i:set_cvar", &name, &value, &flags))
        return NULL;

    cvar_t* var = Cvar_FindVar(name);
    if (!var) {
        Cvar_Get(name, value, flags);
        Py_RETURN_TRUE;
    }

    if (flags == -1)
        Cvar_Set2(name, value, qtrue);
    else
        Cvar_Set2(name, value, qfalse);

    Py_RETURN_FALSE;
}

/*
 * ================================================================
 *                           set_cvar_limit
 * ================================================================
*/

static PyObject* PyMinqlx_SetCvarLimit(PyObject* self, PyObject* args) {
    char *name, *value, *min, *max;
    int flags = 0;
    if (!PyArg_ParseTuple(args, "ssss|i:set_cvar_limit", &name, &value, &min, &max, &flags))
        return NULL;

    Cvar_GetLimit(name, value, min, max, flags);
    Py_RETURN_NONE;
}

/*
 * ================================================================
 *                             kick
 * ================================================================
*/

static PyObject* PyMinqlx_Kick(PyObject* self, PyObject* args) {
    int i;
    PyObject* reason;
    if (!PyArg_ParseTuple(args, "iO:kick", &i, &reason))
        return NULL;

	if (i >= 0 && i < sv_maxclients->integer) {
		if (svs->clients[i].state != CS_ACTIVE) {
			PyErr_Format(PyExc_ValueError,
					"client_id must be None or the ID of an active player.");
			return NULL;
		}
		else if (reason == Py_None || (PyUnicode_Check(reason) && PyUnicode_AsUTF8(reason)[0] == 0)) {
			// Default kick message for None or empty strings.
			My_SV_DropClient(&svs->clients[i], "was kicked.");
		}
		else if (PyUnicode_Check(reason)) {
			My_SV_DropClient(&svs->clients[i], PyUnicode_AsUTF8(reason));
		}
	}
	else {
		PyErr_Format(PyExc_ValueError,
				"client_id needs to be a number from 0 to %d, or None.",
				sv_maxclients->integer);
		return NULL;
	}

    Py_RETURN_NONE;
}

/*
 * ================================================================
 *                          console_print
 * ================================================================
*/

static PyObject* PyMinqlx_ConsolePrint(PyObject* self, PyObject* args) {
    char* text;
    if (!PyArg_ParseTuple(args, "s:console_print", &text))
        return NULL;

    My_Com_Printf("%s\n", text);

    Py_RETURN_NONE;
}

/*
 * ================================================================
 *                          get_configstring
 * ================================================================
*/

static PyObject* PyMinqlx_GetConfigstring(PyObject* self, PyObject* args) {
    int i;
    char csbuffer[4096];
    if (!PyArg_ParseTuple(args, "i:get_configstring", &i))
        return NULL;
    else if (i < 0 || i > MAX_CONFIGSTRINGS) {
		PyErr_Format(PyExc_ValueError,
						 "index needs to be a number from 0 to %d.",
						 MAX_CONFIGSTRINGS);
		return NULL;
	}

    SV_GetConfigstring(i, csbuffer, sizeof(csbuffer));
    return PyUnicode_DecodeUTF8(csbuffer, strlen(csbuffer), "ignore");
}

/*
 * ================================================================
 *                          set_configstring
 * ================================================================
*/

static PyObject* PyMinqlx_SetConfigstring(PyObject* self, PyObject* args) {
    int i;
    char* cs;
    if (!PyArg_ParseTuple(args, "is:set_configstring", &i, &cs))
        return NULL;
    else if (i < 0 || i > MAX_CONFIGSTRINGS) {
    	PyErr_Format(PyExc_ValueError,
    	                 "index needs to be a number from 0 to %d.",
						 MAX_CONFIGSTRINGS);
		return NULL;
    }

    My_SV_SetConfigstring(i, cs);

    Py_RETURN_NONE;
}

/*
 * ================================================================
 *                          force_vote
 * ================================================================
*/

static PyObject* PyMinqlx_ForceVote(PyObject* self, PyObject* args) {
	int pass;
    if (!PyArg_ParseTuple(args, "p:force_vote", &pass))
        return NULL;

    if (!level->voteTime) {
    	// No active vote.
    	Py_RETURN_FALSE;
    }
    else if (pass && level->voteTime) {
    	// We tell the server every single client voted yes, making it pass in the next G_RunFrame.
		for (int i = 0; i < sv_maxclients->integer; i++) {
			if (svs->clients[i].state == CS_ACTIVE)
				g_entities[i].client->pers.voteState = VOTE_YES;
		}
    }
    else if (!pass && level->voteTime) {
    	// If we tell the server the vote is over, it'll fail it right away.
		level->voteTime -= 30000;
    }

    Py_RETURN_TRUE;
}

/*
 * ================================================================
 *                       add_console_command
 * ================================================================
*/

static PyObject* PyMinqlx_AddConsoleCommand(PyObject* self, PyObject* args) {
    char* cmd;
    if (!PyArg_ParseTuple(args, "s:add_console_command", &cmd))
        return NULL;

    Cmd_AddCommand(cmd, PyCommand);

    Py_RETURN_NONE;
}

/*
 * ================================================================
 *                         register_handler
 * ================================================================
*/

static PyObject* PyMinqlx_RegisterHandler(PyObject* self, PyObject* args) {
    char* event;
    PyObject* new_handler;

    if (!PyArg_ParseTuple(args, "sO:register_handler", &event, &new_handler)) {
    	return NULL;
    }
    else if (new_handler != Py_None && !PyCallable_Check(new_handler)) {
		PyErr_SetString(PyExc_TypeError, "The handler must be callable.");
		return NULL;
	}

	for (handler_t* h = handlers; h->name; h++) {
		if (!strcmp(h->name, event)) {
			Py_XDECREF(*h->handler);
			if (new_handler == Py_None)
				*h->handler = NULL;
			else {
				*h->handler = new_handler;
				Py_INCREF(new_handler);
			}

			Py_RETURN_NONE;
		}
	}

	PyErr_SetString(PyExc_ValueError, "Invalid event.");
	return NULL;
}

/*
 * ================================================================
 *                          player_state
 * ================================================================
*/

static PyObject* PyMinqlx_PlayerState(PyObject* self, PyObject* args) {
    int client_id;

    if (!PyArg_ParseTuple(args, "i:player_state", &client_id))
        return NULL;
    else if (client_id < 0 || client_id >= sv_maxclients->integer) {
        PyErr_Format(PyExc_ValueError,
                     "client_id needs to be a number from 0 to %d.",
                     sv_maxclients->integer);
        return NULL;
    }
    else if (!g_entities[client_id].client)
        Py_RETURN_NONE;

    PyObject* state = PyStructSequence_New(&player_state_type);
    PyStructSequence_SetItem(state, 0, PyBool_FromLong(g_entities[client_id].client->ps.pm_type == 0));

    PyObject* pos = PyStructSequence_New(&vector3_type);
    PyStructSequence_SetItem(pos, 0,
        PyFloat_FromDouble(g_entities[client_id].client->ps.origin[0]));
    PyStructSequence_SetItem(pos, 1,
        PyFloat_FromDouble(g_entities[client_id].client->ps.origin[1]));
    PyStructSequence_SetItem(pos, 2,
        PyFloat_FromDouble(g_entities[client_id].client->ps.origin[2]));
    PyStructSequence_SetItem(state, 1, pos);

    PyObject* vel = PyStructSequence_New(&vector3_type);
    PyStructSequence_SetItem(vel, 0,
        PyFloat_FromDouble(g_entities[client_id].client->ps.velocity[0]));
    PyStructSequence_SetItem(vel, 1,
        PyFloat_FromDouble(g_entities[client_id].client->ps.velocity[1]));
    PyStructSequence_SetItem(vel, 2,
        PyFloat_FromDouble(g_entities[client_id].client->ps.velocity[2]));
    PyStructSequence_SetItem(state, 2, vel);

    PyStructSequence_SetItem(state, 3, PyLong_FromLongLong(g_entities[client_id].health));
    PyStructSequence_SetItem(state, 4, PyLong_FromLongLong(g_entities[client_id].client->ps.stats[STAT_ARMOR]));
    PyStructSequence_SetItem(state, 5, PyBool_FromLong(g_entities[client_id].client->noclip));
    PyStructSequence_SetItem(state, 6, PyLong_FromLongLong(g_entities[client_id].client->ps.weapon));

    // Get weapons and ammo count.
    PyObject* weapons = PyStructSequence_New(&weapons_type);
    PyObject* ammo = PyStructSequence_New(&weapons_type);
    for (int i = 0; i < weapons_desc.n_in_sequence; i++) {
        PyStructSequence_SetItem(weapons, i, PyBool_FromLong(g_entities[client_id].client->ps.stats[STAT_WEAPONS] & (1 << (i+1))));
        PyStructSequence_SetItem(ammo, i, PyLong_FromLongLong(g_entities[client_id].client->ps.ammo[i+1]));
    }
    PyStructSequence_SetItem(state, 7, weapons);
    PyStructSequence_SetItem(state, 8, ammo);

    PyObject* powerups = PyStructSequence_New(&powerups_type);
    int index;
    for (int i = 0; i < powerups_desc.n_in_sequence; i++) {
        index = i+PW_QUAD;
        if (index == PW_FLIGHT) // Skip flight.
            index = PW_INVULNERABILITY;
        int remaining = g_entities[client_id].client->ps.powerups[index];
        if (remaining) // We don't want the time, but the remaining time.
            remaining -= level->time;
        PyStructSequence_SetItem(powerups, i, PyLong_FromLongLong(remaining));
    }
    PyStructSequence_SetItem(state, 9, powerups);

    PyObject* holdable;
    switch (g_entities[client_id].client->ps.stats[STAT_HOLDABLE_ITEM]) {
        case 0:
            holdable = Py_None;
            Py_INCREF(Py_None);
            break;
        case 27:
            holdable = PyUnicode_FromString("teleporter");
            break;
        case 28:
            holdable = PyUnicode_FromString("medkit");
            break;
        case 34:
            holdable = PyUnicode_FromString("flight");
            break;
        case 37:
            holdable = PyUnicode_FromString("kamikaze");
            break;
        case 38:
            holdable = PyUnicode_FromString("portal");
            break;
        case 39:
            holdable = PyUnicode_FromString("invulnerability");
            break;
        default:
            holdable = PyUnicode_FromString("unknown");
    }
    PyStructSequence_SetItem(state, 10, holdable);

    PyObject* flight = PyStructSequence_New(&flight_type);
    PyStructSequence_SetItem(flight, 0,
        PyLong_FromLongLong(g_entities[client_id].client->ps.stats[STAT_CUR_FLIGHT_FUEL]));
    PyStructSequence_SetItem(flight, 1,
        PyLong_FromLongLong(g_entities[client_id].client->ps.stats[STAT_MAX_FLIGHT_FUEL]));
    PyStructSequence_SetItem(flight, 2,
        PyLong_FromLongLong(g_entities[client_id].client->ps.stats[STAT_FLIGHT_THRUST]));
    PyStructSequence_SetItem(flight, 3,
        PyLong_FromLongLong(g_entities[client_id].client->ps.stats[STAT_FLIGHT_REFUEL]));
    PyStructSequence_SetItem(state, 11, flight);

    PyStructSequence_SetItem(state, 12, PyBool_FromLong(g_entities[client_id].client->ps.pm_type == 4));

    return state;
}

/*
 * ================================================================
 *                          player_stats
 * ================================================================
*/

static PyObject* PyMinqlx_PlayerStats(PyObject* self, PyObject* args) {
    int client_id;

    if (!PyArg_ParseTuple(args, "i:player_stats", &client_id))
        return NULL;
    else if (client_id < 0 || client_id >= sv_maxclients->integer) {
        PyErr_Format(PyExc_ValueError,
                     "client_id needs to be a number from 0 to %d.",
                     sv_maxclients->integer);
        return NULL;
    }
    else if (!g_entities[client_id].client)
        Py_RETURN_NONE;

    PyObject* stats = PyStructSequence_New(&player_stats_type);
    int score = g_entities[client_id].client->sess.sessionTeam == TEAM_SPECTATOR ?
        0 : g_entities[client_id].client->ps.persistant[PERS_ROUND_SCORE];
    PyStructSequence_SetItem(stats, 0, PyLong_FromLongLong(score));
    PyStructSequence_SetItem(stats, 1, PyLong_FromLongLong(g_entities[client_id].client->expandedStats.numKills));
    PyStructSequence_SetItem(stats, 2, PyLong_FromLongLong(g_entities[client_id].client->expandedStats.numDeaths));
    PyStructSequence_SetItem(stats, 3, PyLong_FromLongLong(g_entities[client_id].client->expandedStats.totalDamageDealt));
    PyStructSequence_SetItem(stats, 4, PyLong_FromLongLong(g_entities[client_id].client->expandedStats.totalDamageTaken));
    PyStructSequence_SetItem(stats, 5, PyLong_FromLongLong(level->time - g_entities[client_id].client->pers.enterTime));
    PyStructSequence_SetItem(stats, 6, PyLong_FromLongLong(g_entities[client_id].client->ps.ping));

    return stats;
}

/*
 * ================================================================
 *                          set_position
 * ================================================================
*/

static PyObject* PyMinqlx_SetPosition(PyObject* self, PyObject* args) {
    int client_id;
    PyObject* new_position;

    if (!PyArg_ParseTuple(args, "iO:set_position", &client_id, &new_position))
        return NULL;
    else if (client_id < 0 || client_id >= sv_maxclients->integer) {
        PyErr_Format(PyExc_ValueError,
                     "client_id needs to be a number from 0 to %d.",
                     sv_maxclients->integer);
        return NULL;
    }
    else if (!g_entities[client_id].client)
        Py_RETURN_FALSE;
    else if (!PyObject_TypeCheck(new_position, &vector3_type)) {
        PyErr_Format(PyExc_ValueError, "Argument must be of type minqlx.Vector3.");
        return NULL;
    }

    g_entities[client_id].client->ps.origin[0] =
        (float)PyFloat_AsDouble(PyStructSequence_GetItem(new_position, 0));
    g_entities[client_id].client->ps.origin[1] =
        (float)PyFloat_AsDouble(PyStructSequence_GetItem(new_position, 1));
    g_entities[client_id].client->ps.origin[2] =
        (float)PyFloat_AsDouble(PyStructSequence_GetItem(new_position, 2));

    Py_RETURN_TRUE;
}

/*
 * ================================================================
 *                          set_velocity
 * ================================================================
*/

static PyObject* PyMinqlx_SetVelocity(PyObject* self, PyObject* args) {
    int client_id;
    PyObject* new_velocity;

    if (!PyArg_ParseTuple(args, "iO:set_velocity", &client_id, &new_velocity))
        return NULL;
    else if (client_id < 0 || client_id >= sv_maxclients->integer) {
        PyErr_Format(PyExc_ValueError,
                     "client_id needs to be a number from 0 to %d.",
                     sv_maxclients->integer);
        return NULL;
    }
    else if (!g_entities[client_id].client)
        Py_RETURN_FALSE;
    else if (!PyObject_TypeCheck(new_velocity, &vector3_type)) {
        PyErr_Format(PyExc_ValueError, "Argument must be of type minqlx.Vector3.");
        return NULL;
    }

    g_entities[client_id].client->ps.velocity[0] =
        (float)PyFloat_AsDouble(PyStructSequence_GetItem(new_velocity, 0));
    g_entities[client_id].client->ps.velocity[1] =
        (float)PyFloat_AsDouble(PyStructSequence_GetItem(new_velocity, 1));
    g_entities[client_id].client->ps.velocity[2] =
        (float)PyFloat_AsDouble(PyStructSequence_GetItem(new_velocity, 2));

    Py_RETURN_TRUE;
}

/*
* ================================================================
*                             noclip
* ================================================================
*/

static PyObject* PyMinqlx_NoClip(PyObject* self, PyObject* args) {
    int client_id, activate;
    if (!PyArg_ParseTuple(args, "ip:noclip", &client_id, &activate))
        return NULL;
    else if (client_id < 0 || client_id >= sv_maxclients->integer) {
        PyErr_Format(PyExc_ValueError,
                     "client_id needs to be a number from 0 to %d.",
                     sv_maxclients->integer);
        return NULL;
    }
    else if (!g_entities[client_id].client)
        Py_RETURN_FALSE;

    if ((activate && g_entities[client_id].client->noclip) || (!activate && !g_entities[client_id].client->noclip)) {
        // Change was made.
        Py_RETURN_FALSE;
    }

    g_entities[client_id].client->noclip = activate ? qtrue : qfalse;
    Py_RETURN_TRUE;
}

/*
* ================================================================
*                           set_health
* ================================================================
*/

static PyObject* PyMinqlx_SetHealth(PyObject* self, PyObject* args) {
    int client_id, health;
    if (!PyArg_ParseTuple(args, "ii:set_health", &client_id, &health))
        return NULL;
    else if (client_id < 0 || client_id >= sv_maxclients->integer) {
        PyErr_Format(PyExc_ValueError,
                     "client_id needs to be a number from 0 to %d.",
                     sv_maxclients->integer);
        return NULL;
    }
    else if (!g_entities[client_id].client)
        Py_RETURN_FALSE;

    g_entities[client_id].health = health;
    Py_RETURN_TRUE;
}

/*
* ================================================================
*                           set_armor
* ================================================================
*/

static PyObject* PyMinqlx_SetArmor(PyObject* self, PyObject* args) {
    int client_id, armor;
    if (!PyArg_ParseTuple(args, "ii:set_armor", &client_id, &armor))
        return NULL;
    else if (client_id < 0 || client_id >= sv_maxclients->integer) {
        PyErr_Format(PyExc_ValueError,
                     "client_id needs to be a number from 0 to %d.",
                     sv_maxclients->integer);
        return NULL;
    }
    else if (!g_entities[client_id].client)
        Py_RETURN_FALSE;

    g_entities[client_id].client->ps.stats[STAT_ARMOR] = armor;
    Py_RETURN_TRUE;
}

/*
* ================================================================
*                           set_weapons
* ================================================================
*/

static PyObject* PyMinqlx_SetWeapons(PyObject* self, PyObject* args) {
    int client_id, weapon_flags = 0;
    PyObject* weapons;
    if (!PyArg_ParseTuple(args, "iO:set_weapons", &client_id, &weapons))
        return NULL;
    else if (client_id < 0 || client_id >= sv_maxclients->integer) {
        PyErr_Format(PyExc_ValueError,
                     "client_id needs to be a number from 0 to %d.",
                     sv_maxclients->integer);
        return NULL;
    }
    else if (!g_entities[client_id].client)
        Py_RETURN_FALSE;
    else if (!PyObject_TypeCheck(weapons, &weapons_type)) {
        PyErr_Format(PyExc_ValueError, "Argument must be of type minqlx.Weapons.");
        return NULL;
    }

    PyObject* w;
    for (int i = 0; i < weapons_desc.n_in_sequence; i++) {
        w = PyStructSequence_GetItem(weapons, i);
        if (!PyBool_Check(w)) {
            PyErr_Format(PyExc_ValueError, "Tuple argument %d is not a boolean.", i);
            return NULL;
        }

        weapon_flags |= w == Py_True ? (1 << (i+1)) : 0;
    }

    g_entities[client_id].client->ps.stats[STAT_WEAPONS] = weapon_flags;
    Py_RETURN_TRUE;
}

/*
* ================================================================
*                           set_weapon
* ================================================================
*/

static PyObject* PyMinqlx_SetWeapon(PyObject* self, PyObject* args) {
    int client_id, weapon;
    if (!PyArg_ParseTuple(args, "ii:set_weapon", &client_id, &weapon))
        return NULL;
    else if (client_id < 0 || client_id >= sv_maxclients->integer) {
        PyErr_Format(PyExc_ValueError,
                     "client_id needs to be a number from 0 to %d.",
                     sv_maxclients->integer);
        return NULL;
    }
    else if (!g_entities[client_id].client)
        Py_RETURN_FALSE;
    else if (weapon < 0 || weapon > 16) {
        PyErr_Format(PyExc_ValueError, "Weapon must be a number from 0 to 15.");
        return NULL;
    }

    g_entities[client_id].client->ps.weapon = weapon;
    Py_RETURN_TRUE;
}

/*
* ================================================================
*                           set_ammo
* ================================================================
*/

static PyObject* PyMinqlx_SetAmmo(PyObject* self, PyObject* args) {
    int client_id;
    PyObject* ammos;
    if (!PyArg_ParseTuple(args, "iO:set_ammo", &client_id, &ammos))
        return NULL;
    else if (client_id < 0 || client_id >= sv_maxclients->integer) {
        PyErr_Format(PyExc_ValueError,
                     "client_id needs to be a number from 0 to %d.",
                     sv_maxclients->integer);
        return NULL;
    }
    else if (!g_entities[client_id].client)
        Py_RETURN_FALSE;
    else if (!PyObject_TypeCheck(ammos, &weapons_type)) {
        PyErr_Format(PyExc_ValueError, "Argument must be of type minqlx.Weapons.");
        return NULL;
    }

    PyObject* a;
    for (int i = 0; i < weapons_desc.n_in_sequence; i++) {
        a = PyStructSequence_GetItem(ammos, i);
        if (!PyLong_Check(a)) {
            PyErr_Format(PyExc_ValueError, "Tuple argument %d is not an integer.", i);
            return NULL;
        }

        g_entities[client_id].client->ps.ammo[i+1] = PyLong_AsLong(a);
    }

    Py_RETURN_TRUE;
}

/*
* ================================================================
*                           set_powerups
* ================================================================
*/

static PyObject* PyMinqlx_SetPowerups(PyObject* self, PyObject* args) {
    int client_id, t;
    PyObject* powerups;
    if (!PyArg_ParseTuple(args, "iO:set_powerups", &client_id, &powerups))
        return NULL;
    else if (client_id < 0 || client_id >= sv_maxclients->integer) {
        PyErr_Format(PyExc_ValueError,
                     "client_id needs to be a number from 0 to %d.",
                     sv_maxclients->integer);
        return NULL;
    }
    else if (!g_entities[client_id].client)
        Py_RETURN_FALSE;
    else if (!PyObject_TypeCheck(powerups, &powerups_type)) {
        PyErr_Format(PyExc_ValueError, "Argument must be of type minqlx.Powerups.");
        return NULL;
    }

    PyObject* powerup;

    // Quad -> Invulnerability, but skip flight.
    for (int i = 0; i < powerups_desc.n_in_sequence; i++) {
        powerup = PyStructSequence_GetItem(powerups, i);
        if (!PyLong_Check(powerup)) {
            PyErr_Format(PyExc_ValueError, "Tuple argument %d is not an integer.", i);
            return NULL;
        }

        // If i == 5, it'll modify flight, which isn't a real powerup.
        // We bump it up and modify invulnerability instead.
        if (i+PW_QUAD == PW_FLIGHT)
            i = PW_INVULNERABILITY - PW_QUAD;

        t = PyLong_AsLong(powerup);
        if (!t) {
            g_entities[client_id].client->ps.powerups[i+PW_QUAD] = 0;
            continue;
        }

        g_entities[client_id].client->ps.powerups[i+PW_QUAD] = level->time - (level->time % 1000) + t;
    }

    Py_RETURN_TRUE;
}

/*
* ================================================================
*                           set_holdable
* ================================================================
*/

static PyObject* PyMinqlx_SetHoldable(PyObject* self, PyObject* args) {
    int client_id, i;
    if (!PyArg_ParseTuple(args, "ii:set_holdable", &client_id, &i))
        return NULL;
    else if (client_id < 0 || client_id >= sv_maxclients->integer) {
        PyErr_Format(PyExc_ValueError,
                     "client_id needs to be a number from 0 to %d.",
                     sv_maxclients->integer);
        return NULL;
    }
    else if (!g_entities[client_id].client)
        Py_RETURN_FALSE;

    if (i == 37)  // 37 - kamikaze
        g_entities[client_id].client->ps.eFlags |= EF_KAMIKAZE;
    else
        g_entities[client_id].client->ps.eFlags &= ~EF_KAMIKAZE;

    g_entities[client_id].client->ps.stats[STAT_HOLDABLE_ITEM] = i;
    Py_RETURN_TRUE;
}

/*
* ================================================================
*                          drop_holdable
* ================================================================
*/

void __cdecl Switch_Touch_Item(gentity_t *ent) {
    ent->touch = (void*)Touch_Item;
    ent->think = G_FreeEntity;
    ent->nextthink = level->time + 29000;
}

void __cdecl My_Touch_Item(gentity_t *ent, gentity_t *other, trace_t *trace) {
    if (ent->parent == other) return;
    Touch_Item(ent, other, trace);
}

static PyObject* PyMinqlx_DropHoldable(PyObject* self, PyObject* args) {
    int client_id, item;
    vec3_t velocity;
    vec_t angle;
    if (!PyArg_ParseTuple(args, "i:drop_holdable", &client_id))
        return NULL;
    else if (client_id < 0 || client_id >= sv_maxclients->integer) {
        PyErr_Format(PyExc_ValueError,
                     "client_id needs to be a number from 0 to %d.",
                     sv_maxclients->integer);
        return NULL;
    }
    else if (!g_entities[client_id].client)
        Py_RETURN_FALSE;

    // removing kamikaze flag (surrounding skulls)
    g_entities[client_id].client->ps.eFlags &= ~EF_KAMIKAZE;

    item = g_entities[client_id].client->ps.stats[STAT_HOLDABLE_ITEM];
    if (item == 0) Py_RETURN_FALSE;

    angle = g_entities[client_id].s.apos.trBase[1] * (M_PI*2 / 360);
    velocity[0] = 150*cos(angle);
    velocity[1] = 150*sin(angle);
    velocity[2] = 250;

    gentity_t* entity = LaunchItem(bg_itemlist + item, g_entities[client_id].s.pos.trBase, velocity);
    entity->touch     = (void*)My_Touch_Item;
    entity->parent    = &g_entities[client_id];
    entity->think     = Switch_Touch_Item;
    entity->nextthink = level->time + 1000;
    entity->s.pos.trTime = level->time - 500;

    // removing holdable from player entity
    g_entities[client_id].client->ps.stats[STAT_HOLDABLE_ITEM] = 0;

    Py_RETURN_TRUE;
}


/*
* ================================================================
*                           set_flight
* ================================================================
*/

static PyObject* PyMinqlx_SetFlight(PyObject* self, PyObject* args) {
    int client_id;
    PyObject* flight;
    if (!PyArg_ParseTuple(args, "iO:set_flight", &client_id, &flight))
        return NULL;
    else if (client_id < 0 || client_id >= sv_maxclients->integer) {
        PyErr_Format(PyExc_ValueError,
                     "client_id needs to be a number from 0 to %d.",
                     sv_maxclients->integer);
        return NULL;
    }
    else if (!g_entities[client_id].client)
        Py_RETURN_FALSE;
    else if (!PyObject_TypeCheck(flight, &flight_type)) {
        PyErr_Format(PyExc_ValueError, "Argument must be of type minqlx.Flight.");
        return NULL;
    }

    for (int i = 0; i < flight_desc.n_in_sequence; i++)
        if (!PyLong_Check(PyStructSequence_GetItem(flight, i))) {
            PyErr_Format(PyExc_ValueError, "Tuple argument %d is not an integer.", i);
            return NULL;
        }

    g_entities[client_id].client->ps.stats[STAT_CUR_FLIGHT_FUEL] = PyLong_AsLong(PyStructSequence_GetItem(flight, 0));
    g_entities[client_id].client->ps.stats[STAT_MAX_FLIGHT_FUEL] = PyLong_AsLong(PyStructSequence_GetItem(flight, 1));
    g_entities[client_id].client->ps.stats[STAT_FLIGHT_THRUST] = PyLong_AsLong(PyStructSequence_GetItem(flight, 2));
    g_entities[client_id].client->ps.stats[STAT_FLIGHT_REFUEL] = PyLong_AsLong(PyStructSequence_GetItem(flight, 3));
    Py_RETURN_TRUE;
}

/*
* ================================================================
*                        set_invulnerability
* ================================================================
*/

static PyObject* PyMinqlx_SetInvulnerability(PyObject* self, PyObject* args) {
    int client_id, time;
    if (!PyArg_ParseTuple(args, "ii:set_invulnerability", &client_id, &time))
        return NULL;
    else if (client_id < 0 || client_id >= sv_maxclients->integer) {
        PyErr_Format(PyExc_ValueError,
                     "client_id needs to be a number from 0 to %d.",
                     sv_maxclients->integer);
        return NULL;
    }
    else if (!g_entities[client_id].client)
        Py_RETURN_FALSE;
    else if (time <= 0) {
        PyErr_Format(PyExc_ValueError, "time needs to be positive integer.");
        return NULL;
    }

    g_entities[client_id].client->invulnerabilityTime = level->time + time;
    Py_RETURN_TRUE;
}

/*
* ================================================================
*                           set_score
* ================================================================
*/

static PyObject* PyMinqlx_SetScore(PyObject* self, PyObject* args) {
    int client_id, score;
    if (!PyArg_ParseTuple(args, "ii:set_score", &client_id, &score))
        return NULL;
    else if (client_id < 0 || client_id >= sv_maxclients->integer) {
        PyErr_Format(PyExc_ValueError,
                     "client_id needs to be a number from 0 to %d.",
                     sv_maxclients->integer);
        return NULL;
    }
    else if (!g_entities[client_id].client)
        Py_RETURN_FALSE;

    g_entities[client_id].client->ps.persistant[PERS_ROUND_SCORE] = score;
    Py_RETURN_TRUE;
}

/*
* ================================================================
*                           callvote
* ================================================================
*/

static PyObject* PyMinqlx_Callvote(PyObject* self, PyObject* args) {
    char *vote, *vote_disp;
    int vote_time = 30;
    char buf[64];
    if (!PyArg_ParseTuple(args, "ss|i:callvote", &vote, &vote_disp, &vote_time))
        return NULL;

    strncpy(level->voteString, vote, sizeof(level->voteString));
    strncpy(level->voteDisplayString, vote_disp, sizeof(level->voteDisplayString));
    level->voteTime = (level->time - 30000) + vote_time * 1000;
    level->voteYes = 0;
    level->voteNo = 0;

    for (int i = 0; i < sv_maxclients->integer; i++)
        if (g_entities[i].client)
            g_entities[i].client->pers.voteState = VOTE_PENDING;

    My_SV_SetConfigstring(CS_VOTE_STRING, level->voteDisplayString);
    snprintf(buf, sizeof(buf), "%d", level->voteTime);
    My_SV_SetConfigstring(CS_VOTE_TIME, buf);
    My_SV_SetConfigstring(CS_VOTE_YES, "0");
    My_SV_SetConfigstring(CS_VOTE_NO, "0");

    Py_RETURN_NONE;
}

/*
* ================================================================
*                      allow_single_player
* ================================================================
*/

static PyObject* PyMinqlx_AllowSinglePlayer(PyObject* self, PyObject* args) {
    int x;
    if (!PyArg_ParseTuple(args, "p:allow_single_player", &x))
        return NULL;

    if (x)
        level->mapIsTrainingMap = qtrue;
    else
        level->mapIsTrainingMap = qfalse;

    Py_RETURN_NONE;
}

/*
* ================================================================
*                           player_spawn
* ================================================================
*/

static PyObject* PyMinqlx_PlayerSpawn(PyObject* self, PyObject* args) {
    int client_id;
    if (!PyArg_ParseTuple(args, "i:player_spawn", &client_id))
        return NULL;
    else if (client_id < 0 || client_id >= sv_maxclients->integer) {
        PyErr_Format(PyExc_ValueError,
                     "client_id needs to be a number from 0 to %d.",
                     sv_maxclients->integer);
        return NULL;
    }
    else if (!g_entities[client_id].client)
        Py_RETURN_FALSE;

    g_entities[client_id].client->ps.pm_type = PM_NORMAL;
    My_ClientSpawn(&g_entities[client_id]);
    Py_RETURN_TRUE;
}

/*
* ================================================================
*                           set_privileges
* ================================================================
*/

static PyObject* PyMinqlx_SetPrivileges(PyObject* self, PyObject* args) {
    int client_id, priv;
    if (!PyArg_ParseTuple(args, "ii:set_privileges", &client_id, &priv))
        return NULL;
    else if (client_id < 0 || client_id >= sv_maxclients->integer) {
        PyErr_Format(PyExc_ValueError,
                     "client_id needs to be a number from 0 to %d.",
                     sv_maxclients->integer);
        return NULL;
    }
    else if (!g_entities[client_id].client)
        Py_RETURN_FALSE;

    g_entities[client_id].client->sess.privileges = priv;
    Py_RETURN_TRUE;
}

/*
* ================================================================
*                        destroy_kamikaze_timers
* ================================================================
*/

static PyObject* PyMinqlx_DestroyKamikazeTimers(PyObject* self, PyObject* args) {
    int i;
    gentity_t* ent;

    for (i = 0; i < MAX_GENTITIES; i++) {
        ent = &g_entities[i];
        if (!ent->inuse)
            continue;

        // removing kamikaze skull from dead body
        if (ent->client && ent->health <= 0) {
            ent->client->ps.eFlags &= ~EF_KAMIKAZE;
        }

        if (strcmp(ent->classname, "kamikaze timer") == 0)
            G_FreeEntity(ent);
    }
    Py_RETURN_TRUE;
}

/*
* ================================================================
*                        spawn_item
* ================================================================
*/

static PyObject* PyMinqlx_SpawnItem(PyObject* self, PyObject* args) {
    int item_id, x, y, z;
    if (!PyArg_ParseTuple(args, "iiii:spawn_item", &item_id, &x, &y, &z))
        return NULL;
    if (item_id < 1 || item_id >= bg_numItems) {
        PyErr_Format(PyExc_ValueError,
                     "item_id needs to be a number from 1 to %d.",
                     bg_numItems);
        return NULL;
    }

    vec3_t origin = {x, y, z};
    vec3_t velocity = {0};

    gentity_t* ent = LaunchItem(bg_itemlist + item_id, origin, velocity);
    ent->nextthink = 0;
    ent->think = 0;
    G_AddEvent(ent, EV_ITEM_RESPAWN, 0); // make item be scaled up

    Py_RETURN_TRUE;
}

/*
* ================================================================
*                        remove_dropped_items
* ================================================================
*/

static PyObject* PyMinqlx_RemoveDroppedItems(PyObject* self, PyObject* args) {
    int i;
    gentity_t* ent;

    for (i = 0; i < MAX_GENTITIES; i++) {
        ent = &g_entities[i];
        if (!ent->inuse)
            continue;

        if (ent->flags & FL_DROPPED_ITEM)
            G_FreeEntity(ent);
    }
    Py_RETURN_TRUE;
}

/*
* ================================================================
*                         slay_with_mod
* ================================================================
*/

// it is alternative to Slay from command.c
static PyObject* PyMinqlx_SlayWithMod(PyObject* self, PyObject* args) {
    int client_id, mod;
    if (!PyArg_ParseTuple(args, "ii:slay_with_mod", &client_id, &mod))
        return NULL;
    else if (client_id < 0 || client_id >= sv_maxclients->integer) {
        PyErr_Format(PyExc_ValueError,
                     "client_id needs to be a number from 0 to %d.",
                     sv_maxclients->integer);
        return NULL;
    }
    else if (!g_entities[client_id].client)
        Py_RETURN_FALSE;
    else if (g_entities[client_id].health <= 0)
        Py_RETURN_TRUE;

    gentity_t* ent = &g_entities[client_id];
    int damage = g_entities[client_id].health + (mod == MOD_KAMIKAZE ? 100000 : 0);

    g_entities[client_id].client->ps.stats[STAT_ARMOR] = 0;

    // self damage = half damage, so multiplaying by 2
    G_Damage( ent, ent, ent, NULL, NULL, damage*2, DAMAGE_NO_PROTECTION, mod );
    Py_RETURN_TRUE;
}

/*
* ================================================================
*                         replace_items
* ================================================================
*/

void replace_item_core(gentity_t* ent, int item_id) {
    char csbuffer[4096];

    if (item_id) {
        ent->s.modelindex = item_id;
        ent->classname = bg_itemlist[item_id].classname;
        ent->item = &bg_itemlist[item_id];

        // this forces client to load new item
        SV_GetConfigstring(CS_ITEMS, csbuffer, sizeof(csbuffer));
        csbuffer[item_id] = '1';
        My_SV_SetConfigstring(CS_ITEMS, csbuffer);

    } else {
        G_FreeEntity(ent);
    }
}

static PyObject* PyMinqlx_ReplaceItems(PyObject* self, PyObject* args) {
    PyObject *arg1, *arg2 ;
    int entity_id = 0, item_id = 0;
    char *entity_classname = NULL, *item_classname = NULL;
    gentity_t* ent;


    if (!PyArg_ParseTuple(args, "OO:replace_items", &arg1, &arg2))
        return NULL;

    // checking type of first arg
    if (PyLong_Check(arg1)) {
        entity_id = PyLong_AsLong(arg1);
    } else if (PyUnicode_Check(arg1))
        entity_classname = PyUnicode_AsUTF8(arg1);
    else {
        PyErr_Format(PyExc_ValueError, "entity needs to be type of int or string.");
        return NULL;
    }

    // checking type of second arg
    if (PyLong_Check(arg2))
        item_id = PyLong_AsLong(arg2);
    else if (PyUnicode_Check(arg2))
        item_classname = PyUnicode_AsUTF8(arg2);
    else {
        PyErr_Format(PyExc_ValueError, "item needs to be type of int or string.");
        return NULL;
    }

    // convert second arg to item_id, if needed
    int i=1;
    if (item_classname == NULL) i=bg_numItems;
    for (; i<bg_numItems; i++)
        if (strcmp(bg_itemlist[i].classname, item_classname) == 0) {
            item_id = i;
            break;
        }

    // checking for valid item_id or item_classname
    if (item_classname && item_id == 0) {

        // throw error if invalid item_classname
        PyErr_Format(PyExc_ValueError, "invalid item classname: %s.", item_classname);
        return NULL;
    } else if (item_id < 0 || item_id >= bg_numItems) {

        // throw error if invalid item_id
        PyErr_Format(PyExc_ValueError, "item_id needs to be between 0 and %d.", bg_numItems-1);
        return NULL;
    }

    // Note: if item_id == 0 and item_classname == NULL, then item will be removed

    if (entity_classname == NULL) {
        // replacing item by entity_id

        // entity_id checking
        if (entity_id < 0 || entity_id >= MAX_GENTITIES) {
            PyErr_Format(PyExc_ValueError, "entity_id needs to be between 0 and %d.", MAX_GENTITIES-1);
            return NULL;
        } else if (g_entities[entity_id].inuse == 0) {
            PyErr_Format(PyExc_ValueError, "entity #%d is not in use.", entity_id);
            return NULL;
        } else if (g_entities[entity_id].s.eType != ET_ITEM) {
            PyErr_Format(PyExc_ValueError, "entity #%d is not item. Cannot replace it.", entity_id);
            return NULL;
        }

        Com_Printf("%s\n", g_entities[entity_id].classname);
        replace_item_core(&g_entities[entity_id], item_id);
        Py_RETURN_TRUE;

    } else {
        // replacing items by entity_classname

        int is_entity_found = 0;
        for (i=0; i<MAX_GENTITIES; i++) {
            ent = &g_entities[i];

            if (!ent->inuse)
                continue;

            if (ent->s.eType != ET_ITEM)
                continue;

            if (strcmp(ent->classname, entity_classname) == 0) {
                is_entity_found = 1;
                replace_item_core(ent, item_id);
            }
        }

        if (is_entity_found)
            Py_RETURN_TRUE;
        else
            Py_RETURN_FALSE;
    }
}

/*
* ================================================================
*                         dev_print_items
* ================================================================
*/

static PyObject* PyMinqlx_DevPrintItems(PyObject* self, PyObject* args) {
    gentity_t* ent;
    char buffer[1024], temp_buffer[1024];
    int buffer_index = 0;
    size_t chars_written;
    char format[] = "%d %s\n";
    qboolean is_buffer_enough = qtrue;

    // default results
    sprintf(buffer, "No items found in the map");

    for (int i=0; i<MAX_GENTITIES; i++) {
        ent = &g_entities[i];

        if (!ent->inuse)
            continue;

        if (ent->s.eType != ET_ITEM)
            continue;

        chars_written = sprintf(temp_buffer, format, i, ent->classname);
        if (is_buffer_enough && buffer_index + chars_written >= sizeof(buffer)) {
            is_buffer_enough = qfalse;
            SV_SendServerCommand(NULL, "print \"%s\"", buffer);
            SV_SendServerCommand(NULL, "print \"%s\"\n", "Check server console for other items\n");
        }

        if (is_buffer_enough == qfalse) {
            Com_Printf(format, i, ent->classname);
        }

        chars_written = sprintf(&buffer[buffer_index], format, i, ent->classname);
        buffer_index += chars_written;
    }

    if (is_buffer_enough)
        SV_SendServerCommand(NULL, "print \"%s\"", buffer);
    Py_RETURN_NONE;
}

/*
* ================================================================
*                         force_weapon_respawn_time
* ================================================================
*/

static PyObject* PyMinqlx_ForceWeaponRespawnTime(PyObject* self, PyObject* args) {
	int respawn_time;
    gentity_t* ent;
	
	if (!PyArg_ParseTuple(args, "i:force_weapon_respawn_time", &respawn_time))
		return NULL;
	
	if (respawn_time < 0) {    
        PyErr_Format(PyExc_ValueError, "respawn time needs to be an integer 0 or greater");
        return NULL;
    }	

    for (int i=0; i<MAX_GENTITIES; i++) {
        ent = &g_entities[i];

        if (!ent->inuse)
            continue;

		if (ent->s.eType != ET_ITEM || ent->item == NULL)
            continue;		
		
		if (ent->item->giType != IT_WEAPON)
			continue;
				
		ent->wait = respawn_time;
    }

    Py_RETURN_TRUE;
}

/*
 * ================================================================
 *             Module definition and initialization
 * ================================================================
*/

static PyMethodDef minqlxMethods[] = {
    {"player_info", PyMinqlx_PlayerInfo, METH_VARARGS,
     "Returns a dictionary with information about a player by ID."},
	{"players_info", PyMinqlx_PlayersInfo, METH_NOARGS,
	 "Returns a list with dictionaries with information about all the players on the server."},
	{"get_userinfo", PyMinqlx_GetUserinfo, METH_VARARGS,
	 "Returns a string with a player's userinfo."},
    {"send_server_command", PyMinqlx_SendServerCommand, METH_VARARGS,
     "Sends a server command to either one specific client or all the clients."},
	{"client_command", PyMinqlx_ClientCommand, METH_VARARGS,
	 "Tells the server to process a command from a specific client."},
	{"console_command", PyMinqlx_ConsoleCommand, METH_VARARGS,
	 "Executes a command as if it was executed from the server console."},
	{"get_cvar", PyMinqlx_GetCvar, METH_VARARGS,
	 "Gets a cvar."},
	{"set_cvar", PyMinqlx_SetCvar, METH_VARARGS,
	 "Sets a cvar."},
    {"set_cvar_limit", PyMinqlx_SetCvarLimit, METH_VARARGS,
     "Sets a non-string cvar with a minimum and maximum value."},
	{"kick", PyMinqlx_Kick, METH_VARARGS,
	 "Kick a player and allowing the admin to supply a reason for it."},
	{"console_print", PyMinqlx_ConsolePrint, METH_VARARGS,
	 "Prints text on the console. If used during an RCON command, it will be printed in the player's console."},
	{"get_configstring", PyMinqlx_GetConfigstring, METH_VARARGS,
	 "Get a configstring."},
	{"set_configstring", PyMinqlx_SetConfigstring, METH_VARARGS,
	 "Sets a configstring and sends it to all the players on the server."},
	{"force_vote", PyMinqlx_ForceVote, METH_VARARGS,
	 "Forces the current vote to either fail or pass."},
	{"add_console_command", PyMinqlx_AddConsoleCommand, METH_VARARGS,
	 "Adds a console command that will be handled by Python code."},
    {"register_handler", PyMinqlx_RegisterHandler, METH_VARARGS,
     "Register an event handler. Can be called more than once per event, but only the last one will work."},
    {"player_state", PyMinqlx_PlayerState, METH_VARARGS,
     "Get information about the player's state in the game."},
    {"player_stats", PyMinqlx_PlayerStats, METH_VARARGS,
     "Get some player stats."},
    {"set_position", PyMinqlx_SetPosition, METH_VARARGS,
     "Sets a player's position vector."},
    {"set_velocity", PyMinqlx_SetVelocity, METH_VARARGS,
     "Sets a player's velocity vector."},
    {"noclip", PyMinqlx_NoClip, METH_VARARGS,
     "Sets noclip for a player."},
    {"set_health", PyMinqlx_SetHealth, METH_VARARGS,
     "Sets a player's health."},
    {"set_armor", PyMinqlx_SetArmor, METH_VARARGS,
     "Sets a player's armor."},
    {"set_weapons", PyMinqlx_SetWeapons, METH_VARARGS,
     "Sets a player's weapons."},
    {"set_weapon", PyMinqlx_SetWeapon, METH_VARARGS,
     "Sets a player's current weapon."},
    {"set_ammo", PyMinqlx_SetAmmo, METH_VARARGS,
     "Sets a player's ammo."},
    {"set_powerups", PyMinqlx_SetPowerups, METH_VARARGS,
     "Sets a player's powerups."},
    {"set_holdable", PyMinqlx_SetHoldable, METH_VARARGS,
     "Sets a player's holdable item."},
    {"drop_holdable", PyMinqlx_DropHoldable, METH_VARARGS,
     "Drops player's holdable item."},
    {"set_flight", PyMinqlx_SetFlight, METH_VARARGS,
     "Sets a player's flight parameters, such as current fuel, max fuel and, so on."},
    {"set_invulnerability", PyMinqlx_SetInvulnerability, METH_VARARGS,
     "Makes player invulnerable for limited time."},
    {"set_score", PyMinqlx_SetScore, METH_VARARGS,
     "Sets a player's score."},
    {"callvote", PyMinqlx_Callvote, METH_VARARGS,
     "Calls a vote as if started by the server and not a player."},
    {"allow_single_player", PyMinqlx_AllowSinglePlayer, METH_VARARGS,
     "Allows or disallows a game with only a single player in it to go on without forfeiting. Useful for race."},
    {"player_spawn", PyMinqlx_PlayerSpawn, METH_VARARGS,
     "Allows or disallows a game with only a single player in it to go on without forfeiting. Useful for race."},
    {"set_privileges", PyMinqlx_SetPrivileges, METH_VARARGS,
     "Sets a player's privileges. Does not persist."},
    {"destroy_kamikaze_timers", PyMinqlx_DestroyKamikazeTimers, METH_NOARGS,
     "Removes all current kamikaze timers."},
    {"spawn_item", PyMinqlx_SpawnItem, METH_VARARGS,
     "Spawns item with specified coordinates."},
    {"remove_dropped_items", PyMinqlx_RemoveDroppedItems, METH_NOARGS,
     "Removes all dropped items."},
    {"slay_with_mod", PyMinqlx_SlayWithMod, METH_VARARGS,
     "Slay player with mean of death."},
    {"replace_items", PyMinqlx_ReplaceItems, METH_VARARGS,
     "Replaces target entity's item with specified one."},
    {"dev_print_items", PyMinqlx_DevPrintItems, METH_NOARGS,
     "Prints all items and entity numbers to server console."},
    {"force_weapon_respawn_time", PyMinqlx_ForceWeaponRespawnTime, METH_VARARGS,
     "Force all weapons to have a specified respawn time, overriding custom map respawn times set for them."},
    {NULL, NULL, 0, NULL}
};

static PyModuleDef minqlxModule = {
    PyModuleDef_HEAD_INIT, "minqlx", NULL, -1, minqlxMethods,
    NULL, NULL, NULL, NULL
};

static PyObject* PyMinqlx_InitModule(void) {
    PyObject* module = PyModule_Create(&minqlxModule);

    // Set minqlx version.
    PyModule_AddStringConstant(module, "__version__", MINQLX_VERSION);

    // Set IS_DEBUG.
    #ifndef NDEBUG
    PyModule_AddObject(module, "DEBUG", Py_True);
    #else
    PyModule_AddObject(module, "DEBUG", Py_False);
    #endif

    // Set a bunch of constants. We set them here because if you define functions in Python that use module
    // constants as keyword defaults, we have to always make sure they're exported first, and fuck that.
    PyModule_AddIntMacro(module, RET_NONE);
    PyModule_AddIntMacro(module, RET_STOP);
    PyModule_AddIntMacro(module, RET_STOP_EVENT);
    PyModule_AddIntMacro(module, RET_STOP_ALL);
    PyModule_AddIntMacro(module, RET_USAGE);
    PyModule_AddIntMacro(module, PRI_HIGHEST);
    PyModule_AddIntMacro(module, PRI_HIGH);
    PyModule_AddIntMacro(module, PRI_NORMAL);
    PyModule_AddIntMacro(module, PRI_LOW);
    PyModule_AddIntMacro(module, PRI_LOWEST);

    // Cvar flags.
    PyModule_AddIntMacro(module, CVAR_ARCHIVE);
    PyModule_AddIntMacro(module, CVAR_USERINFO);
    PyModule_AddIntMacro(module, CVAR_SERVERINFO);
    PyModule_AddIntMacro(module, CVAR_SYSTEMINFO);
    PyModule_AddIntMacro(module, CVAR_INIT);
    PyModule_AddIntMacro(module, CVAR_LATCH);
    PyModule_AddIntMacro(module, CVAR_ROM);
    PyModule_AddIntMacro(module, CVAR_USER_CREATED);
    PyModule_AddIntMacro(module, CVAR_TEMP);
    PyModule_AddIntMacro(module, CVAR_CHEAT);
    PyModule_AddIntMacro(module, CVAR_NORESTART);

    // Privileges.
    PyModule_AddIntMacro(module, PRIV_NONE);
    PyModule_AddIntMacro(module, PRIV_MOD);
    PyModule_AddIntMacro(module, PRIV_ADMIN);
    PyModule_AddIntMacro(module, PRIV_ROOT);
    PyModule_AddIntMacro(module, PRIV_BANNED);

    // Connection states.
    PyModule_AddIntMacro(module, CS_FREE);
    PyModule_AddIntMacro(module, CS_ZOMBIE);
    PyModule_AddIntMacro(module, CS_CONNECTED);
    PyModule_AddIntMacro(module, CS_PRIMED);
    PyModule_AddIntMacro(module, CS_ACTIVE);

    // Teams.
    PyModule_AddIntMacro(module, TEAM_FREE);
    PyModule_AddIntMacro(module, TEAM_RED);
    PyModule_AddIntMacro(module, TEAM_BLUE);
    PyModule_AddIntMacro(module, TEAM_SPECTATOR);

    // Means of death.
    PyModule_AddIntMacro(module, MOD_UNKNOWN);
    PyModule_AddIntMacro(module, MOD_SHOTGUN);
    PyModule_AddIntMacro(module, MOD_GAUNTLET);
    PyModule_AddIntMacro(module, MOD_MACHINEGUN);
    PyModule_AddIntMacro(module, MOD_GRENADE);
    PyModule_AddIntMacro(module, MOD_GRENADE_SPLASH);
    PyModule_AddIntMacro(module, MOD_ROCKET);
    PyModule_AddIntMacro(module, MOD_ROCKET_SPLASH);
    PyModule_AddIntMacro(module, MOD_PLASMA);
    PyModule_AddIntMacro(module, MOD_PLASMA_SPLASH);
    PyModule_AddIntMacro(module, MOD_RAILGUN);
    PyModule_AddIntMacro(module, MOD_LIGHTNING);
    PyModule_AddIntMacro(module, MOD_BFG);
    PyModule_AddIntMacro(module, MOD_BFG_SPLASH);
    PyModule_AddIntMacro(module, MOD_WATER);
    PyModule_AddIntMacro(module, MOD_SLIME);
    PyModule_AddIntMacro(module, MOD_LAVA);
    PyModule_AddIntMacro(module, MOD_CRUSH);
    PyModule_AddIntMacro(module, MOD_TELEFRAG);
    PyModule_AddIntMacro(module, MOD_FALLING);
    PyModule_AddIntMacro(module, MOD_SUICIDE);
    PyModule_AddIntMacro(module, MOD_TARGET_LASER);
    PyModule_AddIntMacro(module, MOD_TRIGGER_HURT);
    PyModule_AddIntMacro(module, MOD_NAIL);
    PyModule_AddIntMacro(module, MOD_CHAINGUN);
    PyModule_AddIntMacro(module, MOD_PROXIMITY_MINE);
    PyModule_AddIntMacro(module, MOD_KAMIKAZE);
    PyModule_AddIntMacro(module, MOD_JUICED);
    PyModule_AddIntMacro(module, MOD_GRAPPLE);
    PyModule_AddIntMacro(module, MOD_SWITCH_TEAMS);
    PyModule_AddIntMacro(module, MOD_THAW);
    PyModule_AddIntMacro(module, MOD_LIGHTNING_DISCHARGE);
    PyModule_AddIntMacro(module, MOD_HMG);
    PyModule_AddIntMacro(module, MOD_RAILGUN_HEADSHOT);

    // Initialize struct sequence types.
    PyStructSequence_InitType(&player_info_type, &player_info_desc);
    PyStructSequence_InitType(&player_state_type, &player_state_desc);
    PyStructSequence_InitType(&player_stats_type, &player_stats_desc);
    PyStructSequence_InitType(&vector3_type, &vector3_desc);
    PyStructSequence_InitType(&weapons_type, &weapons_desc);
    PyStructSequence_InitType(&powerups_type, &powerups_desc);
    PyStructSequence_InitType(&flight_type, &flight_desc);
    Py_INCREF((PyObject*)&player_info_type);
    Py_INCREF((PyObject*)&player_state_type);
    Py_INCREF((PyObject*)&player_stats_type);
    Py_INCREF((PyObject*)&vector3_type);
    Py_INCREF((PyObject*)&weapons_type);
    Py_INCREF((PyObject*)&powerups_type);
    Py_INCREF((PyObject*)&flight_type);
    // Add new types.
    PyModule_AddObject(module, "PlayerInfo", (PyObject*)&player_info_type);
    PyModule_AddObject(module, "PlayerState", (PyObject*)&player_state_type);
    PyModule_AddObject(module, "PlayerStats", (PyObject*)&player_stats_type);
    PyModule_AddObject(module, "Vector3", (PyObject*)&vector3_type);
    PyModule_AddObject(module, "Weapons", (PyObject*)&weapons_type);
    PyModule_AddObject(module, "Powerups", (PyObject*)&powerups_type);
    PyModule_AddObject(module, "Flight", (PyObject*)&flight_type);

    return module;
}

int PyMinqlx_IsInitialized(void) {
   return initialized;
}

PyMinqlx_InitStatus_t PyMinqlx_Initialize(void) {
    if (PyMinqlx_IsInitialized()) {
        DebugPrint("%s was called while already initialized!\n", __func__);
        return PYM_ALREADY_INITIALIZED;
    }

    DebugPrint("Initializing Python...\n");
    Py_SetProgramName(PYTHON_FILENAME);
    PyImport_AppendInittab("_minqlx", &PyMinqlx_InitModule);
    Py_Initialize();
    PyEval_InitThreads();

    // Add the main module.
    PyObject* main_module = PyImport_AddModule("__main__");
    PyObject* main_dict = PyModule_GetDict(main_module);
    // Run script to load pyminqlx.
    PyObject* res = PyRun_String(loader, Py_file_input, main_dict, main_dict);
    if (res == NULL) {
		DebugPrint("PyRun_String() returned NULL. Did you modify the loader?\n");
		return PYM_MAIN_SCRIPT_ERROR;
	}
    PyObject* ret = PyDict_GetItemString(main_dict, "ret");
    Py_XDECREF(ret);
    Py_DECREF(res);
    if (ret == NULL) {
		DebugPrint("The loader script return value doesn't exist?\n");
		return PYM_MAIN_SCRIPT_ERROR;
	}
	else if (ret != Py_True) {
		// No need to print anything, since the traceback should be printed already.
		return PYM_MAIN_SCRIPT_ERROR;
	}

    mainstate = PyEval_SaveThread();
    initialized = 1;
    DebugPrint("Python initialized!\n");
    return PYM_SUCCESS;
}

PyMinqlx_InitStatus_t PyMinqlx_Finalize(void) {
    if (!PyMinqlx_IsInitialized()) {
        DebugPrint("%s was called before being initialized!\n", __func__);
        return PYM_NOT_INITIALIZED_ERROR;
    }

    for (handler_t* h = handlers; h->name; h++) {
		*h->handler = NULL;
	}

    PyEval_RestoreThread(mainstate);
    Py_Finalize();
    initialized = 0;

    return PYM_SUCCESS;
}
