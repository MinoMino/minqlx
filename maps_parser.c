#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include "maps_parser.h"

const char fmt[] = ("%" SCNxPTR "-%" SCNxPTR " %s %x %x:%x %u %[^\n]");

/*
 * Pass it a module_info_t pointer with its name initialized, get it full of info back.
 *
 * Returns a negative number on error, otherwise return the number of pages found under
 * that specific module name.
 */
int GetModuleInfo(module_info_t* module_info) {
	int ret = 0;
	pint start, end;
	int file_offset, dev_major, dev_minor, inode;
	char flags[32], path[4096], linebuf[8192];

	// Check if the name's initialized before we do anything.
	if (!strlen(module_info->name)) return -1;

	FILE* fp = fopen("/proc/self/maps", "r");
	while (fgets(linebuf, sizeof(linebuf), fp) != 0) {
		sscanf(linebuf, fmt, &start, &end, flags, &file_offset, &dev_major, &dev_minor, &inode, path);

		// Some pages have no module name. Ignore those.
		size_t pathlen = strlen(path);
		if (!pathlen) continue;

		int slash = -1;
		for (size_t i = 0; i < pathlen; i++)
			if (path[i] == '/') slash = i;

		// Special name such as [heap]? Ignore.
		if (slash == -1) continue;

		// Check if it's the module we're interested it.
		if (strcmp(module_info->name, &path[slash + 1])) continue;

		// Return error if there's an ambiguity. Could happen if two modules
		// are different, but have the same filename.
		// TODO: Add option to pass the path instead of name to avoid this.
		if (ret && strcmp(path, module_info->path)) return -2;

		if (!ret) { // Only once.
			strcpy(module_info->path, path);
		}

		// Addresses
		module_info->address_start[ret] = start;
		module_info->address_end[ret] = end;

		// Permissions
		module_info->permissions[ret] = 0;
		if (flags[0] == 'r') module_info->permissions[ret] |= PG_READ;
		if (flags[1] == 'w') module_info->permissions[ret] |= PG_WRITE;
		if (flags[2] == 'x') module_info->permissions[ret] |= PG_EXECUTE;
		if (flags[3] == 'p') module_info->permissions[ret] |= PG_PRIVATE;
		if (flags[3] == 's') module_info->permissions[ret] |= PG_SHARED;

		ret++;
	}
	fclose(fp);
	
	module_info->entries = ret;
	return ret;
}

/*
void main() {
	module_info_t module_info;
	strcpy(module_info.name, "mapsparser");

	int res = GetModuleInfo(&module_info);
	if (!res) {
		printf("Fuck this gay Earth\n");
		return;
	}
	else if (res < 0) {
		printf("Returned: %d\n", res);
		return;
	}

	for (int i = 0; i < res; i++) {
		printf("%s (%p-%p): %p\n",
				module_info.name,
				module_info.address_start[i],
				module_info.address_end[i],
				module_info.permissions[i]);
	}
}
*/
