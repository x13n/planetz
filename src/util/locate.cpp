#include "../constants.h"
#include <string>
#include <iostream>
std::string get_bin_path()
{
	char path[PATH_MAX_LEN];
	size_t bytes = readlink("/proc/self/exe", path, PATH_MAX_LEN);
	std::string path_str( path, path+bytes );
	path_str.resize( path_str.find_last_of('/') + 1 );
	return path_str;
}
