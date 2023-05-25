#include <Python.h>

#include "pyminqlx.h"
#include "quake_common.h"

int allow_free_client = -1;

char* ClientCommandDispatcher(int client_id, char* cmd) {
    char* ret = cmd;
    static char ccmd_buf[4096];
    if (!client_command_handler)
        return ret; // No registered handler.
    
    PyGILState_STATE gstate = PyGILState_Ensure();

    PyObject* cmd_string = PyUnicode_DecodeUTF8(cmd, strlen(cmd), "ignore");
    PyObject* result = PyObject_CallFunction(client_command_handler, "iO", client_id, cmd_string);
    
    if (result == NULL)
        DebugError("PyObject_CallFunction() returned NULL.\n",
                __FILE__, __LINE__, __func__);
    else if (PyBool_Check(result) && result == Py_False)
        ret = NULL;
    else if (PyUnicode_Check(result)) {
        strncpy(ccmd_buf, PyUnicode_AsUTF8(result), sizeof(ccmd_buf));
        ret = ccmd_buf;
    }
    
    Py_XDECREF(cmd_string);
    Py_XDECREF(result);

    PyGILState_Release(gstate);
    return ret;
}

char* ServerCommandDispatcher(int client_id, char* cmd) {
    char* ret = cmd;
    static char scmd_buf[4096];
    if (!server_command_handler)
        return ret; // No registered handler.

    PyGILState_STATE gstate = PyGILState_Ensure();

    PyObject* cmd_string = PyUnicode_DecodeUTF8(cmd, strlen(cmd), "ignore");
    PyObject* result = PyObject_CallFunction(server_command_handler, "iO", client_id, cmd_string);

    if (result == NULL)
        DebugError("PyObject_CallFunction() returned NULL.\n",
                __FILE__, __LINE__, __func__);
    else if (PyBool_Check(result) && result == Py_False)
        ret = NULL;
    else if (PyUnicode_Check(result)) {
        strncpy(scmd_buf, PyUnicode_AsUTF8(result), sizeof(scmd_buf));
        ret = scmd_buf;
    }

    Py_XDECREF(cmd_string);
    Py_XDECREF(result);

    PyGILState_Release(gstate);
    return ret;
}

void FrameDispatcher(void) {
    if (!frame_handler)
        return; // No registered handler.

    PyGILState_STATE gstate = PyGILState_Ensure();

    PyObject* result = PyObject_CallObject(frame_handler, NULL);

    Py_XDECREF(result);

    PyGILState_Release(gstate);
    return;
}

char* ClientConnectDispatcher(int client_id, int is_bot) {
	char* ret = NULL;
    static char connect_buf[4096];
	if (!client_connect_handler)
		return ret; // No registered handler.

	PyGILState_STATE gstate = PyGILState_Ensure();

	// Tell PyMinqlx_PlayerInfo it's OK to get player info for someone with CS_FREE.
	allow_free_client = client_id;
	PyObject* result = PyObject_CallFunction(client_connect_handler, "iO", client_id, is_bot ? Py_True : Py_False);
	allow_free_client = -1;

	if (result == NULL)
		DebugError("PyObject_CallFunction() returned NULL.\n",
				__FILE__, __LINE__, __func__);
	else if (PyBool_Check(result) && result == Py_False)
		ret = "You are banned from this server.";
	else if (PyUnicode_Check(result)) {
		strncpy(connect_buf, PyUnicode_AsUTF8(result), sizeof(connect_buf));
        ret = connect_buf;
    }

	Py_XDECREF(result);

	PyGILState_Release(gstate);
	return ret;
}

void ClientDisconnectDispatcher(int client_id, const char* reason) {
	if (!client_disconnect_handler)
		return; // No registered handler.

	PyGILState_STATE gstate = PyGILState_Ensure();

    // Tell PyMinqlx_PlayerInfo it's OK to get player info for someone with CS_FREE.
    allow_free_client = client_id;
	PyObject* result = PyObject_CallFunction(client_disconnect_handler, "is", client_id, reason);
    allow_free_client = -1;
	
    if (result == NULL)
		DebugError("PyObject_CallFunction() returned NULL.\n",
				__FILE__, __LINE__, __func__);

	Py_XDECREF(result);

	PyGILState_Release(gstate);
	return;
}

// Does not trigger on bots.
int ClientLoadedDispatcher(int client_id) {
	int ret = 1;
	if (!client_loaded_handler)
		return ret; // No registered handler.

	PyGILState_STATE gstate = PyGILState_Ensure();

	PyObject* result = PyObject_CallFunction(client_loaded_handler, "i", client_id);

	// Only change to 0 if we got False returned to us.
	if (result == NULL) {
		DebugError("PyObject_CallFunction() returned NULL.\n",
				__FILE__, __LINE__, __func__);
		PyGILState_Release(gstate);
		return ret;
	}
	else if (PyBool_Check(result) && result == Py_False) {
		ret = 0;
	}

	Py_XDECREF(result);

	PyGILState_Release(gstate);
	return ret;
}

void NewGameDispatcher(int restart) {
	if (!new_game_handler)
		return; // No registered handler.

	PyGILState_STATE gstate = PyGILState_Ensure();

	PyObject* result = PyObject_CallFunction(new_game_handler, "O", restart ? Py_True : Py_False);

	if (result == NULL)
		DebugError("PyObject_CallFunction() returned NULL.\n", __FILE__, __LINE__, __func__);

	Py_XDECREF(result);

	PyGILState_Release(gstate);
	return;
}

char* SetConfigstringDispatcher(int index, char* value) {
	char* ret = value;
    static char setcs_buf[4096];
	if (!set_configstring_handler)
		return ret; // No registered handler.

	PyGILState_STATE gstate = PyGILState_Ensure();

    PyObject* value_string = PyUnicode_DecodeUTF8(value, strlen(value), "ignore");
	PyObject* result = PyObject_CallFunction(set_configstring_handler, "iO", index, value_string);

	if (result == NULL)
		DebugError("PyObject_CallFunction() returned NULL.\n",
				__FILE__, __LINE__, __func__);
	else if (PyBool_Check(result) && result == Py_False)
		ret = NULL;
	else if (PyUnicode_Check(result)) {
		strncpy(setcs_buf, PyUnicode_AsUTF8(result), sizeof(setcs_buf));
        ret = setcs_buf;
    }

    Py_XDECREF(value_string);
	Py_XDECREF(result);

	PyGILState_Release(gstate);
	return ret;
}

void RconDispatcher(const char* cmd) {
    if (!rcon_handler)
        return; // No registered handler.

    PyGILState_STATE gstate = PyGILState_Ensure();

    PyObject* result = PyObject_CallFunction(rcon_handler, "s", cmd);

    if (result == NULL)
        DebugError("PyObject_CallFunction() returned NULL.\n",
                __FILE__, __LINE__, __func__);
    Py_XDECREF(result);

    PyGILState_Release(gstate);
}

char* ConsolePrintDispatcher(char* text) {
    char* ret = text;
    static char print_buf[4096];
    if (!console_print_handler)
        return ret; // No registered handler.

    PyGILState_STATE gstate = PyGILState_Ensure();

    PyObject* text_string = PyUnicode_DecodeUTF8(text, strlen(text), "ignore");
    PyObject* result = PyObject_CallFunction(console_print_handler, "O", text_string);

    if (result == NULL)
        DebugError("PyObject_CallFunction() returned NULL.\n",
                __FILE__, __LINE__, __func__);
    else if (PyBool_Check(result) && result == Py_False)
        ret = NULL;
    else if (PyUnicode_Check(result)) {
        strncpy(print_buf, PyUnicode_AsUTF8(result), sizeof(print_buf));
        ret = print_buf;
    }

    Py_XDECREF(text_string);
    Py_XDECREF(result);

    PyGILState_Release(gstate);
    return ret;
}

void ClientSpawnDispatcher(int client_id) {
    if (!client_spawn_handler)
        return; // No registered handler.

    PyGILState_STATE gstate = PyGILState_Ensure();

    PyObject* result = PyObject_CallFunction(client_spawn_handler, "i", client_id);

    // Only change to 0 if we got False returned to us.
    if (result == NULL) {
        DebugError("PyObject_CallFunction() returned NULL.\n",
                __FILE__, __LINE__, __func__);
    }
    Py_XDECREF(result);

    PyGILState_Release(gstate);
}

void KamikazeUseDispatcher(int client_id) {
    if (!kamikaze_use_handler)
        return; // No registered handler.

    PyGILState_STATE gstate = PyGILState_Ensure();

    PyObject* result = PyObject_CallFunction(kamikaze_use_handler, "i", client_id);

    // Only change to 0 if we got False returned to us.
    if (result == NULL) {
        DebugError("PyObject_CallFunction() returned NULL.\n",
                __FILE__, __LINE__, __func__);
    }
    Py_XDECREF(result);

    PyGILState_Release(gstate);
}

void KamikazeExplodeDispatcher(int client_id, int is_used_on_demand) {
    if (!kamikaze_explode_handler)
        return; // No registered handler.

    PyGILState_STATE gstate = PyGILState_Ensure();

    PyObject* result = PyObject_CallFunction(kamikaze_explode_handler, "ii", client_id, is_used_on_demand);

    // Only change to 0 if we got False returned to us.
    if (result == NULL) {
        DebugError("PyObject_CallFunction() returned NULL.\n",
                __FILE__, __LINE__, __func__);
    }
    Py_XDECREF(result);

    PyGILState_Release(gstate);
}

void DamageDispatcher(int target_id, int attacker_id, int damage, int dflags, int mod) {
    if (!damage_handler)
        return; // No registered handler

    PyGILState_STATE gstate = PyGILState_Ensure();

    PyObject* result
    if (attacker_id >= 0) {
        result = PyObject_CallFunction(damage_handler, "iiiii", target_id, attacker_id, damage, dflags, mod);
    } else {
        result = PyObject_CallFunction(damage_handler, "iOiii", target_id, Py_None, damage, dflags, mod);
    }
    
    Py_XDECREF(result);

    PyGILState_Release(gstate);
}
