/*
Copyright (C) 1997-2001 Id Software, Inc.
Copyright (C) 2015 Mino <mino@minomino.org>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*
 * Mino: A lot of this is from Q3 sources, but obviously the structs aren't
 * exactly the same, so there's a good number of modifications to make it
 * fit QL. The end of the file has a bunch of stuff I added. Might want
 * to refactor it. TODO.
*/

#ifndef QUAKE_COMMON_H
#define QUAKE_COMMON_H

#include <stdint.h>

#include "patterns.h"
#include "common.h"

#define	CS_SCORES1				6
#define	CS_SCORES2				7
#define CS_VOTE_TIME			8
#define CS_VOTE_STRING			9
#define	CS_VOTE_YES				10
#define	CS_VOTE_NO				11
#define CS_ITEMS          15

#define MAX_CLIENTS 64
#define MAX_CHALLENGES  1024
#define MAX_MSGLEN  16384
#define MAX_PS_EVENTS   2
#define MAX_MAP_AREA_BYTES  32  // bit vector of area visibility
#define MAX_INFO_STRING 1024
#define MAX_RELIABLE_COMMANDS   64  // max string commands buffered for restransmit
#define MAX_STRING_CHARS    1024    // max length of a string passed to Cmd_TokenizeString
#define MAX_NAME_LENGTH 32  // max length of a client name
#define MAX_QPATH   64  // max length of a quake game pathname
#define MAX_DOWNLOAD_WINDOW 8   // max of eight download frames
#define MAX_NETNAME			36
#define PACKET_BACKUP   32  // number of old messages that must be kept on client and
                            // server for delta comrpession and ping estimation
#define	PACKET_MASK	(PACKET_BACKUP-1)
#define MAX_ENT_CLUSTERS    16
#define MAX_MODELS  256 // these are sent over the net as 8 bits
#define MAX_CONFIGSTRINGS   1024
#define GENTITYNUM_BITS     10      // don't need to send any more
#define MAX_GENTITIES       (1<<GENTITYNUM_BITS)
#define MAX_ITEM_MODELS 4
#define MAX_SPAWN_VARS 64
#define MAX_SPAWN_VARS_CHARS 4096
#define BODY_QUEUE_SIZE 8

// bit field limits
#define MAX_STATS               16
#define MAX_PERSISTANT          16
#define MAX_POWERUPS            16
#define MAX_WEAPONS             16

// Button flags
#define	BUTTON_ATTACK		1
#define	BUTTON_TALK			2			// displays talk balloon and disables actions
#define	BUTTON_USE_HOLDABLE	4			// Mino: +button2
#define	BUTTON_GESTURE		8			// Mino: +button3
#define	BUTTON_WALKING		16
// Block of unused button flags, or at least flags I couldn't trigger.
// Q3 used them for bot commands, so probably unused in QL.
#define BUTTON_UNUSED1		32
#define	BUTTON_UNUSED2		64
#define BUTTON_UNUSED3		128
#define BUTTON_UNUSED4		256
#define BUTTON_UNUSED5		512
#define BUTTON_UNUSED6		1024
#define	BUTTON_UPMOVE		2048  // Mino: Not in Q3. I'm guessing it's for cg_autohop.
#define	BUTTON_ANY			4096  // any key whatsoever
#define BUTTON_IS_ACTIVE 	65536 // Mino: No idea what it is, but it goes off after a while of being
								  //       AFK, then goes on after being active for a while.

// eflags
#define	EF_DEAD				0x00000001		// don't draw a foe marker over players with EF_DEAD
#define EF_TICKING			0x00000002		// used to make players play the prox mine ticking sound
#define	EF_TELEPORT_BIT		0x00000004		// toggled every time the origin abruptly changes
#define	EF_AWARD_EXCELLENT	0x00000008		// draw an excellent sprite
#define EF_PLAYER_EVENT		0x00000010
#define	EF_BOUNCE			0x00000010		// for missiles
#define	EF_BOUNCE_HALF		0x00000020		// for missiles
#define	EF_AWARD_GAUNTLET	0x00000040		// draw a gauntlet sprite
#define	EF_NODRAW			0x00000080		// may have an event, but no model (unspawned items)
#define	EF_FIRING			0x00000100		// for lightning gun
#define	EF_KAMIKAZE			0x00000200
#define	EF_MOVER_STOP		0x00000400		// will push otherwise
#define EF_AWARD_CAP		0x00000800		// draw the capture sprite
#define	EF_TALK				0x00001000		// draw a talk balloon
#define	EF_CONNECTION		0x00002000		// draw a connection trouble sprite
#define	EF_VOTED			0x00004000		// already cast a vote
#define	EF_AWARD_IMPRESSIVE	0x00008000		// draw an impressive sprite
#define	EF_AWARD_DEFEND		0x00010000		// draw a defend sprite
#define	EF_AWARD_ASSIST		0x00020000		// draw a assist sprite
#define EF_AWARD_DENIED		0x00040000		// denied
#define EF_TEAMVOTED		0x00080000		// already cast a team vote

#define FL_DROPPED_ITEM 0x00001000

#define DAMAGE_NO_PROTECTION 0x00000008

typedef enum {qfalse, qtrue} qboolean;
typedef unsigned char byte;

typedef struct gentity_s gentity_t;
typedef struct gclient_s gclient_t;

typedef float vec_t;
typedef vec_t vec2_t[2];
typedef vec_t vec3_t[3];
typedef vec_t vec4_t[4];
typedef vec_t vec5_t[5];

typedef int fileHandle_t;

// The permission levels used by QL's admin commands.
typedef enum {
    PRIV_BANNED = 0xFFFFFFFF,
    PRIV_NONE = 0x0,
    PRIV_MOD = 0x1,
    PRIV_ADMIN = 0x2,
    PRIV_ROOT = 0x3,
} privileges_t;

// Vote type. As opposed to in Q3, votes are counted every frame.
typedef enum {
	VOTE_NONE,
	VOTE_PENDING,
	VOTE_YES,
	VOTE_NO,
	VOTE_FORCE_PASS,
	VOTE_FORCE_FAIL,
    VOTE_EXPIRED
} voteState_t;

typedef enum {
    CS_FREE,        // can be reused for a new connection
    CS_ZOMBIE,      // client has been disconnected, but don't reuse
                    // connection for a couple seconds
    CS_CONNECTED,   // has been assigned to a client_t, but no gamestate yet
    CS_PRIMED,      // gamestate has been sent, but client hasn't sent a usercmd
    CS_ACTIVE       // client is fully in game
} clientState_t;

typedef enum {
    PREGAME = 0x0,
    ROUND_WARMUP = 0x1,
    ROUND_SHUFFLE = 0x2,
    ROUND_BEGUN = 0x3,
    ROUND_OVER = 0x4,
    POSTGAME = 0x5,
} roundStateState_t;

typedef enum {
  STAT_HEALTH,
  STAT_HOLDABLE_ITEM,
  STAT_RUNE,
  STAT_WEAPONS,
  STAT_ARMOR,
  STAT_BSKILL,
  STAT_CLIENTS_READY,
  STAT_MAX_HEALTH,
  STAT_SPINUP,
  STAT_FLIGHT_THRUST,
  STAT_MAX_FLIGHT_FUEL,
  STAT_CUR_FLIGHT_FUEL,
  STAT_FLIGHT_REFUEL,
  STAT_QUADKILLS,
  STAT_ARMORTYPE,
  STAT_KEY,
  STAT_OTHER_HEALTH,
  STAT_OTHER_ARMOR,
} statIndex_t;

typedef enum {
	GAME_INIT,	// ( int levelTime, int randomSeed, int restart );
	// init and shutdown will be called every single level
	// The game should call G_GET_ENTITY_TOKEN to parse through all the
	// entity configuration text and spawn gentities.

	GAME_SHUTDOWN,	// (void);

	GAME_CLIENT_CONNECT,	// ( int clientNum, qboolean firstTime, qboolean isBot );
	// return NULL if the client is allowed to connect, otherwise return
	// a text string with the reason for denial

	GAME_CLIENT_BEGIN,				// ( int clientNum );

	GAME_CLIENT_USERINFO_CHANGED,	// ( int clientNum );

	GAME_CLIENT_DISCONNECT,			// ( int clientNum );

	GAME_CLIENT_COMMAND,			// ( int clientNum );

	GAME_CLIENT_THINK,				// ( int clientNum );

	GAME_RUN_FRAME,					// ( int levelTime );

	GAME_CONSOLE_COMMAND,			// ( void );
	// ConsoleCommand will be called when a command has been issued
	// that is not recognized as a builtin function.
	// The game can issue trap_argc() / trap_argv() commands to get the command
	// and parameters.  Return qfalse if the game doesn't recognize it as a command.

	BOTAI_START_FRAME				// ( int time );
} gameExport_t;

typedef enum {
  PM_NORMAL = 0x0,
  PM_NOCLIP = 0x1,
  PM_SPECTATOR = 0x2,
  PM_DEAD = 0x3,
  PM_FREEZE = 0x4,
  PM_INTERMISSION = 0x5,
  PM_TUTORIAL = 0x6,
} pmtype_t;

typedef enum {
    EV_NONE = 0x0,
    EV_FOOTSTEP = 0x1,
    EV_FOOTSTEP_METAL = 0x2,
    EV_FOOTSPLASH = 0x3,
    EV_FOOTWADE = 0x4,
    EV_SWIM = 0x5,
    EV_FALL_SHORT = 0x6,
    EV_FALL_MEDIUM = 0x7,
    EV_FALL_FAR = 0x8,
    EV_JUMP_PAD = 0x9,
    EV_JUMP = 0xA,
    EV_WATER_TOUCH = 0xB,
    EV_WATER_LEAVE = 0xC,
    EV_WATER_UNDER = 0xD,
    EV_WATER_CLEAR = 0xE,
    EV_ITEM_PICKUP = 0xF,
    EV_GLOBAL_ITEM_PICKUP = 0x10,
    EV_NOAMMO = 0x11,
    EV_CHANGE_WEAPON = 0x12,
    EV_DROP_WEAPON = 0x13,
    EV_FIRE_WEAPON = 0x14,
    EV_USE_ITEM0 = 0x15,
    EV_USE_ITEM1 = 0x16,
    EV_USE_ITEM2 = 0x17,
    EV_USE_ITEM3 = 0x18,
    EV_USE_ITEM4 = 0x19,
    EV_USE_ITEM5 = 0x1A,
    EV_USE_ITEM6 = 0x1B,
    EV_USE_ITEM7 = 0x1C,
    EV_USE_ITEM8 = 0x1D,
    EV_USE_ITEM9 = 0x1E,
    EV_USE_ITEM10 = 0x1F,
    EV_USE_ITEM11 = 0x20,
    EV_USE_ITEM12 = 0x21,
    EV_USE_ITEM13 = 0x22,
    EV_USE_ITEM14 = 0x23,
    EV_USE_ITEM15 = 0x24,
    EV_ITEM_RESPAWN = 0x25,
    EV_ITEM_POP = 0x26,
    EV_PLAYER_TELEPORT_IN = 0x27,
    EV_PLAYER_TELEPORT_OUT = 0x28,
    EV_GRENADE_BOUNCE = 0x29,
    EV_GENERAL_SOUND = 0x2A,
    EV_GLOBAL_SOUND = 0x2B,
    EV_GLOBAL_TEAM_SOUND = 0x2C,
    EV_BULLET_HIT_FLESH = 0x2D,
    EV_BULLET_HIT_WALL = 0x2E,
    EV_MISSILE_HIT = 0x2F,
    EV_MISSILE_MISS = 0x30,
    EV_MISSILE_MISS_METAL = 0x31,
    EV_RAILTRAIL = 0x32,
    EV_SHOTGUN = 0x33,
    EV_BULLET = 0x34,
    EV_PAIN = 0x35,
    EV_DEATH1 = 0x36,
    EV_DEATH2 = 0x37,
    EV_DEATH3 = 0x38,
    EV_DROWN = 0x39,
    EV_OBITUARY = 0x3A,
    EV_POWERUP_QUAD = 0x3B,
    EV_POWERUP_BATTLESUIT = 0x3C,
    EV_POWERUP_REGEN = 0x3D,
    EV_POWERUP_ARMORREGEN = 0x3E,
    EV_GIB_PLAYER = 0x3F,
    EV_SCOREPLUM = 0x40,
    EV_PROXIMITY_MINE_STICK = 0x41,
    EV_PROXIMITY_MINE_TRIGGER = 0x42,
    EV_KAMIKAZE = 0x43,
    EV_OBELISKEXPLODE = 0x44,
    EV_OBELISKPAIN = 0x45,
    EV_INVUL_IMPACT = 0x46,
    EV_JUICED = 0x47,
    EV_LIGHTNINGBOLT = 0x48,
    EV_DEBUG_LINE = 0x49,
    EV_TAUNT = 0x4A,
    EV_TAUNT_YES = 0x4B,
    EV_TAUNT_NO = 0x4C,
    EV_TAUNT_FOLLOWME = 0x4D,
    EV_TAUNT_GETFLAG = 0x4E,
    EV_TAUNT_GUARDBASE = 0x4F,
    EV_TAUNT_PATROL = 0x50,
    EV_FOOTSTEP_SNOW = 0x51,
    EV_FOOTSTEP_WOOD = 0x52,
    EV_ITEM_PICKUP_SPEC = 0x53,
    EV_OVERTIME = 0x54,
    EV_GAMEOVER = 0x55,
    EV_MISSILE_MISS_DMGTHROUGH = 0x56,
    EV_THAW_PLAYER = 0x57,
    EV_THAW_TICK = 0x58,
    EV_SHOTGUN_KILL = 0x59,
    EV_POI = 0x5A,
    EV_DEBUG_HITBOX = 0x5B,
    EV_LIGHTNING_DISCHARGE = 0x5C,
    EV_RACE_START = 0x5D,
    EV_RACE_CHECKPOINT = 0x5E,
    EV_RACE_FINISH = 0x5F,
    EV_DAMAGEPLUM = 0x60,
    EV_AWARD = 0x61,
    EV_INFECTED = 0x62,
    EV_NEW_HIGH_SCORE = 0x63,
    EV_NUM_ETYPES = 0x64,
} entity_event_t;

typedef enum {
	IT_BAD,
	IT_WEAPON,				// EFX: rotate + upscale + minlight
	IT_AMMO,				// EFX: rotate
	IT_ARMOR,				// EFX: rotate + minlight
	IT_HEALTH,				// EFX: static external sphere + rotating internal
	IT_POWERUP,				// instant on, timer based
							// EFX: rotate + external ring that rotates
	IT_HOLDABLE,			// single use, holdable item
							// EFX: rotate + bob
	IT_PERSISTANT_POWERUP,
	IT_TEAM
} itemType_t;

typedef enum {
    PW_NONE = 0x0,
    PW_SPAWNARMOR = 0x0,
    PW_REDFLAG = 0x1,
    PW_BLUEFLAG = 0x2,
    PW_NEUTRALFLAG = 0x3,
    PW_QUAD = 0x4,
    PW_BATTLESUIT = 0x5,
    PW_HASTE = 0x6,
    PW_INVIS = 0x7,
    PW_REGEN = 0x8,
    PW_FLIGHT = 0x9,
    PW_INVULNERABILITY = 0xA,
    NOTPW_SCOUT = 0xB,
    NOTPW_GUARD = 0xC,
    NOTPW_DOUBLER = 0xD,
    NOTPW_ARMORREGEN = 0xE,
    PW_FREEZE = 0xF,
    PW_NUM_POWERUPS = 0x10,
} powerup_t;

typedef enum {
    H_NONE = 0x0,
    H_MEGA = 0x1,
    H_LARGE = 0x2,
    H_MEDIUM = 0x3,
    H_SMALL = 0x4,
    H_NUM_HEALTHS = 0x5,
} healthPickup_t;

typedef enum {
    HI_NONE = 0x0,
    HI_TELEPORTER = 0x1,
    HI_MEDKIT = 0x2,
    HI_KAMIKAZE = 0x3,
    HI_PORTAL = 0x4,
    HI_INVULNERABILITY = 0x5,
    HI_FLIGHT = 0x6,
    HI_NUM_HOLDABLE = 0x7,
} holdable_t;

typedef enum {
    WP_NONE = 0x0,
    WP_GAUNTLET = 0x1,
    WP_MACHINEGUN = 0x2,
    WP_SHOTGUN = 0x3,
    WP_GRENADE_LAUNCHER = 0x4,
    WP_ROCKET_LAUNCHER = 0x5,
    WP_LIGHTNING = 0x6,
    WP_RAILGUN = 0x7,
    WP_PLASMAGUN = 0x8,
    WP_BFG = 0x9,
    WP_GRAPPLING_HOOK = 0xA,
    WP_NAILGUN = 0xB,
    WP_PROX_LAUNCHER = 0xC,
    WP_CHAINGUN = 0xD,
    WP_HMG = 0xE,
    WP_HANDS = 0xF,
    WP_NUM_WEAPONS = 0x10,
} weapon_t;

typedef enum {
  WEAPON_READY = 0x0,
  WEAPON_RAISING = 0x1,
  WEAPON_DROPPING = 0x2,
  WEAPON_FIRING = 0x3,
} weaponstate_t;

typedef enum {
    RUNE_NONE = 0x0,
    RUNE_SCOUT = 0x1,
    RUNE_GUARD = 0x2,
    RUNE_DAMAGE = 0x3,
    RUNE_ARMORREGEN = 0x4,
    RUNE_MAX = 0x5,
} rune_t;

typedef enum {
	TEAM_BEGIN,		// Beginning a team game, spawn at base
	TEAM_ACTIVE		// Now actively playing
} playerTeamStateState_t;

typedef enum {
	TEAM_FREE,
	TEAM_RED,
	TEAM_BLUE,
	TEAM_SPECTATOR,

	TEAM_NUM_TEAMS
} team_t;

// https://github.com/brugal/wolfcamql/blob/73e2d707e5dd1fb0fc50d4ad9f00940909c4b3ec/code/game/bg_public.h#L1142-L1188
// means of death
typedef enum {
  MOD_UNKNOWN,
  MOD_SHOTGUN,
  MOD_GAUNTLET,
  MOD_MACHINEGUN,
  MOD_GRENADE,
  MOD_GRENADE_SPLASH,
  MOD_ROCKET,
  MOD_ROCKET_SPLASH,
  MOD_PLASMA,
  MOD_PLASMA_SPLASH,
  MOD_RAILGUN,
  MOD_LIGHTNING,
  MOD_BFG,
  MOD_BFG_SPLASH,
  MOD_WATER,
  MOD_SLIME,
  MOD_LAVA,
  MOD_CRUSH,
  MOD_TELEFRAG,
  MOD_FALLING,
  MOD_SUICIDE,
  MOD_TARGET_LASER,
  MOD_TRIGGER_HURT,
  MOD_NAIL,
  MOD_CHAINGUN,
  MOD_PROXIMITY_MINE,
  MOD_KAMIKAZE,
  MOD_JUICED,
  MOD_GRAPPLE,
  MOD_SWITCH_TEAMS,
  MOD_THAW,
  MOD_LIGHTNING_DISCHARGE,
  MOD_HMG,
  MOD_RAILGUN_HEADSHOT
} meansOfDeath_t;

typedef enum {
	SPECTATOR_NOT,
	SPECTATOR_FREE,
	SPECTATOR_FOLLOW,
	SPECTATOR_SCOREBOARD
} spectatorState_t;

typedef enum {
	CON_DISCONNECTED,
	CON_CONNECTING,
	CON_CONNECTED
} clientConnected_t;

// movers are things like doors, plats, buttons, etc
typedef enum {
	MOVER_POS1,
	MOVER_POS2,
	MOVER_1TO2,
	MOVER_2TO1
} moverState_t;

enum {
  PERS_ROUND_SCORE = 0x0,
  PERS_COMBOKILL_COUNT = 0x1,
  PERS_RAMPAGE_COUNT = 0x2,
  PERS_MIDAIR_COUNT = 0x3,
  PERS_REVENGE_COUNT = 0x4,
  PERS_PERFORATED_COUNT = 0x5,
  PERS_HEADSHOT_COUNT = 0x6,
  PERS_ACCURACY_COUNT = 0x7,
  PERS_QUADGOD_COUNT = 0x8,
  PERS_FIRSTFRAG_COUNT = 0x9,
  PERS_PERFECT_COUNT = 0xA,
};

enum cvar_flags {
    CVAR_ARCHIVE = 1,
    CVAR_USERINFO = 2,
    CVAR_SERVERINFO = 4,
    CVAR_SYSTEMINFO = 8,
    CVAR_INIT = 16,
    CVAR_LATCH = 32,
    CVAR_ROM = 64,
    CVAR_USER_CREATED = 128,
    CVAR_TEMP = 256,
    CVAR_CHEAT = 512,
    CVAR_NORESTART = 1024,
    CVAR_UNKOWN1 = 2048,
    CVAR_UNKOWN2 = 4096,
    CVAR_UNKOWN3 = 8192,
    CVAR_UNKOWN4 = 16384,
    CVAR_UNKOWN5 = 32768,
    CVAR_UNKOWN6 = 65536,
    CVAR_UNKOWN7 = 131072,
    CVAR_UNKOWN8 = 262144,
    CVAR_UNKOWN9 = 524288,
    CVAR_UNKOWN10 = 1048576
};

// paramters for command buffer stuffing
typedef enum {
	EXEC_NOW,			// don't return until completed, a VM should NEVER use this,
						// because some commands might cause the VM to be unloaded...
	EXEC_INSERT,		// insert at current position, but don't run yet
	EXEC_APPEND			// add to end of the command buffer (normal case)
} cbufExec_t;

// Mino: Quite different from Q3. Not sure on everything.
typedef struct cvar_s {
	char		*name;
	char		*string;
	char		*resetString;		// cvar_restart will reset to this value
	char		*latchedString;		// for CVAR_LATCH vars
    char        *defaultString;
    char		*minimumString;
    char		*maximumString;
	int			flags;
	qboolean	modified;
    uint8_t     _unknown2[4];
	int			modificationCount;	// incremented each time the cvar is changed
	float		value;				// atof( string )
	int			integer;			// atoi( string )
    uint8_t     _unknown3[8];
	struct cvar_s *next;
	struct cvar_s *hashNext;
} cvar_t;

typedef struct {
    qboolean    allowoverflow;  // if false, do a Com_Error
    qboolean    overflowed;     // set to true if the buffer size failed (with allowoverflow set)
    qboolean    oob;            // set to true if the buffer size failed (with allowoverflow set)
    byte    *data;
    int     maxsize;
    int     cursize;
    int     readcount;
    int     bit;                // for bitwise reads and writes
} msg_t;

typedef struct __attribute__((aligned(4))) usercmd_s {
  int serverTime;
  int angles[3];
  int buttons;
  byte weapon;
  byte weaponPrimary;
  byte fov;
  char forwardmove;
  char rightmove;
  char upmove;
} usercmd_t;

typedef enum {
    NS_CLIENT,
    NS_SERVER
} netsrc_t;

typedef enum {
    NA_BOT,
    NA_BAD,                 // an address lookup failed
    NA_LOOPBACK,
    NA_BROADCAST,
    NA_IP,
    NA_IPX,
    NA_BROADCAST_IPX
} netadrtype_t;

typedef enum {
    TR_STATIONARY,
    TR_INTERPOLATE,             // non-parametric, but interpolate between snapshots
    TR_LINEAR,
    TR_LINEAR_STOP,
    TR_SINE,                    // value = base + sin( time / duration ) * delta
    TR_GRAVITY
} trType_t;

typedef struct {
    netadrtype_t    type;

    byte    ip[4];
    byte    ipx[10];

    unsigned short  port;
} netadr_t;

typedef struct {
    netsrc_t    sock;

    int         dropped;            // between last packet and previous

    netadr_t    remoteAddress;
    int         qport;              // qport value to write when transmitting

    // sequencing variables
    int         incomingSequence;
    int         outgoingSequence;

    // incoming fragment assembly buffer
    int         fragmentSequence;
    int         fragmentLength; 
    byte        fragmentBuffer[MAX_MSGLEN];

    // outgoing fragment buffer
    // we need to space out the sending of large fragmented messages
    qboolean    unsentFragments;
    int         unsentFragmentStart;
    int         unsentLength;
    byte        unsentBuffer[MAX_MSGLEN];
} netchan_t;

typedef struct cplane_s {
  vec3_t normal;
  float dist;
  byte type;
  byte signbits;
  byte pad[2];
} cplane_t;

// a trace is returned when a box is swept through the world
typedef struct {
  qboolean allsolid;
  qboolean startsolid;
  float fraction;
  vec3_t endpos;
  cplane_t plane;
  int surfaceFlags;
  int contents;
  int entityNum;
} trace_t;

// playerState_t is a full superset of entityState_t as it is used by players,
// so if a playerState_t is transmitted, the entityState_t can be fully derived
// from it.
typedef struct playerState_s {
  int commandTime;
  int pm_type;
  int bobCycle;
  int pm_flags;
  int pm_time;
  vec3_t origin;
  vec3_t velocity;
  int weaponTime;
  int gravity;
  int speed;
  int delta_angles[3];
  int groundEntityNum;
  int legsTimer;
  int legsAnim;
  int torsoTimer;
  int torsoAnim;
  int movementDir;
  vec3_t grapplePoint;
  int eFlags;
  int eventSequence;
  int events[2];
  int eventParms[2];
  int externalEvent;
  int externalEventParm;
  int clientNum;
  int location;
  int weapon;
  int weaponPrimary;
  int weaponstate;
  int fov;
  vec3_t viewangles;
  int viewheight;
  int damageEvent;
  int damageYaw;
  int damagePitch;
  int damageCount;
  int stats[16];
  int persistant[16];
  int powerups[16];
  int ammo[16];
  int generic1;
  int loopSound;
  int jumppad_ent;
  int jumpTime;
  int doubleJumped;
  int crouchTime;
  int crouchSlideTime;
  char forwardmove;
  char rightmove;
  char upmove;
  int ping;
  int pmove_framecount;
  int jumppad_frame;
  int entityEventSequence;
  int freezetime;
  int thawtime;
  int thawClientNum_valid;
  int thawClientNum;
  int respawnTime;
  int localPersistant[16];
  int roundDamage;
  int roundShots;
  int roundHits;
} playerState_t;

typedef struct __attribute__((aligned(8))) {
  playerState_t *ps;
  usercmd_t cmd;
  int tracemask;
  int debugLevel;
  int noFootsteps;
  qboolean gauntletHit;
  int numtouch;
  int touchents[32];
  vec3_t mins;
  vec3_t maxs;
  int watertype;
  int waterlevel;
  float xyspeed;
  float stepHeight;
  int stepTime;
  void (*trace)(trace_t *, const vec_t *, const vec_t *, const vec_t *, const vec_t *, int, int);
  int (*pointcontents)(const vec_t *, int);
  qboolean hookEnemy;
} pmove_t;

typedef struct {
    int             areabytes;
    byte            areabits[MAX_MAP_AREA_BYTES];       // portalarea visibility bits
    playerState_t   ps;
    int             num_entities;
    int             first_entity;       // into the circular sv_packet_entities[]
                                        // the entities MUST be in increasing state number
                                        // order, otherwise the delta compression will fail
    int             messageSent;        // time the message was transmitted
    int             messageAcked;       // time the message was acked
    int             messageSize;        // used to rate drop packets
} clientSnapshot_t;

typedef struct netchan_buffer_s {
    msg_t           msg;
    byte            msgBuffer[MAX_MSGLEN];
    struct netchan_buffer_s *next;
} netchan_buffer_t;

typedef struct {
  trType_t trType;
  int trTime;
  int trDuration;
  vec3_t trBase;
  vec3_t trDelta;
  float gravity;
} trajectory_t;

typedef struct entityState_s {
  int number;
  int eType;
  int eFlags;
  trajectory_t pos;
  trajectory_t apos;
  int time;
  int time2;
  vec3_t origin;
  vec3_t origin2;
  vec3_t angles;
  vec3_t angles2;
  int otherEntityNum;
  int otherEntityNum2;
  int groundEntityNum;
  int constantLight;
  int loopSound;
  int modelindex;
  int modelindex2;
  int clientNum;
  int frame;
  int solid;
  int event;
  int eventParm;
  int powerups;
  int health;
  int armor;
  int weapon;
  int location;
  int legsAnim;
  int torsoAnim;
  int generic1;
  int jumpTime;
  int doubleJumped;
} entityState_t;

typedef struct {
  entityState_t s;
  qboolean linked;
  int linkcount;
  int svFlags;
  int singleClient;
  qboolean bmodel;
  vec3_t mins;
  vec3_t maxs;
  int contents;
  vec3_t absmin;
  vec3_t absmax;
  vec3_t currentOrigin;
  vec3_t currentAngles;
  int ownerNum;
} entityShared_t;

typedef struct {
    entityState_t   s;              // communicated by server to clients
    entityShared_t  r;              // shared by both the server system and game
} sharedEntity_t;

typedef struct client_s {
    clientState_t   state;
    char            userinfo[MAX_INFO_STRING];      // name, etc

    char            reliableCommands[MAX_RELIABLE_COMMANDS][MAX_STRING_CHARS];
    int             reliableSequence;       // last added reliable message, not necesarily sent or acknowledged yet
    int             reliableAcknowledge;    // last acknowledged reliable message
    int             reliableSent;           // last sent reliable message, not necesarily acknowledged yet
    int             messageAcknowledge;

    int             gamestateMessageNum;    // netchan->outgoingSequence of gamestate
    int             challenge;

    usercmd_t       lastUsercmd;
    int             lastMessageNum;     // for delta compression
    int             lastClientCommand;  // reliable client message sequence
    char            lastClientCommandString[MAX_STRING_CHARS];
    sharedEntity_t  *gentity;           // SV_GentityNum(clientnum)
    char            name[MAX_NAME_LENGTH];          // extracted from userinfo, high bits masked
    
    // Mino: I think everything above this is correct. Below is a mess.
    
    // downloading
    char            downloadName[MAX_QPATH]; // if not empty string, we are downloading
    fileHandle_t    download;           // file being downloaded
    int             downloadSize;       // total bytes (can't use EOF because of paks)
    int             downloadCount;      // bytes sent
    int             downloadClientBlock;    // last block we sent to the client, awaiting ack
    int             downloadCurrentBlock;   // current block number
    int             downloadXmitBlock;  // last block we xmited
    unsigned char   *downloadBlocks[MAX_DOWNLOAD_WINDOW];   // the buffers for the download blocks
    int             downloadBlockSize[MAX_DOWNLOAD_WINDOW];
    qboolean        downloadEOF;        // We have sent the EOF block
    int             downloadSendTime;   // time we last got an ack from the client

    int             deltaMessage;       // frame last client usercmd message
    int             nextReliableTime;   // svs.time when another reliable command will be allowed
    int             lastPacketTime;     // svs.time when packet was last received
    int             lastConnectTime;    // svs.time when connection started
    int             nextSnapshotTime;   // send another snapshot when svs.time >= nextSnapshotTime
    qboolean        rateDelayed;        // true if nextSnapshotTime was set based on rate instead of snapshotMsec
    int             timeoutCount;       // must timeout a few frames in a row so debugging doesn't break
    clientSnapshot_t    frames[PACKET_BACKUP];  // updates can be delta'd from here
    int             ping;
    int             rate;               // bytes / second
    int             snapshotMsec;       // requests a snapshot every snapshotMsec unless rate choked
    int             pureAuthentic;
    qboolean  gotCP; // TTimo - additional flag to distinguish between a bad pure checksum, and no cp command at all
    netchan_t       netchan;
    netchan_buffer_t *netchan_start_queue;
    netchan_buffer_t **netchan_end_queue;
    
    // Mino: Holy crap. A bunch of data was added. I have no idea where it actually goes,
    // but this will at least correct sizeof(client_t).
#if defined(__x86_64__) || defined(_M_X64)
    uint8_t         _unknown2[36808];
#elif defined(__i386) || defined(_M_IX86)
    uint8_t         _unknown2[36836]; // TODO: Outdated.
#endif

    // Mino: Woohoo! How nice of them to put the SteamID last.
    uint64_t        steam_id;
} client_t;

//
// SERVER
//

typedef struct {
    netadr_t    adr;
    int         challenge;
    int         time;               // time the last packet was sent to the autherize server
    int         pingTime;           // time the challenge response was sent to client
    int         firstTime;          // time the adr was first used, for authorize timeout checks
    qboolean    connected;
} challenge_t;

// this structure will be cleared only when the game dll changes
typedef struct {
    qboolean    initialized;                // sv_init has completed
    int         time;                       // will be strictly increasing across level changes
    int         snapFlagServerBit;          // ^= SNAPFLAG_SERVERCOUNT every SV_SpawnServer()
    client_t    *clients;                   // [sv_maxclients->integer];
    int         numSnapshotEntities;        // sv_maxclients->integer*PACKET_BACKUP*MAX_PACKET_ENTITIES
    int         nextSnapshotEntities;       // next snapshotEntities to use
    entityState_t   *snapshotEntities;      // [numSnapshotEntities]
    int         nextHeartbeatTime;
    challenge_t challenges[MAX_CHALLENGES]; // to prevent invalid IPs from connecting
    netadr_t    redirectAddress;            // for rcon return messages
    netadr_t    authorizeAddress;           // for rcon return messages
} serverStatic_t;

typedef struct svEntity_s {
    struct worldSector_s *worldSector;
    struct svEntity_s *nextEntityInWorldSector;
    
    entityState_t   baseline;       // for delta compression of initial sighting
    int         numClusters;        // if -1, use headnode instead
    int         clusternums[MAX_ENT_CLUSTERS];
    int         lastCluster;        // if all the clusters don't fit in clusternums
    int         areanum, areanum2;
    int         snapshotCounter;    // used to prevent double adding from portal views
} svEntity_t;

typedef struct worldSector_s {
    int     axis;       // -1 = leaf node
    float   dist;
    struct worldSector_s    *children[2];
    svEntity_t  *entities;
} worldSector_t;

typedef enum {
    SS_DEAD,            // no map loaded
    SS_LOADING,         // spawning level entities
    SS_GAME             // actively running
} serverState_t;

typedef struct {
    serverState_t   state;
    qboolean        restarting;         // if true, send configstring changes during SS_LOADING
    int             serverId;           // changes each server start
    int             restartedServerId;  // serverId before a map_restart
    int             checksumFeed;       // the feed key that we use to compute the pure checksum strings
    // https://zerowing.idsoftware.com/bugzilla/show_bug.cgi?id=475
    // the serverId associated with the current checksumFeed (always <= serverId)
    int       checksumFeedServerId; 
    int             snapshotCounter;    // incremented for each snapshot built
    int             timeResidual;       // <= 1000 / sv_frame->value
    int             nextFrameTime;      // when time > nextFrameTime, process world
    struct cmodel_s *models[MAX_MODELS];
    char            *configstrings[MAX_CONFIGSTRINGS];
    svEntity_t      svEntities[MAX_GENTITIES];

    char            *entityParsePoint;  // used during game VM init

    // the game virtual machine will update these on init and changes
    sharedEntity_t  *gentities;
    int             gentitySize;
    int             num_entities;       // current number, <= MAX_GENTITIES

    playerState_t   *gameClients;
    int             gameClientSize;     // will be > sizeof(playerState_t) due to game private data

    int             restartTime;
} server_t;

typedef struct {
  playerTeamStateState_t state;
  int captures;
  int basedefense;
  int carrierdefense;
  int flagrecovery;
  int fragcarrier;
  int assists;
  int flagruntime;
  int flagrunrelays;
  int lasthurtcarrier;
  int lastreturnedflag;
  int lastfraggedcarrier;
} playerTeamState_t;

typedef struct {
  unsigned int statId;
  int lastThinkTime;
  int teamJoinTime;
  int totalPlayTime;
  int serverRank;
  qboolean serverRankIsTied;
  int teamRank;
  qboolean teamRankIsTied;
  int numKills;
  int numDeaths;
  int numSuicides;
  int numTeamKills;
  int numTeamKilled;
  int numWeaponKills[16];
  int numWeaponDeaths[16];
  int shotsFired[16];
  int shotsHit[16];
  int damageDealt[16];
  int damageTaken[16];
  int powerups[16];
  int holdablePickups[7];
  int weaponPickups[16];
  int weaponUsageTime[16];
  int numCaptures;
  int numAssists;
  int numDefends;
  int numHolyShits;
  int totalDamageDealt;
  int totalDamageTaken;
  int previousHealth;
  int previousArmor;
  int numAmmoPickups;
  int numFirstMegaHealthPickups;
  int numMegaHealthPickups;
  int megaHealthPickupTime;
  int numHealthPickups;
  int numFirstRedArmorPickups;
  int numRedArmorPickups;
  int redArmorPickupTime;
  int numFirstYellowArmorPickups;
  int numYellowArmorPickups;
  int yellowArmorPickupTime;
  int numFirstGreenArmorPickups;
  int numGreenArmorPickups;
  int greenArmorPickupTime;
  int numQuadDamagePickups;
  int numQuadDamageKills;
  int numBattleSuitPickups;
  int numRegenerationPickups;
  int numHastePickups;
  int numInvisibilityPickups;
  int numRedFlagPickups;
  int numBlueFlagPickups;
  int numNeutralFlagPickups;
  int numMedkitPickups;
  int numArmorPickups;
  int numDenials;
  int killStreak;
  int maxKillStreak;
  int xp;
  int domThreeFlagsTime;
  int numMidairShotgunKills;
} expandedStatObj_t;

// client data that stays across multiple respawns, but is cleared
// on each level change or team change at ClientBegin()
typedef struct __attribute__((aligned(8))) {
  clientConnected_t connected;
  usercmd_t cmd;
  qboolean localClient;
  qboolean initialSpawn;
  qboolean predictItemPickup;
  char netname[40];
  char country[24];
  uint64_t steamId;
  int maxHealth;
  int voteCount;
  voteState_t voteState;
  int complaints;
  int complaintClient;
  int complaintEndTime;
  int damageFromTeammates;
  int damageToTeammates;
  qboolean ready;
  int autoaction;
  int timeouts;
  int enterTime;
  playerTeamState_t teamState;
  int damageResidual;
  int inactivityTime;
  int inactivityWarning;
  int lastUserinfoUpdate;
  int userInfoFloodInfractions;
  int lastMapVoteTime;
  int lastMapVoteIndex;
} clientPersistant_t;

// client data that stays across multiple levels or tournament restarts
// this is achieved by writing all the data to cvar strings at game shutdown
// time and reading them back at connection time.  Anything added here
// MUST be dealt with in G_InitSessionData() / G_ReadSessionData() / G_WriteSessionData()
typedef struct {
  team_t sessionTeam;
  int spectatorTime;
  spectatorState_t spectatorState;
  int spectatorClient;
  int weaponPrimary;
  int wins;
  int losses;
  qboolean teamLeader;
  privileges_t privileges;
  int specOnly;
  int playQueue;
  qboolean updatePlayQueue;
  int muted;
  int prevScore;
} clientSession_t;

typedef struct gitem_s {
  char *classname;
  const char *pickup_sound;
  const char *world_model[4];
  const char *premium_model[4];
  const char *icon;
  const char *pickup_name;
  int quantity;
  itemType_t giType;
  int giTag;
  qboolean itemTimer;
  unsigned int maskGametypeRenderSkip;
  unsigned int maskGametypeForceSpawn;
} gitem_t;

typedef enum {
  ET_GENERAL,
  ET_PLAYER,
  ET_ITEM,
  ET_MISSILE,
  ET_MOVER,
  ET_BEAM,
  ET_PORTAL,
  ET_SPEAKER,
  ET_PUSH_TRIGGER,
  ET_TELEPORT_TRIGGER,
  ET_INVISIBLE,
  ET_GRAPPLE,       // grapple hooked on wall
  ET_TEAM,

  ET_EVENTS       // any of the EV_* events can be added freestanding
              // by setting eType to ET_EVENTS + eventNum
              // this avoids having to set eFlags and eventNum
} entityType_t;

struct gclient_s;

struct gentity_s {
  entityState_t s;
  entityShared_t r;
  struct gclient_s *client;
  qboolean inuse;
  char *classname;
  int spawnflags;
  qboolean neverFree;
  int flags;
  char *model;
  char *model2;
  int freetime;
  int eventTime;
  qboolean freeAfterEvent;
  qboolean unlinkAfterEvent;
  qboolean physicsObject;
  float physicsBounce;
  int clipmask;
  moverState_t moverState;
  int soundPos1;
  int sound1to2;
  int sound2to1;
  int soundPos2;
  int soundLoop;
  gentity_t *parent;
  gentity_t *nextTrain;
  gentity_t *prevTrain;
  vec3_t pos1;
  vec3_t pos2;
  char *message;
  char *cvar;
  char *tourPointTarget;
  char *tourPointTargetName;
  char *noise;
  int timestamp;
  float angle;
  char *target;
  char *targetname;
  char *targetShaderName;
  char *targetShaderNewName;
  gentity_t *target_ent;
  float speed;
  vec3_t movedir;
  int nextthink;
  void (*think)(gentity_t *);
  void (*framethink)(gentity_t *);
  void (*reached)(gentity_t *);
  void (*blocked)(gentity_t *, gentity_t *);
  void (*touch)(gentity_t *, gentity_t *);
  void (*use)(gentity_t *, gentity_t *, gentity_t *);
  void (*pain)(gentity_t *, gentity_t *, int);
  void (*die)(gentity_t *, gentity_t *, gentity_t *, int, int);
  int pain_debounce_time;
  int fly_sound_debounce_time;
  int health;
  qboolean takedamage;
  int damage;
  int damageFactor;
  int splashDamage;
  int splashRadius;
  int methodOfDeath;
  int splashMethodOfDeath;
  int count;
  gentity_t *enemy;
  gentity_t *activator;
  const char *team;
  gentity_t *teammaster;
  gentity_t *teamchain;
  int kamikazeTime;
  int kamikazeShockTime;
  int watertype;
  int waterlevel;
  int noise_index;
  int bouncecount;
  float wait;
  float random;
  int spawnTime;
  const gitem_t *item;
  int pickupCount;
};

typedef struct {
  qboolean racingActive;
  int startTime;
  int lastTime;
  int best_race[64];
  int current_race[64];
  int currentCheckPoint;
  qboolean weaponUsed;
  gentity_t *nextRacePoint;
  gentity_t *nextRacePoint2;
} raceInfo_t;

// this structure is cleared on each ClientSpawn(),
// except for 'client->pers' and 'client->sess'
struct __attribute__((aligned(8))) gclient_s {
  playerState_t ps;
  clientPersistant_t pers;
  clientSession_t sess;
  qboolean noclip;
  int lastCmdTime;
  int buttons;
  int oldbuttons;
  int damage_armor;
  int damage_blood;
  vec3_t damage_from;
  qboolean damage_fromWorld;
  int impressiveCount;
  int accuracyCount;
  int accuracy_shots;
  int accuracy_hits;
  int lastClientKilled;
  int lastKilledClient;
  int lastHurtClient[2];
  int lastHurtMod[2];
  int lastHurtTime[2];
  int lastKillTime;
  int lastGibTime;
  int rampageCounter;
  int revengeCounter[64];
  int respawnTime;
  int rewardTime;
  int airOutTime;
  qboolean fireHeld;
  gentity_t *hook;
  int switchTeamTime;
  int timeResidual;
  int timeResidualScout;
  int timeResidualArmor;
  int timeResidualHealth;
  int timeResidualPingPOI;
  int timeResidualSpecInfo;
  qboolean healthRegenActive;
  qboolean armorRegenActive;
  gentity_t *persistantPowerup;
  int portalID;
  int ammoTimes[16];
  int invulnerabilityTime;
  expandedStatObj_t expandedStats;
  int ignoreChatsTime;
  int lastUserCmdTime;
  qboolean freezePlayer;
  int deferredSpawnTime;
  int deferredSpawnCount;
  raceInfo_t race;
  int shotgunDmg[64];
  int round_shots;
  int round_hits;
  int round_damage;
  qboolean queuedSpectatorFollow;
  int queuedSpectatorClient;
};

typedef struct {
  roundStateState_t eCurrent;
  roundStateState_t eNext;
  int tNext;
  int startTime;
  int turn;
  int round;
  team_t prevRoundWinningTeam;
  qboolean touch;
  qboolean capture;
} roundState_t;

typedef struct __attribute__((aligned(8))) {
  struct gclient_s *clients;
  struct gentity_s *gentities;
  int gentitySize;
  int num_entities;
  int warmupTime;
  fileHandle_t logFile;
  int maxclients;
  int time;
  int frametime;
  int startTime;
  int teamScores[4];
  int nextTeamInfoTime;
  qboolean newSession;
  qboolean restarted;
  qboolean shufflePending;
  int shuffleReadyTime;
  int numConnectedClients;
  int numNonSpectatorClients;
  int numPlayingClients;
  int numReadyClients;
  int numReadyHumans;
  int numStandardClients;
  int sortedClients[64];
  int follow1;
  int follow2;
  int snd_fry;
  int warmupModificationCount;
  char voteString[1024];
  char voteDisplayString[1024];
  int voteExecuteTime;
  int voteTime;
  int voteYes;
  int voteNo;
  int pendingVoteCaller;
  qboolean spawning;
  int numSpawnVars;
  char *spawnVars[64][2];
  int numSpawnVarChars;
  char spawnVarChars[4096];
  int intermissionQueued;
  int intermissionTime;
  qboolean readyToExit;
  qboolean votingEnded;
  int exitTime;
  vec3_t intermission_origin;
  vec3_t intermission_angle;
  qboolean locationLinked;
  gentity_t *locationHead;
  int timePauseBegin;
  int timeOvertime;
  int timeInitialPowerupSpawn;
  int bodyQueIndex;
  gentity_t *bodyQue[8];
  int portalSequence;
  qboolean gameStatsReported;
  qboolean mapIsTrainingMap;
  int clientNum1stPlayer;
  int clientNum2ndPlayer;
  char scoreboardArchive1[1024];
  char scoreboardArchive2[1024];
  char firstScorer[40];
  char lastScorer[40];
  char lastTeamScorer[40];
  char firstFrag[40];
  vec3_t red_flag_origin;
  vec3_t blue_flag_origin;
  int spawnCount[4];
  int runeSpawns[5];
  int itemCount[60];
  int suddenDeathRespawnDelay;
  int suddenDeathRespawnDelayLastAnnounced;
  int numRedArmorPickups[4];
  int numYellowArmorPickups[4];
  int numGreenArmorPickups[4];
  int numMegaHealthPickups[4];
  int numQuadDamagePickups[4];
  int numBattleSuitPickups[4];
  int numRegenerationPickups[4];
  int numHastePickups[4];
  int numInvisibilityPickups[4];
  int quadDamagePossessionTime[4];
  int battleSuitPossessionTime[4];
  int regenerationPossessionTime[4];
  int hastePossessionTime[4];
  int invisibilityPossessionTime[4];
  int numFlagPickups[4];
  int numMedkitPickups[4];
  int flagPossessionTime[4];
  gentity_t *dominationPoints[5];
  int dominationPointCount;
  int dominationPointsTallied;
  int racePointCount;
  qboolean disableDropWeapon;
  qboolean teamShuffleActive;
  int lastTeamScores[4];
  int lastTeamRoundScores[4];
  team_t attackingTeam;
  roundState_t roundState;
  int lastTeamCountSent;
  int infectedConscript;
  int lastZombieSurvivor;
  int zombieScoreTime;
  int lastInfectionTime;
  char intermissionMapNames[3][1024];
  char intermissionMapTitles[3][1024];
  char intermissionMapConfigs[3][1024];
  int intermissionMapVotes[3];
  qboolean matchForfeited;
  int allReadyTime;
  qboolean notifyCvarChange;
  int notifyCvarChangeTime;
  int lastLeadChangeTime;
  int lastLeadChangeClient;
} level_locals_t;

// Some extra stuff that's not in the Q3 source. These are the commands you
// get when you type ? in the console. The array has a sentinel struct, so
// check "cmd" == NULL.
typedef struct {
	privileges_t needed_privileges;
	int unknown1;
	char* cmd; // The command name, e.g. "tempban".
	void (*admin_func)(gentity_t* ent);
	int unknown2;
	int unknown3;
	char* description; // Command description that gets printed when you do "?".
} adminCmd_t;

// A pointer to the qagame module in memory and its entry point.
extern void* qagame;
extern void* qagame_dllentry;

// Additional key struct pointers.
extern serverStatic_t* svs;
extern gentity_t* g_entities;
extern level_locals_t* level;
extern gitem_t* bg_itemlist;
extern int bg_numItems;
// Cvars.
extern cvar_t* sv_maxclients;

// Internal QL function pointer types.
typedef void (__cdecl *Com_Printf_ptr)(char* fmt, ...);
typedef void (__cdecl *Cmd_AddCommand_ptr)(char* cmd, void* func);
typedef char* (__cdecl *Cmd_Args_ptr)(void);
typedef char* (__cdecl *Cmd_Argv_ptr)(int arg);
typedef int (__cdecl *Cmd_Argc_ptr)(void);
typedef void (__cdecl *Cmd_TokenizeString_ptr)(const char* text_in);
typedef void (__cdecl *Cbuf_ExecuteText_ptr)(int exec_when, const char* text);
typedef cvar_t* (__cdecl *Cvar_FindVar_ptr)(const char* var_name);
typedef cvar_t* (__cdecl *Cvar_Get_ptr)(const char* var_name, const char* var_value, int flags);
typedef cvar_t* (__cdecl *Cvar_GetLimit_ptr)(const char* var_name, const char* var_value, const char* min, const char* max, int flag);
typedef cvar_t* (__cdecl *Cvar_Set2_ptr)(const char* var_name, const char* value, qboolean force);
typedef void (__cdecl *SV_SendServerCommand_ptr)(client_t* cl, const char* fmt, ...);
typedef void (__cdecl *SV_ExecuteClientCommand_ptr)(client_t* cl, const char* s, qboolean clientOK);
typedef void (__cdecl *SV_ClientEnterWorld_ptr)(client_t *client, usercmd_t *cmd);
typedef void (__cdecl *SV_Shutdown_ptr)(char* finalmsg);
typedef void (__cdecl *SV_Map_f_ptr)(void);
typedef void (__cdecl *SV_ClientThink_ptr)(client_t* cl, usercmd_t* cmd);
typedef void (__cdecl *SV_SetConfigstring_ptr)(int index, const char* value);
typedef void (__cdecl *SV_GetConfigstring_ptr)(int index, char* buffer, int bufferSize);
typedef void (__cdecl *SV_DropClient_ptr)(client_t* drop, const char* reason);
typedef void (__cdecl *FS_Startup_ptr)(const char* gameName);
typedef void (__cdecl *Sys_SetModuleOffset_ptr)(char* moduleName, void* offset);
typedef void (__cdecl *SV_LinkEntity_ptr)(sharedEntity_t* gEnt);
typedef void (__cdecl *SV_SpawnServer_ptr)(char* server, qboolean killBots);
typedef void (__cdecl *Cmd_ExecuteString_ptr)(const char* text);
// VM functions.
typedef void (__cdecl *G_RunFrame_ptr)(int time);
typedef void (__cdecl *G_AddEvent_ptr)(gentity_t* ent, int event, int eventParm);
typedef void (__cdecl *G_InitGame_ptr)(int levelTime, int randomSeed, int restart);
typedef int (__cdecl *CheckPrivileges_ptr)(gentity_t* ent, char* cmd);
typedef char* (__cdecl *ClientConnect_ptr)(int clientNum, qboolean firstTime, qboolean isBot);
typedef void (__cdecl *ClientSpawn_ptr)(gentity_t* ent);
typedef void (__cdecl *Cmd_CallVote_f_ptr)(gentity_t *ent);
typedef void (__cdecl *G_Damage_ptr)(gentity_t *targ, gentity_t *inflictor, gentity_t *attacker, vec3_t dir, vec3_t point, int damage, int dflags, int mod);
typedef void (__cdecl *Touch_Item_ptr)(gentity_t *ent, gentity_t *other, trace_t *trace);
typedef gentity_t* (__cdecl *LaunchItem_ptr)(gitem_t *item, vec3_t origin, vec3_t velocity);
typedef gentity_t* (__cdecl *Drop_Item_ptr)(gentity_t *ent, gitem_t *item, float angle);
typedef void (__cdecl *G_StartKamikaze_ptr)(gentity_t *ent);
typedef void (__cdecl *G_FreeEntity_ptr)(gentity_t *ed);
typedef void (__cdecl *Sys_IsLANAddress_ptr)(netadr_t adr);

// Some of them are initialized by Initialize(), but not all of them necessarily.
extern Com_Printf_ptr Com_Printf;
extern Cmd_AddCommand_ptr Cmd_AddCommand;
extern Cmd_Args_ptr Cmd_Args;
extern Cmd_Argv_ptr Cmd_Argv;
extern Cmd_Argc_ptr Cmd_Argc;
extern Cmd_TokenizeString_ptr Cmd_TokenizeString;
extern Cbuf_ExecuteText_ptr Cbuf_ExecuteText;
extern Cvar_FindVar_ptr Cvar_FindVar;
extern Cvar_Get_ptr Cvar_Get;
extern Cvar_GetLimit_ptr Cvar_GetLimit;
extern Cvar_Set2_ptr Cvar_Set2;
extern SV_SendServerCommand_ptr SV_SendServerCommand;
extern SV_ExecuteClientCommand_ptr SV_ExecuteClientCommand;
extern SV_ClientEnterWorld_ptr SV_ClientEnterWorld;
extern SV_Shutdown_ptr SV_Shutdown; // Used to get svs pointer.
extern SV_Map_f_ptr SV_Map_f; // Used to get Cmd_Argc
extern SV_SetConfigstring_ptr SV_SetConfigstring;
extern SV_GetConfigstring_ptr SV_GetConfigstring;
extern SV_DropClient_ptr SV_DropClient;
extern Sys_SetModuleOffset_ptr Sys_SetModuleOffset;
extern SV_SpawnServer_ptr SV_SpawnServer;
extern Cmd_ExecuteString_ptr Cmd_ExecuteString;
extern Sys_IsLANAddress_ptr Sys_IsLANAddress;
// VM functions.
extern G_RunFrame_ptr G_RunFrame;
extern G_AddEvent_ptr G_AddEvent;
extern G_InitGame_ptr G_InitGame;
extern CheckPrivileges_ptr CheckPrivileges;
extern ClientConnect_ptr ClientConnect;
extern ClientSpawn_ptr ClientSpawn;
extern Cmd_CallVote_f_ptr Cmd_CallVote_f;
extern G_Damage_ptr G_Damage;
extern Touch_Item_ptr Touch_Item;
extern LaunchItem_ptr LaunchItem;
extern Drop_Item_ptr Drop_Item;
extern G_StartKamikaze_ptr G_StartKamikaze;
extern G_FreeEntity_ptr G_FreeEntity;

// Server replacement functions for hooks.
void __cdecl My_Cmd_AddCommand(char* cmd, void* func);
void __cdecl My_Sys_SetModuleOffset(char* moduleName, void* offset);
#ifndef NOPY
void __cdecl My_SV_ExecuteClientCommand(client_t *cl, char *s, qboolean clientOK);
void __cdecl My_SV_SendServerCommand(client_t* cl, char* fmt, ...);
void __cdecl My_SV_ClientEnterWorld(client_t* client, usercmd_t* cmd);
void __cdecl My_SV_SetConfigstring(int index, char* value);
void __cdecl My_SV_DropClient(client_t* drop, const char* reason);
void __cdecl My_Com_Printf(char* fmt, ...);
void __cdecl My_SV_SpawnServer(char* server, qboolean killBots);
// VM replacement functions for hooks.
void __cdecl My_G_RunFrame(int time);
void __cdecl My_G_InitGame(int levelTime, int randomSeed, int restart);
char* __cdecl My_ClientConnect(int clientNum, qboolean firstTime, qboolean isBot);
void __cdecl My_ClientSpawn(gentity_t* ent);

void __cdecl My_G_StartKamikaze(gentity_t* ent);
#endif

// Custom commands added using Cmd_AddCommand during initialization.
void __cdecl SendServerCommand(void); // "cmd"
void __cdecl CenterPrint(void); // "cp"
void __cdecl RegularPrint(void); // "p"
void __cdecl Slap(void); // "slap"
void __cdecl Slay(void); // "slay"
#ifndef NOPY
// PyRcon gives the owner the ability to execute pyminqlx commands as if the
// owner executed them.
void __cdecl PyRcon(void);
// PyCommand is special. It'll serve as the handler for console commands added
// using Python. This means it can serve as the handler for a bunch of commands,
// and it'll take care of redirecting it to Python.
void __cdecl PyCommand(void);
void __cdecl RestartPython(void); // "pyrestart"
#endif

#endif /* QUAKE_COMMON_H */
