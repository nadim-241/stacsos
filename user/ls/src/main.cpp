#include <stacsos/console.h>
#include <stacsos/memops.h>
#include <stacsos/objects.h>
#include <stacsos/user-syscall.h>

using namespace stacsos;

void write_short(stat_result *buff, int count)
{
	for (int i = 0; i < count; i++) {
		const char *kind = buff[i].kind == 0 ? "[F]" : "[D]";
		console::get().writef("%s %s\n", kind, buff[i].name);
	}
}

void write_long(stat_result *buff, int count)
{
	for (int i = 0; i < count; i++) {
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

void get_dir_children(const char *path, stat_result *buff) { syscalls::stat(buff, false, path); }

int main(char *command_line)
{
	if (!command_line || memops::strlen(command_line) == 0) {
		console::get().write("error: usage: ls <abs_path>\n");
		return 1;
	}
	const char *path = command_line;
	stat_result buff;
	if (!get_dir_info(path, &buff)) {
		console::get().writef("error: path %s not found\n", path);
		return 1;
	}

	if (buff.kind == 0) {
		write_long(&buff, 1);
	} else {
		stat_result children[buff.size_or_count];
		get_dir_children(path, &(children[0]));
		write_long(&(children[0]), buff.size_or_count);
	}
	return 0;
}
