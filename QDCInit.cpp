#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <list>
using namespace std;

#include "QDCInit.hpp"

// Documentation

namespace {
	const char DSA []    ="dsa";                   const char Dsa ='a';
	const char DSB []    ="dsb";                   const char Dsb ='b';
	//const char OUTF[]    ="#1";


	const char RDA[]     ="ByDayByAll";
	const char RDMA[]    ="ByDayByMonthByAll";
	const char RDMYA[]   ="ByDayByMonthByYearByAll";
	const char RDYA[]    ="ByDayByYearByAll";
	const char RMA[]     ="ByMonthByAll";
	const char RMYA[]    ="ByMonthByYearByAll";
	const char RYA[]     ="ByYearByAll" ;

	const char RDY[]     ="ByDayByYear";
	const char RDMY[]    ="ByDayByMonthByYear";
	const char RMY[]     ="ByMonthByYear";

	const char RDM[]     ="ByDayByMonth";

	const char RD[]      ="ByDay";
	const char RM[]      ="ByMonth";
	const char RY[]      ="ByYear";
	const char RA[]      ="ByAll";

	const char XYZ[]     ="XYZ";                   const char Xyz ='x';

	//const char SINT[]    ="nScalarIntervals";      const char Sint ='s';
	//const char CMPL[]    ="complexity";
	//const char DIST[]    ="distributionParameter"; const char Dist ='D';
};

QDCInit::QDCInit(QDC &app, int argc, char *argv[])
	:EMApplicationInit(app, argc, argv) {

	notify(); //required if init() is to be called back;

	//following shows that reference to app direct, or baseclass emApp& can be used

	EMApplicationInit::options.add_options()
		((string(DSA )+","+Dsa   ).c_str(),PO::value< vector<string> >(),                                  "data set A source file format:filename")
		((string(DSB )+","+Dsb   ).c_str(),PO::value< vector<string> >(),                                  "data set B source file format:filename")


		((string(RDA)            ).c_str(),PO::bool_switch( &app._bRepByDayByAll),                         "Report data By Day By All")
		((string(RDMA)           ).c_str(),PO::bool_switch( &app._bRepByDayByMonthByAll),                  "Report data By Day By Month By All")
		((string(RDMYA)          ).c_str(),PO::bool_switch( &app._bRepByDayByMonthByYearByAll),            "Report data By Day By Month By Year By All")
		((string(RDYA)           ).c_str(),PO::bool_switch( &app._bRepByDayByYearByAll),                   "Report data By Day By Year All")
		((string(RMA)            ).c_str(),PO::bool_switch( &app._bRepByMonthByAll),                       "Report data By Month By All")
		((string(RMYA)           ).c_str(),PO::bool_switch( &app._bRepByMonthByYearByAll),                 "Report data By Month By Year By All")
		((string(RYA)            ).c_str(),PO::bool_switch( &app._bRepByYearByAll),                        "Report data By Year By All")

		((string(RDY)            ).c_str(),PO::bool_switch( &app._bRepByDayByYear),                        "Report data By Day By Year")
		((string(RDMY)           ).c_str(),PO::bool_switch( &app._bRepByDayByMonthByYear),                 "Report data By Day By Month By Year")
		((string(RMY)            ).c_str(),PO::bool_switch( &app._bRepByMonthByYear),                      "Report data By Month By Year")

		((string(RDM)            ).c_str(),PO::bool_switch( &app._bRepByDayByMonth),                       "Report data By Day By Month")

		((string(RD)             ).c_str(),PO::bool_switch( &app._bRepByDay),                              "Report data By Day")
		((string(RM)             ).c_str(),PO::bool_switch( &app._bRepByMonth),                            "Report data By Month")
		((string(RY)             ).c_str(),PO::bool_switch( &app._bRepByYear),                             "Report data By Year")
		((string(RA)             ).c_str(),PO::bool_switch( &app._bRepByAll),                              "Report data By All")

		((string(XYZ)+","+Xyz    ).c_str(),PO::bool_switch( &app._bXYZ),                                   "Compare in XYZ")

		//((string(DIST) +","+Dist ).c_str(),PO::value(&app._distributionParameters),                        "distribution parameter(list)")
		//((string(SINT) +","+Sint ).c_str(),PO::value( &app._nScalarContinuityIntervals)->default_value(1), "Number of scalar continuity intervals")
		//((string(CMPL)           ).c_str(),PO::value( &app._complexityParameter)->default_value(1),        "Penalise models poorly supported by data/larger=less penalty")

		//((string(OUTF)           ).c_str(),PO::value<string>(), "Output file(positional 1)")
	;
	//positionals.add(OUTF,1);

}


int QDCInit::init() {
	int i=0;

	if (map.count(DSA)) {
		vector<string> dss(map[DSA].as< vector<string> >());
		vector<QDC::DataSource>& ds = (dynamic_cast<QDC&>(emApp))._dsa;
		ds.reserve(dss.size());

		string format = "auto";
		for (vector<string>::iterator i = dss.begin(); i!= dss.end(); ++i) {
			ds.push_back(QDC::DataSource(*i));
			if (ds.back().format.length()==0) ds.back().format = format;
			else                              format = ds.back().format;
			if (!ds.back()) {
				throw EMApplicationException( string("Could not open <"+ds.back().format+"> Data Source <"+ds.back().name+">"));
			}
			ds.back().disconnect();
		}
	}

	if (map.count(DSB)) {
		vector<string> dss(map[DSB].as< vector<string> >());
		vector<QDC::DataSource>& ds = (dynamic_cast<QDC&>(emApp))._dsb;
		ds.reserve(dss.size());

		string format = "auto";
		for (vector<string>::iterator i = dss.begin(); i!= dss.end(); ++i) {
			ds.push_back(QDC::DataSource(*i));
			if (ds.back().format.length()==0) ds.back().format = format;
			else                              format = ds.back().format;
			if (!ds.back()) {
				throw EMApplicationException( string("Could not open <"+ds.back().format+"> Data Source <"+ds.back().name+">"));
			}
			ds.back().disconnect();
		}
	}

/*
	cout<<"Check OUTF"<<endl;
	ofstream& outf = (dynamic_cast<QDC&>(emApp))._out;
	if (map.count(OUTF)) {

		outf.open(map[OUTF].as<string>().c_str(),ios_base::app);
		if (!outf) throw EMApplicationException( string("Could not open output file: ")+map[OUTF].as<string>() );

	} else {
		//outf.rdbuf(cout.rdbuf()); ?? how do I do this
		throw EMApplicationException("Ouput file not specified");
	}
 */
	return i;
};

