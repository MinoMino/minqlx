#ifndef PATTERNS_H
#define PATTERNS_H

// RELOFFSET = Relative offset, i.e. needs a base added.
// RELP = Relative pointer, i.e. dereference this as int32_t, then add this + 4.
//        e.g. (*(int32_t*)OFFSET_RELP_G_ENTITIES + OFFSET_RELP_G_ENTITIES + 4)
// PP = Pointer to a pointer.

#if defined(__x86_64__) || defined(_M_X64)
// Generated by minfuncfind64.
#define PTRN_COM_PRINTF "\x41\x54\x55\x53\x48\x81\xec\x00\x00\x00\x00\x84\xc0\x48\x89\xb4\x24\x00\x00\x00\x00\x48\x89\x94\x24\x00\x00\x00\x00\x48\x89\x8c\x24\x00\x00\x00\x00\x4c\x89\x84\x24\x00\x00\x00\x00"
#define MASK_COM_PRINTF "XXXXXXX----XXXXXX----XXXX----XXXX----XXXX----"
#define PTRN_CMD_ADDCOMMAND "\x41\x55\x49\x89\xf5\x41\x54\x49\x89\xfc\x55\x53\x48\x83\xec\x00\x48\x8b\x1d\x00\x00\x00\x00\x48\x85\xdb\x75\x00\xeb\x00\x66\x90\x48\x8b\x1b\x48\x85\xdb\x74\x00\x48\x8b\x73\x00\x4c\x89\xe7"
#define MASK_CMD_ADDCOMMAND "XXXXXXXXXXXXXXX-XXX----XXXX-X-XXXXXXXXX-XXX-XXX"
#define PTRN_CMD_ARGS "\x8b\x05\x00\x00\x00\x00\xc6\x05\x00\x00\x00\x00\x00\x83\xf8\x00\x0f\x8e\x00\x00\x00\x00\x41\x54\x44\x8d\x60\x00\x83\xe8\x00\x55\x48\x8d\x68\x00\x53\x31\xdb\x66\x0f\x1f\x84\x00\x00\x00\x00\x00"
#define MASK_CMD_ARGS "XX----XX----XXX-XX----XXXXX-XX-XXXX-XXXXXXX----X"
#define PTRN_CMD_ARGV "\x3b\x3d\x00\x00\x00\x00\xb8\x00\x00\x00\x00\x73\x00\x48\x63\xff\x48\x8b\x04\xfd\x00\x00\x00\x00\xf3\xc3"
#define MASK_CMD_ARGV "XX----X----X-XXXXXXX----XX"
#define PTRN_CMD_ARGC "\x8b\x05\x00\x00\x00\x00\xc3"
#define MASK_CMD_ARGC "XX----X"
#define PTRN_CMD_TOKENIZESTRING "\x48\x85\xff\x53\xc7\x05\x00\x00\x44\x00\x00\x00\x00\x00\x48\x89\xfb\x0f\x84\x00\x00\x00\x00\x48\x89\xfe\xba\x00\x00\x00\x00\xbf\x00\x00\x00\x00\xe8\x00\x00\x00\x00\x8b\x0d\x00\x00\x00\x00"
#define MASK_CMD_TOKENIZESTRING "XXXXXX--X----XXXXXX----XXXX----X----X----XX----"
#define PTRN_CBUF_EXECUTETEXT "\x83\xff\x00\x74\x00\x83\xff\x00\x74\x00\x85\xff\x74\x00\xbe\x00\x00\x00\x00\x31\xff\x31\xc0\xe9\x00\x00\x00\x00\x0f\x1f\x40\x00\x48\x85\xf6\x74\x00\x80\x3e\x00\x75\x00\xe9\x00\x00\x00\x00\x90"
#define MASK_CBUF_EXECUTETEXT "XX-X-XX-X-XXX-X----XXXXX----XXX-XXXX-XX-X-X----X"
#define PTRN_CVAR_FINDVAR "\x55\x48\x89\xfd\x53\x48\x83\xec\x00\xe8\x00\x00\x00\x00\x48\x8b\x1c\xc5\x00\x00\x00\x00\x48\x85\xdb\x75\x00\xeb\x00\x0f\x1f\x00\x48\x8b\x5b\x00\x48\x85\xdb\x74\x00\x48\x8b\x33\x48\x89\xef"
#define MASK_CVAR_FINDVAR "XXXXXXXX-X----XXXX----XXXX-X-XXXXXX-XXXX-XXXXXX"
#define PTRN_CVAR_GET "\x41\x56\x48\x85\xff\x41\x55\x41\x89\xd5\x41\x54\x49\x89\xf4\x55\x48\x89\xfd\x53\x0f\x84\x00\x00\x00\x00\x48\x85\xf6\x0f\x84\x00\x00\x00\x00\x48\x89\xef\xe8\x00\x00\x00\x00\x85\xc0"
#define MASK_CVAR_GET "XXXXXXXXXXXXXXXXXXXXXX----XXXXX----XXXX----XX"
#define PTRN_CVAR_GETLIMIT "\x41\x57\x45\x89\xc7\x41\x56\x49\x89\xd6\x41\x55\x49\x89\xcd\x41\x54\x49\x89\xf4\x31\xf6\x55\x48\x89\xfd\x48\x89\xd7\x53\x48\x83\xec\x00\xe8\x00\x00\x00\x00\x66\x0f\x14\xc0\x31\xf6\x4c\x89\xef"
#define MASK_CVAR_GETLIMIT "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX-X----XXXXXXXXX"
#define PTRN_CVAR_SET2 "\x41\x57\x31\xc0\x41\x56\x41\x89\xd6\x48\x89\xf2\x41\x55\x41\x54\x49\x89\xf4\x48\x89\xfe\x55\x48\x89\xfd\xbf\x00\x00\x00\x00\x53\x48\x83\xec\x00\xe8\x00\x00\x00\x00\x48\x89\xef\xe8\x00\x00\x00\x00"
#define MASK_CVAR_SET2 "XXXXXXXXXXXXXXXXXXXXXXXXXXX----XXXX-X----XXXX----"
#define PTRN_SV_SENDSERVERCOMMAND "\x41\x55\x41\x54\x55\x48\x89\xfd\x53\x48\x81\xec\x00\x00\x00\x00\x84\xc0\x48\x89\x94\x24\x00\x00\x00\x00\x48\x89\x8c\x24\x00\x00\x00\x00\x4c\x89\x84\x24\x00\x00\x00\x00\x4c\x89\x8c\x24\x00\x00\x00\x00"
#define MASK_SV_SENDSERVERCOMMAND "XXXXXXXXXXXX----XXXXXX----XXXX----XXXX----XXXX----"
#define PTRN_SV_EXECUTECLIENTCOMMAND "\x41\x55\x41\x89\xd5\x41\x54\x49\x89\xfc\x48\x89\xf7\x55\xbd\x00\x00\x00\x00\x53\x48\x83\xec\x00\xe8\x00\x00\x00\x00\x48\x8b\x1d\x00\x00\x00\x00\x48\x85\xdb\x75\x00\xe9\x00\x00\x00\x00\x66\x90"
#define MASK_SV_EXECUTECLIENTCOMMAND "XXXXXXXXXXXXXXX----XXXX-X----XXX----XXXX-X----XX"
#define PTRN_SV_SHUTDOWN "\x48\x8b\x05\x00\x00\x00\x00\x48\x85\xc0\x74\x00\x44\x8b\x50\x00\x45\x85\xd2\x75\x00\xf3\xc3"
#define MASK_SV_SHUTDOWN "XXX----XXXX-XXX-XXXX-XX"
#define PTRN_SV_CLIENTENTERWORLD "\x41\x55\x31\xc0\x49\xbd\x00\x00\x00\x00\x00\x00\x00\x00\x41\x54\x49\x89\xf4\x48\x8d\xb7\x00\x00\x00\x00\x55\x53\x48\x89\xfb\xbf\x00\x00\x00\x00\x48\x89\xdd\x48\x83\xec\x00\xe8\x00\x00\x00\x00"
#define MASK_SV_CLIENTENTERWORLD "XXXXXX--------XXXXXXXX----XXXXXX----XXXXXX-X----"
#define PTRN_SV_CLIENTTHINK "\x48\x8b\x06\x83\x3f\x00\x48\x89\x87\x00\x00\x00\x00\x48\x8b\x46\x00\x48\x89\x87\x00\x00\x00\x00\x48\x8b\x46\x00\x48\x89\x87\x00\x00\x00\x00\x8b\x46\x00\x89\x87\x00\x00\x00\x00\x74\x00\xf3\xc3"
#define MASK_SV_CLIENTTHINK "XXXXX-XXX----XXX-XXX----XXX-XXX----XX-XX----X-XX"
#define PTRN_SV_SETCONFIGSTRING "\x41\x57\x41\x56\x41\x55\x41\x54\x41\x89\xfc\x55\x53\x48\x81\xec\x00\x00\x00\x00\x64\x48\x8b\x04\x25\x00\x00\x00\x00\x48\x89\x84\x24\x00\x00\x00\x00\x31\xc0\x81\xff\x00\x00\x00\x00\x48\x89\x74\x24\x00"
#define MASK_SV_SETCONFIGSTRING "XXXXXXXXXXXXXXXX----XXXXX----XXXX----XXXX----XXXX-"
#define PTRN_SV_GETCONFIGSTRING "\x41\x54\x85\xd2\x49\x89\xf4\x55\x89\xd5\x53\x48\x63\xdf\x7e\x00\x81\xfb\x00\x00\x00\x00\x77\x00\x48\x8b\x34\xdd\x00\x00\x00\x00\x48\x85\xf6\x74\x00\x5b\x89\xea\x4c\x89\xe7\x5d\x41\x5c"
#define MASK_SV_GETCONFIGSTRING "XXXXXXXXXXXXXXX-XX----X-XXXX----XXXX-XXXXXXXXX"
#define PTRN_SV_DROPCLIENT "\x41\x54\x55\x48\x89\xfd\x53\x48\x83\xec\x00\x83\x3f\x00\x0f\x84\x00\x00\x00\x00\x48\x8b\x87\x00\x00\x00\x00\x49\x89\xf4\x48\x85\xc0\x74\x00\xf6\x80\xe0\x01\x00\x00\x00\x75\x00\xbb\x00\x00\x00\x00"
#define MASK_SV_DROPCLIENT "XXXXXXXXXX-XX-XX----XXX----XXXXXXX-XXXXXX-X-X----"
#define PTRN_SV_MAP_F "\x41\x55\xbf\x00\x00\x00\x00\x41\x54\x55\x53\x48\x81\xec\x00\x00\x00\x00\x64\x48\x8b\x04\x25\x00\x00\x00\x00\x48\x89\x84\x24\x00\x00\x00\x00\x31\xc0\xe8\x00\x00\x00\x00\xbf\x00\x00\x00\x00\x48\x89\xc5"
#define MASK_SV_MAP_F "XXX----XXXXXXX----XXXXX----XXXX----XXX----X----XXX"
#define PTRN_SYS_SETMODULEOFFSET "\x55\x48\x89\xf2\x31\xc0\x48\x89\xf5\x48\x89\xfe\x53\x48\x89\xfb\xbf\x00\x00\x00\x00\x48\x83\xec\x00\xe8\x00\x00\x00\x00\xbf\x00\x00\x00\x00\xb9\x00\x00\x00\x00\x48\x89\xde\xf3\xa6\x74\x00"
#define MASK_SYS_SETMODULEOFFSET "XXXXXXXXXXXXXXXXX----XXX-X----X----X----XXXXXX-"
#define PTRN_VA "\x53\x48\x81\xec\x00\x00\x00\x00\x84\xc0\x48\x89\x74\x24\x00\x48\x89\x54\x24\x00\x48\x89\x4c\x24\x00\x4c\x89\x44\x24\x00\x4c\x89\x4c\x24\x00\x74\x00\x0f\x29\x44\x24\x00\x0f\x29\x4c\x24\x00"
#define MASK_VA "XXXX----XXXXXX-XXXX-XXXX-XXXX-XXXX-X-XXXX-XXXX-"
#define PTRN_CLIENTSPAWN "\x41\x57\x41\x56\x49\x89\xfe\x41\x55\x41\x54\x55\x53\x48\x81\xec\x00\x00\x00\x00\x4c\x8b\xbf\x00\x00\x00\x00\x64\x48\x8b\x04\x25\x00\x00\x00\x00\x48\x89\x84\x24\x00\x00\x00\x00\x31\xc0"
#define MASK_CLIENTSPAWN "XXXXXXXXXXXXXXXX----XXX----XXXXX----XXXX----XX"
#define PTRN_SV_SPAWNSERVER "\x41\x55\x41\x54\x41\x89\xf4\x55\x48\x89\xfd\x53\x48\x81\xec\x00\x00\x00\x00\x64\x48\x8b\x04\x25\x00\x00\x00\x00\x48\x89\x84\x24\x00\x00\x00\x00\x31\xc0\xe8\x00\x00\x00\x00\x31\xc0\xbf\x00\x00\x00\x00"
#define MASK_SV_SPAWNSERVER "XXXXXXXXXXXXXXX----XXXXX----XXXX----XXX----XXX----"
#define PTRN_CMD_EXECUTESTRING "\x41\x54\x49\x89\xfc\x55\x53\xe8\x00\x00\x00\x00\x44\x8b\x0d\x00\x00\x00\x00\x45\x85\xc9\x0f\x84\x00\x00\x00\x00\x48\x8b\x1d\x00\x00\x00\x00\xbd\x00\x00\x00\x00\x48\x85\xdb\x75\x00\xeb\x00\x90"
#define MASK_CMD_EXECUTESTRING "XXXXXXXX----XXX----XXXXX----XXX----X----XXXX-X-X"

// Generated by minfuncfind64. qagame functions.
#define PTRN_G_RUNFRAME "\x8b\x05\x00\x00\x00\x00\x85\xc0\x74\x00\xf3\xc3"
#define MASK_G_RUNFRAME "XX----XXX-XX"
#define PTRN_G_ADDEVENT "\x85\xf6\x74\x00\x48\x8b\x8f\x00\x00\x00\x00\x48\x85\xc9\x74\x00\x8b\x81\x00\x00\x00\x00\x25\x00\x00\x00\x00\x05\x00\x00\x00\x00\x25\x00\x00\x00\x00\x09\xf0\x89\x81\x00\x00\x00\x00"
#define MASK_G_ADDEVENT "XXX-XXX----XXXX-XX----X----X----X----XXXX----"
#define PTRN_G_INITGAME "\x41\x54\x55\x53\x48\x81\xec\x00\x00\x00\x00\x84\xc0\x48\x89\xb4\x24\x00\x00\x00\x00\x48\x89\x94\x24\x00\x00\x00\x00\x48\x89\x8c\x24\x00\x00\x00\x00\x4c\x89\x84\x24\x00\x00\x00\x00"
#define MASK_G_INITGAME "XXXXXXX----XXXXXX----XXXX----XXXX----XXXX----"
#define PTRN_CHECKPRIVILEGES "\x41\x56\x89\x15\x00\x00\x00\x00\x49\x89\xfe\x48\x8d\x3d\x00\x00\x00\x00\x41\x55\x41\x89\xd5\x41\x54\x49\x89\xf4\x55\x31\xed\x53\x48\x8d\x1d\x00\x00\x00\x00\xeb\x00\x0f\x1f\x80\x00\x00\x00\x00"
#define MASK_CHECKPRIVILEGES "XXXX----XXXXXX----XXXXXXXXXXXXXXXXX----X-XXX----"
#define PTRN_CLIENTCONNECT "\x41\x57\x4c\x63\xff\x41\x56\x41\x89\xf6\x41\x55\x41\x54\x55\x4c\x89\xfd\x48\xc1\xe5\x00\x53\x89\xfb\x48\x81\xec\x00\x00\x00\x00\x4c\x8b\x2d\x00\x00\x00\x00\x64\x48\x8b\x04\x25\x00\x00\x00\x00"
#define MASK_CLIENTCONNECT "XXXXXXXXXXXXXXXXXXXXX-XXXXXX----XXX----XXXXX----"
#define PTRN_GETCLIENTNAME "\x48\x83\xec\x00\x48\x8b\x05\x00\x00\x00\x00\x48\x63\xf7\x48\x69\xf6\x00\x00\x00\x00\x48\x8d\x3d\x00\x00\x00\x00\x48\x03\x30\x31\xc0\x48\x81\xc6\x00\x00\x00\x00\xe8\x00\x00\x00\x00"
#define MASK_GETCLIENTNAME "XXX-XXX----XXXXXX----XXX----XXXXXXXX----X----"
#define PTRN_BG_ITEMLIST "\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x5b\x8d\x50\x24\xa3\x7f\x0\x0\x6c\x8d\x50\x24\xa3\x7f\x0\x0\x98\x9b\x50\x24\xa3\x7f\x0\x0\xb8\x9b\x50\x24\xa3\x7f\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x84\x8d\x50\x24\xa3\x7f\x0\x0\x96\x8d\x50\x24\xa3\x7f\x0\x0\x5\x0\x0\x0\x3\x0\x0\x0\x4\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0"
#define MASK_BG_ITEMLIST "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX--------------------------------XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX----------------XXXXXXXXXXXXXXXXXXXXXXXX"
#define PTRN_TOUCH_ITEM "\x41\x57\x41\x56\x41\x55\x41\x54\x55\x53\x48\x89\xf3\x48\x81\xec\x00\x00\x00\x00\x4c\x8b\x86\x00\x00\x00\x00\x4d\x85\xc0\x74\x00\x8b\x96\x00\x00\x00\x00\x85\xd2\x7e\x00\x4c\x8b\x35\x00\x00\x00\x00"
#define MASK_TOUCH_ITEM "XXXXXXXXXXXXXXXX----XXX----XXXX-XX----XXX-XXX----"
#define PTRN_LAUNCHITEM "\x41\x55\x31\xc0\x49\x89\xf5\x41\x54\x49\x89\xd4\x55\x48\x89\xfd\x53\x48\x83\xec\x00\xe8\x00\x00\x00\x00\xc7\x40\x04\x00\x00\x00\x00\x48\x89\xc3\x48\x89\xe8\x48\x2b\x05\x00\x00\x00\x00"
#define MASK_LAUNCHITEM "XXXXXXXXXXXXXXXXXXXX-X----XXX----XXXXXXXXX----"
#define PTRN_DROP_ITEM "\x41\x54\x31\xc9\x31\xd2\x49\x89\xf4\x55\x53\x48\x89\xfb\x48\x83\xec\x00\xf3\x0f\x10\x4f\x00\x48\x8d\x6c\x24\x00\xc7\x44\x24\x20\x00\x00\x00\x00\xf3\x0f\x58\xc8\xf3\x0f\x10\x57\x00\x48\x8d\x7c\x24\x00"
#define MASK_DROP_ITEM "XXXXXXXXXXXXXXXXX-XXXX-XXXX-XXXX----XXXXXXXX-XXXX-"
#define PTRN_G_STARTKAMIKAZE "\x41\x55\x31\xc0\x41\x54\x55\x48\x89\xfd\x53\x48\x83\xec\x00\xe8\x00\x00\x00\x00\x4c\x8b\x25\x00\x00\x00\x00\xc7\x40\x04\x00\x00\x00\x00\x48\x89\xc3\x41\x8b\x44\x00\x24\x89\x83\x00\x00\x00\x00"
#define MASK_G_STARTKAMIKAZE "XXXXXXXXXXXXXX-X----XXX----XXX----XXXXXX-XXX----"
#define PTRN_G_FREEENTITY "\x48\x8b\x05\x00\x00\x00\x00\x53\x48\x89\xfb\x48\x8b\x00\xff\x90\x00\x00\x00\x00\x8b\x83\x00\x00\x00\x00\x85\xc0\x74\x00\x5b\xc3"
#define MASK_G_FREEENTITY "XXX----XXXXXXXXX----XX----XXX-XX"

// Functions without patterns.
#define OFFSET_RELP_CMD_ARGC ((pint)SV_Map_f + 0x81)

// qagame structs and global varaibles.
#define OFFSET_RELP_VM_CALL_TABLE		((pint)qagame_dllentry + 0x3)
#define OFFSET_RELP_G_ENTITIES          ((pint)G_RunFrame + 0x11B)
#define OFFSET_RELP_ADMINCOMMANDS       ((pint)CheckPrivileges + 0x56)
#define OFFSET_RELP_LEVEL       		((pint)G_InitGame + 0x4A1)

// Structs.
#define OFFSET_PP_SVS     ((pint)SV_Shutdown + 0xAC)
#define OFFSET_RELP_SV     ((pint)SV_SetConfigstring + 0xB4)

// VM_Call table offsets.
#define RELOFFSET_VM_CALL_INITGAME   0x18
#define RELOFFSET_VM_CALL_RUNFRAME   0x8

#elif defined(__i386) || defined(_M_IX86)

// Functions
#define OFFSET_RELP_CMD_ARGC ((pint)SV_Map_f + 0x8A)

// qagame structs and global varaibles.
#define OFFSET_RELP_VM_CALL_TABLE		((pint)qagame_dllentry + 0x11)
#define OFFSET_RELP_G_ENTITIES       ((pint)G_RunFrame + 0x143)

// Structs.
#define OFFSET_PP_SVS     ((pint)SV_Shutdown + 0xA0)

// VM_Call table offsets.
#define RELOFFSET_VM_CALL_INITGAME   0xC
#define RELOFFSET_VM_CALL_RUNFRAME   0x4

// Generated by minfuncfind32.
#define PTRN_COM_PRINTF "\x57\xba\x00\x00\x00\x00\x56\xb9\x00\x00\x00\x00\x53\x81\xec\x00\x00\x00\x00\x65\xa1\x00\x00\x00\x00\x89\x84\x24\x00\x00\x00\x00\x31\xc0\x8d\x84\x24\x00\x00\x00\x00\x89\x44\x24\x00"
#define MASK_COM_PRINTF "XX----XX----XXX----XX----XXX----XXXXX----XXX-"
#define PTRN_CMD_ADDCOMMAND "\x56\x53\x83\xec\x00\x8b\x1d\x00\x00\x00\x00\x8b\x74\x24\x00\x85\xdb\x75\x00\xeb\x00\x8d\x76\x00\x8b\x1b\x85\xdb\x74\x00\x8b\x43\x00\x89\x34\x24\x89\x44\x24\x00\xe8\x00\x00\x00\x00\x85\xc0\x75\x00"
#define MASK_CMD_ADDCOMMAND "XXXX-XX----XXX-XXX-X-XX-XXXXX-XX-XXXXXX-X----XXX-"
#define PTRN_CMD_ARGS "\x57\x56\x53\x83\xec\x00\x8b\x3d\x00\x00\x00\x00\xc6\x05\x60\x6a\x2a\x08\x00\x83\xff\x00\x7e\x00\x8d\x5f\x00\xbe\x00\x00\x00\x00\xba\x00\x00\x00\x00\x8b\x0a\x83\xc2\x00\x8d\x81\x00\x00\x00\x00\xf7\xd1"
#define MASK_CMD_ARGS "XXXXX-XX----XXXXXX-XX-X-XX-X----X----XXXX-XX----XX"
#define PTRN_CMD_ARGV "\x8b\x54\x24\x00\xb8\x00\x00\x00\x00\x3b\x15\x00\x00\x00\x00\x73\x00\x8b\x04\x95\x00\x00\x00\x00\xc3"
#define MASK_CMD_ARGV "XXX-X----XX----X-XXX----X"
#define PTRN_CMD_ARGC "\xa1\x00\x00\x00\x00\xc3"
#define MASK_CMD_ARGC "X----X"
#define PTRN_CMD_EXECUTESTRING "\x57\x56\x53\x83\xec\x00\x8b\x7c\x24\x00\x89\x3c\x24\xe8\x00\x00\x00\x00\xa1\x00\x00\x00\x00\x85\xc0\x0f\x84\x00\x00\x00\x00\x8b\x1d\x00\x00\x00\x00\xbe\x00\x00\x00\x00\x85\xdb\x75\x00\xeb\x00\x89\xde"
#define MASK_CMD_EXECUTESTRING "XXXXX-XXX-XXXX----X----XXXX----XX----X----XXX-X-XX"
#define PTRN_CBUF_EXECUTETEXT "\x8b\x44\x24\x00\x8b\x54\x24\x00\x83\xf8\x00\x74\x00\x83\xf8\x00\x74\x00\x85\xc0\x74\x00\xb8\x00\x00\x00\x00\x89\x44\x24\x00\x31\xc0\x89\x44\x24\x00\xe9\x00\x00\x00\x00\x8d\xb6\x00\x00\x00\x00\x85\xd2"
#define MASK_CBUF_EXECUTETEXT "XXX-XXX-XX-X-XX-X-XXX-X----XXX-XXXXX-X----XX----XX"
#define PTRN_CVAR_FINDVAR "\x56\x53\x83\xec\x00\x8b\x74\x24\x00\x89\xf0\xe8\x00\x00\x00\x00\x8b\x1c\x85\x00\x00\x00\x00\x85\xdb\x75\x00\xeb\x00\x8d\x76\x00\x8b\x5b\x00\x85\xdb\x74\x00\x8b\x13\x89\x34\x24\x89\x54\x24\x00"
#define MASK_CVAR_FINDVAR "XXXX-XXX-XXX----XXX----XXX-X-XX-XX-XXX-XXXXXXXX-"
#define PTRN_CVAR_GET "\x83\xec\x00\x89\x74\x24\x00\x8b\x74\x24\x00\x89\x7c\x24\x00\x8b\x7c\x24\x00\x89\x5c\x24\x00\x89\x6c\x24\x00\x85\xf6\x0f\x84\x00\x00\x00\x00\x85\xff\x0f\x84\x00\x00\x00\x00\x89\xf0\xe8\x00\x00\x00\x00"
#define MASK_CVAR_GET "XX-XXX-XXX-XXX-XXX-XXX-XXX-XXXX----XXXX----XXX----"
#define PTRN_CVAR_GETLIMIT "\x55\x31\xc0\x57\x56\x53\x83\xec\x00\x89\x44\x24\x00\x8b\x44\x24\x00\x8b\x5c\x24\x00\x8b\x7c\x24\x00\x8b\x74\x24\x00\x89\x04\x24\xe8\x00\x00\x00\x00\x31\xc0\x89\x44\x24\x00\x89\x3c\x24\xd9\x5c\x24\x00"
#define MASK_CVAR_GETLIMIT "XXXXXXXX-XXX-XXX-XXX-XXX-XXX-XXXX----XXXXX-XXXXXX-"
#define PTRN_SV_ADDOPERATORCOMMANDS "\x83\xec\x00\xb8\x00\x00\x00\x00\x89\x44\x24\x00\xc7\x04\x24\x00\x00\x00\x00\xe8\x00\x00\x00\x00\xb8\x00\x00\x00\x00\x89\x44\x24\x00\xc7\x04\x24\x00\x00\x00\x00\xe8\x00\x00\x00\x00\xb8\x00\x00\x00\x00"
#define MASK_SV_ADDOPERATORCOMMANDS "XX-X----XXX-XXX----X----X----XXX-XXX----X----X----"
#define PTRN_SV_SENDSERVERCOMMAND "\x81\xec\x00\x00\x00\x00\x65\xa1\x00\x00\x00\x00\x89\x84\x24\x00\x00\x00\x00\x31\xc0\x8d\x84\x24\x00\x00\x00\x00\x89\x44\x24\x00\x8b\x84\x24\x00\x00\x00\x00\x89\xac\x24\x00\x00\x00\x00\x8d\x6c\x24\x00"
#define MASK_SV_SENDSERVERCOMMAND "XX----XX----XXX----XXXXX----XXX-XXX----XXX----XXX-"
#define PTRN_SV_EXECUTECLIENTCOMMAND "\x55\x57\x56\xbe\x00\x00\x00\x00\x53\x83\xec\x00\x8b\x44\x24\x00\x8b\x7c\x24\x00\x8b\x6c\x24\x00\x89\x04\x24\xe8\x00\x00\x00\x00\x8b\x1d\x00\x00\x00\x00\x85\xdb\x75\x00\xeb\x00\x8d\x74\x26\x00"
#define MASK_SV_EXECUTECLIENTCOMMAND "XXXX----XXX-XXX-XXX-XXX-XXXX----XX----XXX-X-XXX-"
#define PTRN_SV_SHUTDOWN "\x53\x83\xec\x00\xa1\x00\x00\x00\x00\x8b\x5c\x24\x00\x85\xc0\x74\x00\x8b\x40\x00\x85\xc0\x75\x00\x83\xc4\x00\x5b\xc3"
#define MASK_SV_SHUTDOWN "XXX-X----XXX-XXX-XX-XXX-XX-XX"
#define PTRN_SV_CLIENTENTERWORLD "\x83\xec\x00\x89\x5c\x24\x00\x8b\x5c\x24\x00\xc7\x04\x24\x00\x00\x00\x00\x89\x74\x24\x00\x89\x7c\x24\x00\x8b\x7c\x24\x00\x8d\x83\x00\x00\x00\x00\x89\x44\x24\x00\xe8\x00\x00\x00\x00\x89\xd8"
#define MASK_SV_CLIENTENTERWORLD "XX-XXX-XXX-XXX----XXX-XXX-XXX-XX----XXX-X----XX"
#define PTRN_SV_CLIENTTHINK "\x83\xec\x00\x31\xc0\x8b\x54\x24\x00\x89\x1c\x24\x8b\x5c\x24\x00\x89\x74\x24\x00\x8d\x8a\x00\x00\x00\x00\x8b\x34\x03\x89\x34\x01\x83\xc0\x00\x83\xf8\x00\x72\x00\x83\x3a\x00\x74\x00\x8b\x1c\x24"
#define MASK_SV_CLIENTTHINK "XX-XXXXX-XXXXXX-XXX-XX----XXXXXXXX-XX-X-XX-X-XXX"
#define PTRN_SV_LINKENTITY "\x3c\x00\x54\x84\x28\xd9\x5c\x84\x00\x8d\x44\x24\x00\x89\x44\x24\x00\x8d\x44\x24\x00\x89\x3c\x24\x89\x44\x24\x00\xe8\x00\x00\x00\x00\x89\x3c\x24\x89\x43\x00\x8d\x44\x24\x00\x89\x44\x24\x00"
#define MASK_SV_LINKENTITY "X-XXXXXX-XXX-XXX-XXX-XXXXXX-X----XXXXX-XXX-XXX-"
#define PTRN_SV_MAP_F "\x55\x57\x56\x53\x81\xec\x00\x00\x00\x00\xc7\x04\x24\x00\x00\x00\x00\x8d\x9c\x24\x00\x00\x00\x00\x65\xa1\x00\x00\x00\x00\x89\x84\x24\x00\x00\x00\x00\x31\xc0\x8d\x74\x24\x00\xe8\x00\x00\x00\x00"
#define MASK_SV_MAP_F "XXXXXX----XXX----XXX----XX----XXX----XXXXX-X----"
#define PTRN_FS_STARTUP "\x55\xbd\x00\x00\x00\x00\x57\x31\xff\x56\xbe\x00\x00\x00\x00\x53\x31\xdb\x83\xec\x00\xc7\x04\x24\x00\x00\x00\x00\xe8\x00\x00\x00\x00\x31\xc0\x31\xd2\xa3\x00\x00\x00\x00\x31\xc0\xa3\x00\x00\x00\x00"
#define MASK_FS_STARTUP "XX----XXXXX----XXXXX-XXX----X----XXXXX----XXX----"
#define PTRN_SYS_SETMODULEOFFSET "\x83\xec\x00\x89\x5c\x24\x00\x8b\x5c\x24\x00\x89\x6c\x24\x00\x8b\x6c\x24\x00\xc7\x04\x24\x00\x00\x00\x00\x89\x74\x24\x00\x89\x5c\x24\x00\x89\xde\x89\x6c\x24\x00\x89\x7c\x24\x00\xbf\x00\x00\x00\x00"
#define MASK_SYS_SETMODULEOFFSET "XX-XXX-XXX-XXX-XXX-XXX----XXX-XXX-XXXXX-XXX-X----"

// Generated by minfuncfind32.
#define PTRN_G_RUNFRAME "\x55\x57\x56\x53\xe8\x00\x00\x00\x00\x81\xc3\x00\x00\x00\x00\x83\xec\x00\x8b\xbb\x00\x00\x00\x00\x85\xff\x74\x00\x83\xc4\x00\x5b\x5e\x5f\x5d\xc3"
#define MASK_G_RUNFRAME "XXXXX----XX----XX-XX----XXX-XX-XXXXX"
#define PTRN_G_ADDEVENT "\x83\xec\x00\x89\x74\x24\x00\x8b\x74\x24\x00\x89\x5c\x24\x00\x8b\x54\x24\x00\xe8\x00\x00\x00\x00\x81\xc3\x00\x00\x00\x00\x85\xf6\x74\x00\x8b\x8a\x00\x00\x00\x00\x85\xc9\x74\x00\x8b\x81\x00\x00\x00\x00"
#define MASK_G_ADDEVENT "XX-XXX-XXX-XXX-XXX-X----XX----XXX-XX----XXX-XX----"
#define PTRN_G_INITGAME "\x81\xec\x00\x00\x00\x00\xb9\x00\x00\x00\x00\x89\x9c\x24\x00\x00\x00\x00\xe8\x00\x00\x00\x00\x81\xc3\x00\x00\x00\x00\x89\xb4\x24\x00\x00\x00\x00\x65\xa1\x00\x00\x00\x00\x89\x84\x24\x00\x00\x00\x00"
#define MASK_G_INITGAME "XX----X----XXX----X----XX----XXX----XX----XXX----"

#endif



#endif /* PATTERNS_H */
