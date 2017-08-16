#ifndef __NGKH_SUBPROC_HPP__
#define __NGKH_SUBPROC_HPP__

#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>
#include <csignal>

#include <sys/wait.h>
#include <sys/types.h>

#include <ngkh/subproc.h>

namespace ngkh {

class subproc {
	public:
		//Constructors
		subproc(void) : std_in(NULL), std_out(NULL), _pid(0) {}
		subproc(const std::string &exec_path, const std::vector<std::string> &argv, bool isUseStdOut = false, bool isUseStdIn = false)
			: std_in(NULL), std_out(NULL), _pid(0)
		{ this->create(exec_path, argv, isUseStdOut, isUseStdIn); /* This set member variables. */}

		// Destructor
		~subproc(void);

		// Typedef
		typedef pid_t pid_type; // This means "ngkh::subproc::pid_type"

		// Methods
		void     create(const std::string &exec_path, const std::vector<std::string> &argv, bool isUseStdOut, bool isUseStdIn);
		pid_type pid(void) { return this->_pid; }
		bool     write_stdin(const std::string &str);
		bool     write_stdin(std::istream &ios);
		void     read_stdout(std::string &str);
		void     read_stdout(std::ostream &ios);
		void     close_stdin(void)  { if (this->std_in)  { fclose(this->std_in);  this->std_in  = NULL; } }
		void     close_stdout(void) { if (this->std_out) { fclose(this->std_out); this->std_out = NULL; } }
		bool     is_open_in(void)   { if (this->std_in)  return true; else return false; }
		bool     is_open_out(void)  { if (this->std_out) return true; else return false; }
		bool     wait(void);
		int      get_return(void) { return this->ret; }
		void     terminate(void);

		// static methods
		static std::string find_exec_path(const std::string &exec_path) throw (std::runtime_error);

	private:
		FILE     *std_in;
		FILE     *std_out;
		pid_type  _pid;
		int       ret;
};

// Destructor
inline subproc::~subproc(void)
{
	this->terminate();
	this->close_stdin();
	this->close_stdout();
}

// Methods definition
inline void subproc::create(const std::string &exec_path, const std::vector<std::string> &argv, bool isUseStdOut = false, bool isUseStdIn = false)
{
	size_t num_argv = argv.size();
	char *const *args;

	this->terminate();

	this->close_stdin();
	this->close_stdout();

	args = new char*[num_argv+1];

	for (size_t i = 0; i < num_argv; i++)
		const_cast<char*&>(args[i]) = const_cast<char*>(argv[i].c_str());

	const_cast<char*&>(args[num_argv]) = NULL;

	if (isUseStdOut && isUseStdIn)
		this->_pid = ngkh_subproc(exec_path.c_str(), args, &(this->std_in), &(this->std_out));

	else if (isUseStdOut)
		this->_pid = ngkh_subproc(exec_path.c_str(), args, NULL, &(this->std_out));

	else if (isUseStdIn)
		this->_pid = ngkh_subproc(exec_path.c_str(), args, &(this->std_in), NULL);

	else
		this->_pid = ngkh_subproc(exec_path.c_str(), args, NULL, NULL);

	delete [] args;
}

inline bool subproc::write_stdin(const std::string &str)
{
	if (!this->std_in)
		return false;

	if (fputs(str.c_str(), this->std_in) == -1) {
		std::cerr << "Error: Write to stdin of child proc.\n" << std::endl;
		return false;
	}

	return true;
}

inline bool subproc::write_stdin(std::istream &is)
{
	std::string str;

	if (!this->std_in)
		return false;

	str.assign((std::istreambuf_iterator<char>(is)),
	            std::istreambuf_iterator<char>());

	return this->write_stdin(str);
}

inline void subproc::read_stdout(std::string &str)
{
	char strbuf[256];

	if (!this->std_out)
		return;

	while (fgets(strbuf, 256, this->std_out))
		str += strbuf;
}

inline void subproc::read_stdout(std::ostream &os)
{
	char strbuf[256];

	if (!this->std_out)
		return;

	while (fgets(strbuf, 256, this->std_out))
		os << strbuf;
}

inline bool subproc::wait(void)
{
	int  status;
	bool isNoProblem = false;

	if (this->_pid <= 0)
		return isNoProblem;

	if (waitpid(this->_pid, &status, 0) == -1) {
		this->terminate();
		return isNoProblem;
	}

	if (WIFEXITED(status)) {
		this->ret = WEXITSTATUS(status);
		isNoProblem = true;
	}

	this->_pid = 0;

	return isNoProblem;
}

inline void subproc::terminate(void)
{
	if (this->_pid <= 0)
		return;

	if (waitpid(this->_pid, NULL, WNOHANG) == 0) {
		if (kill(this->_pid, SIGTERM) == -1)
			kill(this->_pid, SIGKILL);

		waitpid(this->_pid, NULL, 0);
	}

	this->_pid = 0;
}

} // end of namespace ngkh

#endif

