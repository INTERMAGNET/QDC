#ifndef QDC_hpp
#define QDC_hpp


//#include <stdint.h>

#include <string>
#include <vector>
#include <fstream>

#include "EMApplication.hpp"
using namespace std;
using namespace EM;

#include "QDComp.hpp"

// Documentation
class QDCInit;

class QDC : public EMApplication {
	friend class QDCInit;
	public:
		const char* name()       {return "QDC";}
		const char* author()     {return "Peter Crosthwaite; for INTERMAGNET http:://www.intermagnet.org";}
		const char* copyright()  {return "Geoscience Australia";}
		unsigned versionMajor()  {return 0;}
		unsigned versionMinor()  {return 1;}
		unsigned versionRelease(){return 0;}

		const char* about()     { return
"QDC = Quasi Definitive Comparison; written for geomagnetic observatory data comparison.\n"
"\n"
"QDC reports statistics of the vector difference DataSetA-DataSetB.\n"
"\n"
"Each data set consists of 1 or more files specified as \"format:filename\".\n"
"DataSetA might be DEFINITIVE data and DataSetB might be QUASIDEFINITIVE data,\n"
"however they are only required to have the same recording interval (e.g. 1-minute).\n"
"The data sets must also have the same recorded elements (e.g. \"HDZ*\") unless they\n"
"are converted to \"XYZ\" using the --XYZ option.\n"
"\n"
"Recognised file formats      are {iaga2002, iaf}.\n"
"Recognised recorded elements are {XYZ*, HDZ*}.\n"
"\n"
"There should be no data gaps (except those represented by MISSING data values).\n"
"\n"
"Other uses:\n"
	"\tCompare preliminary data from two variometers at one station.\n"
	"\tCompare definitive data from two nearby stations.\n"
	"\tetc.\n"
"\n"
"Please report BUGS and modification requests to geomag@ga.gov.au\n"
		;}


		const char* example()     { return
"QDC -a iaf:cnb09dec.bin -b iaga2002:CNB200912010000pmin.min -b iaga2002:CNB200912020000pmin.min --ByDayByMonth --XYZ\n"
"\n"
	"\tConvert the data to XYZ, and report the Monthly statistics of the Daily averages.\n"
	"\tAll days with any amount of data are used.  NO \"90%\" RULE IS APPLIED, nor is any other rule.\n"
"\n"
"QDC -FF -EQDC\n"
	"\twhere a text file named F contains\n"
	"\t\tdsa=iaf:cnb09dec.bin\n"
	"\t\tdsb=iaga2002:CNB200912010000pmin.min\n"
	"\t\tdsb=iaga2002:CNB200912020000pmin.min\n"
	"\t\tXYZ=true\n"
	"\tand where an environment variable is set as follows\n"
	"\t\tset QDCByDayByMonth=true\n"
"\n"
	"\tSame as previous example.\n"
"\n"
"QDC -a iaf:cnb09jan.bin -b iaga2002:CNB200901010000pmin.min --ByDayByMonthbyYear \n"
"\n"
	"\tReport the Yearly statistics of the Monthly average of the Daily averages.\n"
	"\tAll months with any amount of data are used, as are all days with any amount of data.\n"
	"\tNO \"90%\" RULE IS APPLIED, nor is any other rule.\n"

		;}

		QDC();
		virtual ~QDC();
		int main();
		class DataSource {
			public:
				DataSource(const string& n){
					size_t colon=n.find(':');
					if (colon==string::npos) {
						format="";
						name=n;
					} else {
						format=n.substr(0,colon);
						name=n.substr(colon+1,string::npos);
					}
					s = new(ifstream);
					s->open(name.c_str(),ios_base::in);
				}

				DataSource(){}
				void connect(){s->open(name.c_str(),ios_base::in|ios_base::binary);}
				void disconnect(){s->close();}
				SingleSource* extract();
				bool operator!(){return !(*s);}


				string   format;
				string   name;
				ifstream *s;

				class Hints {
					public:
						Hints(Data::Format f=Data::Unknown, size_t sz=0, TimeInterval t=TimeInterval(0,0,0,0,1,0)):fmt(f),n(sz),interval(t){}
						Data::Format fmt;
						size_t       n;
						TimeInterval interval;
				};
			private:
				Hints getHints(); // return information for creating SingleSource
		};

	private:
		//ofstream _out;

		vector<DataSource> _dsa;
		vector<DataSource> _dsb;

		bool _bRepByDayByAll;
		bool _bRepByDayByMonthByAll;
		bool _bRepByDayByMonthByYearByAll;
		bool _bRepByDayByYearByAll;
		bool _bRepByMonthByAll;
		bool _bRepByMonthByYearByAll;
		bool _bRepByYearByAll;

		bool _bRepByDayByYear;
		bool _bRepByDayByMonthByYear;
		bool _bRepByMonthByYear;

		bool _bRepByDayByMonth;

		// and the 4 Basic accumulations of Raw data
		bool _bRepByDay;
		bool _bRepByMonth;
		bool _bRepByYear;
		bool _bRepByAll;


		bool _bRepD;
		bool _bRepM;
		bool _bRepY;
		bool _bRepA;

		bool _bXYZ;

		void extractDataFromSource(DataSource&s, MultiSource& m);
};

#endif
