#include <stacsos/console.h>
#include <stacsos/memops.h>
#include <stacsos/objects.h>
#include <stacsos/user-syscall.h>

using namespace stacsos;

int main(char *command_line)
{
	if (!command_line || memops::strlen(command_line) == 0) {
		console::get().write("error: usage: ls <abs_path>\n");
		return 1;
	}
	const char *path = command_line;
    console::get().writef("Searching for %s\n", path);
	stat_result *buff = new stat_result;
	syscall_result r = syscalls::stat(buff, true, path);

	if (r.code == syscall_result_code::not_found) {
		console::get().writef("error: path %s not found\n", path);
		return 1;
    }
	console::get().writef("%s, %d", buff->name, buff->size_or_count);
	return 0;
}
