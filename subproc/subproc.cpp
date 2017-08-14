#include <string>
#include <stdexcept>
#include <cstdlib>

#include <ngkh/subproc.hpp>
#include <ngkh/subproc.h>

using std::string;
using std::runtime_error;

namespace ngkh {

string subproc::find_exec_path(const string &exec_name) throw (runtime_error)
{
	char   *path_c_str = ngkh_subproc_find_exec_path(exec_name.c_str());
	string  ret;

	if (!path_c_str)
		throw runtime_error("Cannot find absolute path of \"" + exec_name + "\".");

	ret = path_c_str;

	free(path_c_str);

	return string(ret);
}

} // end of namespace ngkh

