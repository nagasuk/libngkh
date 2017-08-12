#ifndef __NGKH_PRINTPROGRESS_H_
#define __NGKH_PRINTPROGRESS_H_

#include <iostream>
#include <string>

namespace ngkh {

class print_progress {
	public:
		print_progress(std::ostream &out = std::cout)
			: maxProgress(1), progress(0), dispWidth(50), dispStr("#"), freeStr(""), dispItvl(20),
			  os(out), last_length(0), cnt(0), count(this->dispItvl), cntup(true) {}
		print_progress(size_t MaxProgress, size_t Progress, size_t DispWidth, const std::string &DispStr,
		               const std::string &FreeStr = "", size_t DispItvl = 20, std::ostream &out = std::cout)
			: maxProgress(MaxProgress), progress(Progress), dispWidth(DispWidth), dispStr(DispStr), freeStr(FreeStr),
			  dispItvl(DispItvl), os(out), last_length(0), cnt(0), count(this->dispItvl), cntup(true) {}

		// Methods
		void setMaxProgress(size_t MaxProgress) { this->maxProgress = MaxProgress; }
		void setProgress(size_t Progress) { this->progress = Progress; }
		void setDispWidth(size_t DispWidth) { this->dispWidth = DispWidth; }
		void setDispStr(const std::string &DispStr) { this->dispStr = DispStr; }
		void setFreeStr(const std::string &FreeStr) { this->freeStr = FreeStr; }
		void setDispInterval(size_t DispItvl) { this->dispItvl = this->count = DispItvl; }

		size_t getMaxProgress(void) { return this->maxProgress; }
		size_t getProgress(void)    { return this->progress; }

		void reset(void) { this->progress = 0; this->cnt = 0; this->count = this->dispItvl; this->cntup = true; }

		// Overrided operators
		const print_progress &operator++(int)  { this->progress++; return *this; }
		      print_progress  operator++(void) { print_progress pre(*this); (*this)++; return pre; }

		void operator()(bool isFlush);

	private:
		size_t        maxProgress;
		size_t        progress;
		size_t        dispWidth;
		std::string   dispStr;
		std::string   freeStr;
		size_t        dispItvl;

		std::ostream &os;

		size_t        last_length;

		size_t        cnt;
		size_t        count;
		bool          cntup;

		size_t        lastLength;

		size_t dispLineLength(void);
};

// Some methods declaration
inline void print_progress::operator()(bool isFlush = false)
{
	if (isFlush) {
		size_t flush_length = this->dispLineLength();

		for (size_t i = 0; i < flush_length; i++) this->os << ' ';
		this->os << '\r' << std::flush;

		this->reset();

		return;
	}

	if ((this->count)++ >= this->dispItvl) {
		size_t nowLength = this->dispLineLength();

		if (nowLength < lastLength) {
			for (size_t i = 0; i < this->lastLength; i++) this->os << ' ';
			this->os << '\r';
		}

		this->os << " |";
		for (size_t i = 0; i < this->cnt; i++) this->os << ' ';
		this->os << this->dispStr;
		for (size_t i = 0; i < (this->dispWidth - this->cnt); i++) this->os << ' ';
		this->os << "| [" << this->progress << '/' << this->maxProgress << ' ' << this->freeStr << "]\r";
		this->os << std::flush;

		this->count = 0;
		this->cnt   = this->cntup ? this->cnt + 1 : this->cnt - 1;
		this->cntup = (this->cnt == 0) ? true : ((this->cnt == this->dispWidth) ? false : this->cntup);

		this->lastLength = nowLength;
	}
}

inline size_t print_progress::dispLineLength(void)
{
	std::ostringstream oss;
	size_t length = 8 + this->dispStr.length() + this->freeStr.length() + this->dispWidth;

	oss << this->progress;
	length += oss.str().length();
	oss.str("");
	oss.clear(std::stringstream::goodbit);
	oss << this->maxProgress;
	length += oss.str().length();

	return length;
}

} // end of namespace ngkh

#endif

