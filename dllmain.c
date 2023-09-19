#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
#include <string.h>
#include <signal.h>
#include <time.h>

#include "common.h"
#include "quake_common.h"
#include "patterns.h"
#include "maps_parser.h"
#ifndef NOPY
#include "pyminqlx.h"
#endif

// For comparison with the dedi's executable name to avoid segfaulting
// bash and the likes if we run this through a script.
extern char* __progname;

#if defined(__x86_64__) || defined(_M_X64)
const char qzeroded[] = "qzeroded.x64";
const char qagame_name[] = "qagamex64.so";
#elif defined(__i386) || defined(_M_IX86)
const char qzeroded[] = "qzeroded.x86";
const char qagame_name[] = "qagamei386.so";
#endif

// Global variables.
int common_initialized = 0;
int cvars_initialized = 0;
serverStatic_t* svs;

Com_Printf_ptr Com_Printf;
Cmd_AddCommand_ptr Cmd_AddCommand;
Cmd_Args_ptr Cmd_Args;
Cmd_Argv_ptr Cmd_Argv;
Cmd_Argc_ptr Cmd_Argc;
Cmd_TokenizeString_ptr Cmd_TokenizeString;
Cbuf_ExecuteText_ptr Cbuf_ExecuteText;
Cvar_FindVar_ptr Cvar_FindVar;
Cvar_Get_ptr Cvar_Get;
Cvar_GetLimit_ptr Cvar_GetLimit;
Cvar_Set2_ptr Cvar_Set2;
SV_SendServerCommand_ptr SV_SendServerCommand;
SV_ExecuteClientCommand_ptr SV_ExecuteClientCommand;
SV_ClientEnterWorld_ptr SV_ClientEnterWorld;
SV_Shutdown_ptr SV_Shutdown;
SV_Map_f_ptr SV_Map_f;
SV_SetConfigstring_ptr SV_SetConfigstring;
SV_GetConfigstring_ptr SV_GetConfigstring;
SV_DropClient_ptr SV_DropClient;
Sys_SetModuleOffset_ptr Sys_SetModuleOffset;
SV_SpawnServer_ptr SV_SpawnServer;
Cmd_ExecuteString_ptr Cmd_ExecuteString;
Sys_IsLANAddress_ptr Sys_IsLANAddress;

// VM functions
G_RunFrame_ptr G_RunFrame;
G_AddEvent_ptr G_AddEvent;
G_InitGame_ptr G_InitGame;
CheckPrivileges_ptr CheckPrivileges;
ClientConnect_ptr ClientConnect;
ClientSpawn_ptr ClientSpawn;
G_Damage_ptr G_Damage;
Touch_Item_ptr Touch_Item;
LaunchItem_ptr LaunchItem;
Drop_Item_ptr Drop_Item;
G_StartKamikaze_ptr G_StartKamikaze;
G_FreeEntity_ptr G_FreeEntity;

// VM global variables.
gentity_t* g_entities;
level_locals_t* level;
gitem_t* bg_itemlist;
int bg_numItems;

// Cvars.
cvar_t* sv_maxclients;

// TODO: Make it output everything to a file too.
void DebugPrint(const char* fmt, ...) {
    va_list args;
	va_start(args, fmt);
    printf(DEBUG_PRINT_PREFIX);
    vprintf(fmt, args);
	va_end(args);
}

// TODO: Make it output everything to a file too.
void DebugError(const char* fmt, const char* file, int line, const char* func, ...) {
    va_list args;
	va_start(args, func);
    fprintf(stderr, DEBUG_ERROR_FORMAT, file, line, func);
    vfprintf(stderr, fmt, args);
	va_end(args);
}

#define STATIC_SEARCH(x, p, m) x = (x ## _ptr) PatternSearchModule(&module, p, m); if (x == NULL) { DebugPrint("ERROR: Unable to find " #x ".\n"); failed = 1;} else DebugPrint(#x ": %p\n", x)

static void SearchFunctions(void) {
	int failed = 0;
	module_info_t module;
	strcpy(module.name, qzeroded);
	int res = GetModuleInfo(&module);
	if (res <= 0) {
		DebugError("GetModuleInfo() returned %d.\n", __FILE__, __LINE__, __func__, res);
		failed = 1;
	}

	DebugPrint("Searching for necessary functions...\n");

	STATIC_SEARCH(Com_Printf, PTRN_COM_PRINTF, MASK_COM_PRINTF);
	STATIC_SEARCH(Cmd_AddCommand, PTRN_CMD_ADDCOMMAND, MASK_CMD_ADDCOMMAND);
	STATIC_SEARCH(Cmd_Args, PTRN_CMD_ARGS, MASK_CMD_ARGS);
	STATIC_SEARCH(Cmd_Argv, PTRN_CMD_ARGV, MASK_CMD_ARGV);
	STATIC_SEARCH(Cmd_TokenizeString, PTRN_CMD_TOKENIZESTRING, MASK_CMD_TOKENIZESTRING);
	STATIC_SEARCH(Cbuf_ExecuteText, PTRN_CBUF_EXECUTETEXT, MASK_CBUF_EXECUTETEXT);
	STATIC_SEARCH(Cvar_FindVar, PTRN_CVAR_FINDVAR, MASK_CVAR_FINDVAR);
	STATIC_SEARCH(Cvar_Get, PTRN_CVAR_GET, MASK_CVAR_GET);
	STATIC_SEARCH(Cvar_GetLimit, PTRN_CVAR_GETLIMIT, MASK_CVAR_GETLIMIT);
	STATIC_SEARCH(Cvar_Set2, PTRN_CVAR_SET2, MASK_CVAR_SET2);
	STATIC_SEARCH(SV_SendServerCommand, PTRN_SV_SENDSERVERCOMMAND, MASK_SV_SENDSERVERCOMMAND);
	STATIC_SEARCH(SV_ExecuteClientCommand, PTRN_SV_EXECUTECLIENTCOMMAND, MASK_SV_EXECUTECLIENTCOMMAND);
	STATIC_SEARCH(SV_Shutdown, PTRN_SV_SHUTDOWN, MASK_SV_SHUTDOWN);
	STATIC_SEARCH(SV_Map_f, PTRN_SV_MAP_F, MASK_SV_MAP_F);
	STATIC_SEARCH(SV_ClientEnterWorld, PTRN_SV_CLIENTENTERWORLD, MASK_SV_CLIENTENTERWORLD);
	STATIC_SEARCH(SV_SetConfigstring, PTRN_SV_SETCONFIGSTRING, MASK_SV_SETCONFIGSTRING);
	STATIC_SEARCH(SV_GetConfigstring, PTRN_SV_GETCONFIGSTRING, MASK_SV_GETCONFIGSTRING);
	STATIC_SEARCH(SV_DropClient, PTRN_SV_DROPCLIENT, MASK_SV_DROPCLIENT);
	STATIC_SEARCH(Sys_SetModuleOffset, PTRN_SYS_SETMODULEOFFSET, MASK_SYS_SETMODULEOFFSET);
	STATIC_SEARCH(SV_SpawnServer, PTRN_SV_SPAWNSERVER, MASK_SV_SPAWNSERVER);
	STATIC_SEARCH(Cmd_ExecuteString, PTRN_CMD_EXECUTESTRING, MASK_CMD_EXECUTESTRING);
	STATIC_SEARCH(Sys_IsLANAddress, PTRN_SYS_ISLANADDRESS, MASK_SYS_ISLANADDRESS);

	// Cmd_Argc is really small, making it hard to search for, so we use a reference to it instead.
	if (SV_Map_f != NULL) {
		Cmd_Argc = (Cmd_Argc_ptr)(*(int32_t*)OFFSET_RELP_CMD_ARGC + OFFSET_RELP_CMD_ARGC + 4);
		DebugPrint("Cmd_Argc: %p\n", Cmd_Argc);
	}

	if (failed) {
		DebugPrint("Exiting.\n");
		exit(1);
	}
}

#define VM_SEARCH(x, p, m) x = (x ## _ptr) PatternSearch((void*)((pint)qagame + 0xB000), 0xB0000, p, m); if (x == NULL) { DebugPrint("ERROR: Unable to find " #x ".\n"); failed = 1;} else DebugPrint(#x ": %p\n", x)

// NOTE: Some functions can easily and reliably be found on the VM_Call table instead.
void SearchVmFunctions(void) {
	int failed = 0;

	// For some reason, the module doesn't show up when reading /proc/self/maps.
	// Perhaps this needs to be called later? In any case, we know exactly where
	// the module is mapped, so I think this is fine. If it ever breaks, it'll
	// be trivial to fix.
	VM_SEARCH(G_AddEvent, PTRN_G_ADDEVENT, MASK_G_ADDEVENT);
	VM_SEARCH(CheckPrivileges, PTRN_CHECKPRIVILEGES, MASK_CHECKPRIVILEGES);
	VM_SEARCH(ClientConnect, PTRN_CLIENTCONNECT, MASK_CLIENTCONNECT);
	VM_SEARCH(ClientSpawn, PTRN_CLIENTSPAWN, MASK_CLIENTSPAWN);
	VM_SEARCH(G_Damage, PTRN_G_DAMAGE, MASK_G_DAMAGE);
	VM_SEARCH(Touch_Item, PTRN_TOUCH_ITEM, MASK_TOUCH_ITEM);
	VM_SEARCH(LaunchItem, PTRN_LAUNCHITEM, MASK_LAUNCHITEM);
	VM_SEARCH(Drop_Item, PTRN_DROP_ITEM, MASK_DROP_ITEM);
	VM_SEARCH(G_StartKamikaze, PTRN_G_STARTKAMIKAZE, MASK_G_STARTKAMIKAZE);
	VM_SEARCH(G_FreeEntity, PTRN_G_FREEENTITY, MASK_G_FREEENTITY);

	if (failed) {
			DebugPrint("Exiting.\n");
			exit(1);
	}
}

// Currently called by My_Cmd_AddCommand(), since it's called at a point where we
// can safely do whatever we do below. It'll segfault if we do it at the entry
// point, since functions like Cmd_AddCommand need initialization first.
void InitializeStatic(void) {
    DebugPrint("Initializing...\n");
    
    // Set the seed for our RNG.
    srand(time(NULL));

    Cmd_AddCommand("cmd", SendServerCommand);
    Cmd_AddCommand("cp", CenterPrint);
    Cmd_AddCommand("print", RegularPrint);
    Cmd_AddCommand("slap", Slap);
    Cmd_AddCommand("slay", Slay);
#ifndef NOPY
    Cmd_AddCommand("qlx", PyRcon);
    Cmd_AddCommand("pycmd", PyCommand);
    Cmd_AddCommand("pyrestart", RestartPython);
#endif
	
#ifndef NOPY
	// Initialize Python and run the main script.
	PyMinqlx_InitStatus_t res = PyMinqlx_Initialize();
    if (res != PYM_SUCCESS) {
        DebugPrint("Python initialization failed: %d\n", res);
        exit(1);
    }
#endif

    common_initialized = 1;
}

// Initialize VM stuff. Needs to be called whenever Sys_SetModuleOffset is called,
// after qagame pointer has been initialized.
void InitializeVm(void) {
    DebugPrint("Initializing VM pointers...\n");
#if defined(__x86_64__) || defined(_M_X64)
    g_entities = (gentity_t*)(*(int32_t*)OFFSET_RELP_G_ENTITIES + OFFSET_RELP_G_ENTITIES + 4);
    level = (level_locals_t*)(*(int32_t*)OFFSET_RELP_LEVEL + OFFSET_RELP_LEVEL + 4);
    bg_itemlist = (gitem_t*)*(int64_t*)((*(int32_t*)OFFSET_RELP_BG_ITEMLIST + OFFSET_RELP_BG_ITEMLIST + 4));
#elif defined(__i386) || defined(_M_IX86)
    g_entities = (gentity_t*)(*(int32_t*)OFFSET_RELP_G_ENTITIES + 0xCEFF4 + (pint)qagame);
    level = (level_locals_t*)(*(int32_t*)OFFSET_RELP_LEVEL + 0xCEFF4 + (pint)qagame);
    bg_itemlist = (gitem_t*)*(int32_t*)((*(int32_t*)OFFSET_RELP_BG_ITEMLIST + 0xCEFF4 + (pint)qagame));
#endif
    for (bg_numItems = 1; bg_itemlist[ bg_numItems ].classname; bg_numItems++);
}

// Called after the game is initialized.
void InitializeCvars(void) {
    sv_maxclients = Cvar_FindVar("sv_maxclients");
    
    cvars_initialized = 1;
}

__attribute__((constructor))
void EntryPoint(void) {
	if (strcmp(__progname, qzeroded))
		return;

	SearchFunctions();

	// Initialize some key structure pointers before hooking, since we
	// might use some of the functions that could be hooked later to
	// get the pointer, such as SV_SetConfigstring.
#if defined(__x86_64__) || defined(_M_X64)
    // 32-bit pointer. intptr_t added to suppress warning about the casting.
    svs = (serverStatic_t*)(intptr_t)(*(uint32_t*)OFFSET_PP_SVS);
#elif defined(__i386) || defined(_M_IX86)
    svs = *(serverStatic_t**)OFFSET_PP_SVS;
#endif

    // TODO: Write script to automatically set version based on git output
    //       when building and then make it print it here.
    DebugPrint("Shared library loaded!\n");
    HookStatic();
}
