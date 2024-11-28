#include <stacsos/console.h>
#include <stacsos/memops.h>
#include <stacsos/objects.h>
#include <stacsos/user-syscall.h>

using namespace stacsos;

/// @brief Writes the short form of a stat_result to console stdout
/// @param buff buffer containing stat_result
/// @param count number of stat_results in the buffer
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

/// @brief Writes the long form of a stat_result to console stdout
/// @param buff buffer containing stat_result
/// @param count number of stat_results in the buffer
void write_long(stat_result *buff, int count)
{
	for (int i = 0; i < count; i++) {
		// print parent directory unix style as ".."
		// and change its type to a directory so that it is
		// more intuitive to the user
		if (buff[i].name[0] == '\0') {
			memops::strncpy(buff[i].name, "..", 2);
			buff[i].kind = 1;
		}
		const char *kind = buff[i].kind == 0 ? "[F]" : "[D]";
		// If it's a directory don't write the size
		if (buff[i].kind == 1) {
			console::get().writef("%s %s\n", kind, buff[i].name);
		} else {
			console::get().writef("%s %s %d\n", kind, buff[i].name, buff[i].size_or_count);
		}
	}
}

/// @brief Gets the directory info for a path, or the file info if the path was a file
/// @param path Path to search at
/// @param buff Result
/// @return true if the file/directory was found, false otherwise
bool get_dir_info(const char *path, stat_result *buff)
{
	syscall_result r = syscalls::stat(buff, true, path);
	if (r.code == syscall_result_code::not_found || r.code == syscall_result_code::not_supported) {
		return false;
	}
	return true;
}

/// @brief
/// @param path
/// @param buff
/// @return
bool get_dir_children(const char *path, stat_result *buff)
{
	syscall_result r = syscalls::stat(buff, false, path);
	return r.code == syscall_result_code::ok;
}

/// @brief Extracts the -l flag from the command line args if it exists
/// @param cmdline Entire command line argument string
/// @param flag set to true if -l is present
/// @return the extracted absolute path, or nullptr if failed
char *extract_args(char *cmdline, bool &flag)
{
	// Skip any leading whitespace
	while (*cmdline == ' ') {
		cmdline++;
	}
	// No args found
	if (*cmdline == '\0') {
		console::get().write("error: no arguments specified\n");
		return nullptr;
	}
	// Check for -l flag
	if (*cmdline == '-' && *(cmdline + 1) == 'l') {
		flag = true;
		cmdline += 2;
	}
	// Advanced past any whitespace following -l flag
	while (*cmdline == ' ') {
		cmdline++;
	}
	// Check that there's a path after the flag
	if (*cmdline == '\0') {
		console::get().write("error: path not specified\n");
		return nullptr;
	}
	// If an absolute path isn't found, complain
	if (*cmdline != '/') {
		console::get().write("error: must specify absolute path\n");
		return nullptr;
	}
	// Return path with flags removed
	return cmdline;
}

int main(char *command_line)
{
	if (!command_line || memops::strlen(command_line) == 0) {
		console::get().write("error: usage: ls <flag> <abs_path>\n");
		return 1;
	}
	bool print_long = false;
	char *path = extract_args(command_line, print_long);
	if (!path) {
		return 1;
	}
	// Fetch directory info for path
	stat_result buff;
	if (!get_dir_info(path, &buff)) {
		console::get().writef("error: path %s not found\n", path);
		return 1;
	}

	// If the syscall found a file at the path then print the result to stdout and exit
	if (buff.kind == 0) {
		print_long ? write_long(&buff, 1) : write_short(&buff, 1);
	} else {
		// Otherwise the syscall found a directory with n children, so stat the children and
		// print the results to stdout
		stat_result children[buff.size_or_count];
		assert(get_dir_children(path, &(children[0])));
		print_long ? write_long(&(children[0]), buff.size_or_count) : write_short(&(children[0]), buff.size_or_count);
	}
	return 0;
}
