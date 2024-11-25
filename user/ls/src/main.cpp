#include <stacsos/console.h>
#include <stacsos/memops.h>
#include <stacsos/objects.h>
#include <stacsos/user-syscall.h>

using namespace stacsos;

void write_short(stat_result *buff, int count)
{
	for (int i = 0; i < count; i++) {
		// don't print parent directory if not printing long form
		if (buff[i].name[0] == '\0') {
			continue;
		}
		const char *kind = buff[i].kind == 0 ? "[F]" : "[D]";
		console::get().writef("%s %s\n", kind, buff[i].name);
	}
}

void write_long(stat_result *buff, int count)
{
	for (int i = 0; i < count; i++) {
		// print parent directory unix style as ".."
		// and change its type to a directory
		if (buff[i].name[0] == '\0') {
			memops::strncpy(buff[i].name, "..", 2);
			buff[i].kind = 1;
		}
		const char *kind = buff[i].kind == 0 ? "[F]" : "[D]";
		if (buff[i].kind == 1) {
			console::get().writef("%s %s\n", kind, buff[i].name);
		} else {
			console::get().writef("%s %s %d\n", kind, buff[i].name, buff[i].size_or_count);
		}
	}
}

bool get_dir_info(const char *path, stat_result *buff)
{
	syscall_result r = syscalls::stat(buff, true, path);
	if (r.code == syscall_result_code::not_found || r.code == syscall_result_code::not_supported) {
		return false;
	}
	return true;
}

bool get_dir_children(const char *path, stat_result *buff)
{
	syscall_result r = syscalls::stat(buff, false, path);
	return r.code == syscall_result_code::ok;
}

char *extract_args(char *cmdline, bool &flag)
{
	// Skip any leading whitespace
	while(*cmdline == ' ') {
		cmdline++;
	}
	if(*cmdline == '\0') {
		console::get().write("error: no arguments specified\n");
		return nullptr;
	}
	if(*cmdline == '-' && *(cmdline + 1) == 'l') {
		flag = true;
		cmdline += 2;
	}
	while(*cmdline == ' ') {
		cmdline++;
	}
	if(*cmdline == '\0') {
		console::get().write("error: path not specified\n");
		return nullptr;
	}
	if(*cmdline != '/') {
		console::get().write("error: must specify absolute path\n");
		return nullptr;
	}
	return cmdline;
}	

int main(char *command_line)
{
	if (!command_line || memops::strlen(command_line) == 0) {
		console::get().write("error: usage: ls <flag> <abs_path>\n");
		return 1;
	}
	bool print_long = false;
	char* path = extract_args(command_line, print_long);
	if(!path) {
		return 1;
	}
	stat_result buff;
	if (!get_dir_info(path, &buff)) {
		console::get().writef("error: path %s not found\n", path);
		return 1;
	}

	if (buff.kind == 0) {
		print_long ? write_long(&buff, 1) : write_short(&buff, 1);
	} else {
		stat_result children[buff.size_or_count];
		assert(get_dir_children(path, &(children[0])));
		print_long ? write_long(&(children[0]), buff.size_or_count) : write_short(&(children[0]), buff.size_or_count);
	}
	return 0;
}
