#ifndef EMApplication_hpp
#define EMApplication_hpp

#include <string>
#include <stdexcept>

// Documentation
namespace EM {

class EMApplicationInit;

class EMApplication {

	friend class EMApplicationInit;

	public:
		virtual const char* name()       =0;
		virtual const char* author()     =0;
		virtual const char* copyright()  =0;
		virtual unsigned versionMajor()  =0;
		virtual unsigned versionMinor()  =0;
		virtual unsigned versionRelease()=0;
		virtual const char* about()      {return "No information available";};
		virtual const char* example()    {return "No information available";};

		// add example about, and have --help = --version --about --example (no -V option)

	private:
		std::string invokedAs;
};

class EMApplicationException : public std::runtime_error {
	public:
		EMApplicationException(const std::string& s) : std::runtime_error(s){}
};


} // namespace EM

#endif
