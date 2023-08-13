#ifndef PYMINQLX_H
#define PYMINQLX_H

#define PYTHON_FILENAME L"python3"
#define CORE_MODULE "minqlx.zip"

#include <Python.h>

#include "quake_common.h"

// Used to determine whether or not initialization worked.
typedef enum {
    PYM_SUCCESS,
    PYM_PY_INIT_ERROR,
    PYM_MAIN_SCRIPT_ERROR,
    PYM_ALREADY_INITIALIZED,
    PYM_NOT_INITIALIZED_ERROR
} PyMinqlx_InitStatus_t;

// Used primarily in Python, but defined here and added using PyModule_AddIntMacro().
enum {
    RET_NONE,
    RET_STOP, // Stop execution of event handlers within Python.
	RET_STOP_EVENT, // Only stop the event, but let other handlers process it.
    RET_STOP_ALL, // Stop execution at an engine level. SCARY STUFF!
	RET_USAGE // Used for commands. Replies to the channel with a command's usage.
};

enum {
    PRI_HIGHEST,
    PRI_HIGH,
    PRI_NORMAL,
    PRI_LOW,
    PRI_LOWEST
};

int PyMinqlx_IsInitialized(void);
PyMinqlx_InitStatus_t PyMinqlx_Initialize(void);
PyMinqlx_InitStatus_t PyMinqlx_Finalize(void);

/*
 * Event handlers. Note that we're using simple PyObject pointers, meaning it only supports
 * a single handler for each event. I don't see the need for multiple handlers, since you can
 * do that more easily in Python-level code instead of C.
*/
typedef struct {
	char* name;
	PyObject** handler;
} handler_t;
extern PyObject* client_command_handler;
extern PyObject* server_command_handler;
extern PyObject* client_connect_handler;
extern PyObject* client_loaded_handler;
extern PyObject* client_disconnect_handler;
extern PyObject* frame_handler;
extern PyObject* new_game_handler;
extern PyObject* set_configstring_handler;
extern PyObject* rcon_handler;
extern PyObject* console_print_handler;
extern PyObject* client_spawn_handler;

extern PyObject* kamikaze_use_handler;
extern PyObject* kamikaze_explode_handler;
extern PyObject* damage_handler;

// Custom console command handler. These are commands added through Python that can be used
// from the console or using RCON.
extern PyObject* custom_command_handler;

// We need to explicitly tell player_info to not return None in the case where
// we are inside My_ClientConnect, because we want to call Python code before
// the real ClientConnect is called, which is where it sets the connection
// state from CS_FREE to CS_CONNECTED. Same thing with My_SV_DropClient.
extern int allow_free_client;

/* Dispatchers. These are called by hooks or whatever and should dispatch events to Python handlers.
 * The return values will often determine what is passed on to the engine. You could for instance
 * implement a chat filter by returning 0 whenever bad words are said through the client_command event.
 * Hell, it could even be used to fix bugs in the server or client (e.g. a broken userinfo command or
 * broken UTF sequences that could crash clients). */
char* ClientCommandDispatcher(int client_id, char* cmd);
char* ServerCommandDispatcher(int client_id, char* cmd);
void FrameDispatcher(void);
char* ClientConnectDispatcher(int client_id, int is_bot);
int ClientLoadedDispatcher(int client_id);
void ClientDisconnectDispatcher(int client_id, const char* reason);
void NewGameDispatcher(int restart);
char* SetConfigstringDispatcher(int index, char* value);
void RconDispatcher(const char* cmd);
char* ConsolePrintDispatcher(char* cmd);
void ClientSpawnDispatcher(int client_id);

void KamikazeUseDispatcher(int client_id);
void KamikazeExplodeDispatcher(int client_id, int is_used_on_demand);
void DamageDispatcher(int target_id, int attacker_id, int damage, int dflags, int mod);

#endif /* PYMINQLX_H */
