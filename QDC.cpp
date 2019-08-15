#include <iostream>
#include <iomanip>
#include <vector>
#include <cstring>
using namespace std;

#include "QDC.hpp"

using namespace EM;


QDC::QDC() {
}

QDC::~QDC() {
}


//#include "random.hpp"
//Random RNG(16031956U);
//Random RNG;

namespace {
	// DataSource must be connect()'ed
	// getHintsXXX and extractXXX must work together - they are tightly copupled



// BEGIN IAGA2002
	QDC::DataSource::Hints getHintsIaga2002(QDC::DataSource& s) {

		const size_t LEN=128;
		char c[LEN];
		char * cp;

		streampos pBeg, pEnd, pData;
		streamsize pLen;

		s.s->seekg(0,ios_base::end); pEnd=s.s->tellg();
		s.s->seekg(0,ios_base::beg); pBeg=s.s->tellg();

		Data::Format fmt=Data::Unknown;
		TimeInterval interval=0;
		pData=s.s->tellg();

		while (s.s->getline(c,LEN)) {
//			if (c[s.s->gcount()-2]!='|') break;

			if (c[69]!='|') break;
			if        (cp = strstr(c,"Reported")) {
				cp+=8;
				while (isspace(*cp)) ++cp;
				if      (strstr(cp,"XYZ")) fmt =Data::XYZ;
				else if (strstr(cp,"HDZ")) fmt =Data::HDZ;
			} else if (cp = strstr(c,"Data Interval Type")) {
				cp+=18;
				if      (strstr(cp,"1-minute")) interval=TimeInterval(0,0,0,0,1,0);
				else if (strstr(cp,"1-second")) interval=TimeInterval(0,0,0,0,0,1);
			}
			pData=s.s->tellg();
		}
		s.s->seekg(pData,ios_base::beg);
		pLen=pEnd-pData;
		// The line length should be 70 + machine dependent CR/LF; use length of the first data line
		size_t lineLength = s.s->gcount();
		return QDC::DataSource::Hints(fmt,(lineLength==0)?0:pLen/lineLength,interval);
	}

	void extractIaga2002( QDC::DataSource& s, SingleSource& t) {
		const size_t LEN=128;
		char c[LEN];
		int Yr,Mon,Day,Hr,Min; double Sec,A,B,C;
		while (s.s->getline(c,LEN)) {
			if (sscanf(c,"%d-%d-%d %d:%d:%lf %*d %lf %lf %lf",&Yr,&Mon,&Day,&Hr,&Min,&Sec,&A,&B,&C) != 9) break;

			// Not every one adheres strictly to the MISSING 99999.00 and UNRECORDED 88888.00 standard

			if (((A>99998.975) && (A<99999.925)) || ((A>88887.975) && (A<88888.825)))A= Data::INVALID;

			if (((B>99998.975) && (B<99999.925)) || ((B>88887.975) && (B<88888.825)))B= Data::INVALID;
			else if (t.format==Data::HDZ)                                            B/=1; // record in arc-minutes

			if (((C>99998.975) && (C<99999.925)) || ((C>88887.975) && (C<88888.825)))C= Data::INVALID;
			t.data.push_back(Data(Time(Yr,Mon,Day,Hr,Min,Sec),A,B,C));
		}
	}
// END   IAGA2002

// BEGIN IAF

class IMByteOrder { // Intermagnet is a Little Endian format; this should work for Big Endian and even PDP endian
	private:
		union LongUnion {char c[4]; long l;} L;

	public:
		IMByteOrder(){L.l = 0x03020100;}
		#if defined (LITTLE_ENDIAN)
			long hostToIMLong(long l){return l;}
		#else
			long hostToIMLong(long l){
				LongUnion in, out;
				in.l=l;
				out.c[0]=in.c[L.c[0]]; out.c[1]=in.c[L.c[1]]; out.c[2]=in.c[L.c[2]]; out.c[3]=in.c[L.c[3]];
				return out.l;
				}
		#endif

		long iMToHostLong(long l) {return hostToIMLong(l);}
};
static IMByteOrder ByteOrder;

typedef struct {
		char station[4];
		long yearDay;
		long colatitude;
		long longitude;
		long elevation;
		char orientation[4];
		char source[4];
		long dConversion;
		char quality[4];
		char instrumentation[4];
		long k9;
		long samplingRate;
		char sensorOrientation[4];
		char publicationDate[4];
		unsigned char version[4];
		long filler[1];
} IMDHeader;

typedef struct {long mmv[1440];}            IMDMMV_1ch;
typedef struct {long hmv[24];}              IMDHMV_1ch;
typedef struct {long dmv[1];}               IMDDMV_1ch;
typedef struct {long k[8]; long filler[4];} IMDKV;

typedef struct {
	IMDHeader h;
	IMDMMV_1ch mmv[4];
	IMDHMV_1ch hmv[4];
	IMDDMV_1ch dmv[4];
	IMDKV         dkv;
} IMDRecord;

	QDC::DataSource::Hints getHintsIAF(QDC::DataSource& s) {

		streampos pBeg, pEnd, pData;
		streamsize pLen;

		s.s->seekg(0,ios_base::end); pEnd=s.s->tellg();
		s.s->seekg(0,ios_base::beg); pBeg=s.s->tellg(); pData=pBeg;

		Data::Format fmt=Data::Unknown;
		TimeInterval interval(0,0,0,0,1,0);

		IMDRecord d;
		IMDHeader h;
		s.s->read((char *)(&d),sizeof(d));
		if (s.s->gcount()==sizeof(d)) {
			if      (d.h.orientation[0]=='X') fmt=Data::XYZ;
			else if (d.h.orientation[0]=='H') fmt=Data::HDZ;
		}
		s.s->seekg(pData,ios_base::beg);

		pLen=pEnd-pData;
		return QDC::DataSource::Hints(fmt,(pLen/sizeof(d))*1440,interval);
	}

	void extractIAF( QDC::DataSource& s, SingleSource& t) {

	// This extracts the 1-minute data from IAF files, but it could extract the 1-hour or 1-day data instead (using format IAF-H IAF-D say)

		IMDRecord d;
		double A, B, C;

		while(s.s) {
			s.s->read((char *)(&d),sizeof(d));
			//cerr<<s.s->gcount()<<" "<<sizeof(d)<<endl;
			if (s.s->gcount()!=sizeof(d)) break;

			long yearDay = ByteOrder.iMToHostLong(d.h.yearDay);
			Time time(yearDay/1000,1,yearDay%1000,0,0,0);
			TimeInterval interval(0,0,0,0,1,0);

			for (int i=0; i<1440; ++i) {
				A = ByteOrder.iMToHostLong(d.mmv[0].mmv[i])/10.L;
				B = ByteOrder.iMToHostLong(d.mmv[1].mmv[i])/10.L;
				C = ByteOrder.iMToHostLong(d.mmv[2].mmv[i])/10.L;
			// Not every one adheres strictly to the MISSING 99999.9 and ???UNRECORDED 88888.8??? standard - which is different from IAGA2002

			if (((A>99998.975) && (A<99999.925)) || ((A>88887.975) && (A<88888.825)))A= Data::INVALID;

			if (((B>99998.975) && (B<99999.925)) || ((B>88887.975) && (B<88888.825)))B= Data::INVALID;
			else if (t.format==Data::HDZ)                                            B/=1; // record in arc-minutes

			if (((C>99998.975) && (C<99999.925)) || ((C>88887.975) && (C<88888.825)))C= Data::INVALID;
				t.data.push_back(Data(time,A,B,C));
				time = time + interval;
			}

		}
	}

// END   IAF

}; // local namespace




QDC::DataSource::Hints QDC::DataSource::getHints() {
	// Must leave the file positioned at the data redy for extraction
	if (format=="iaga2002") return getHintsIaga2002(*this);
	if (format=="iaf"     ) return getHintsIAF     (*this);
	return Hints();
}

SingleSource* QDC::DataSource::extract() {
	connect();
	Hints h=getHints();

	SingleSource* pSingle=new SingleSource(h.fmt, h.n, h.interval);
	//cerr << format <<h.fmt<<" "<<h.n<<" "<<h.interval<<endl;
	if (format=="iaga2002") extractIaga2002(*this, *pSingle);
	if (format=="iaf"     ) extractIAF     (*this, *pSingle);

	if (pSingle->data.size()>0) {
		pSingle->first=pSingle->data[0].t;
		pSingle->last =pSingle->data.back().t;
	}
	disconnect();
	return pSingle;
}


void QDC::extractDataFromSource(DataSource& s, MultiSource& m) {

	SingleSource* pSingle = s.extract();
	if (pSingle->data.size()>0) m.dataList.push_back(pSingle);
	else                        delete(pSingle);
}


class compareSourceStart {
	public:
		bool operator()(SingleSource* a, SingleSource* b){
			return a->first < b->first;}
};


int SingleSource::N(0);


class Options {
	public:
		//    REPORT
		bool bReportRawByDayByAll;
		bool bReportRawByDayByMonthByAll;
		bool bReportRawByDayByMonthByYearByAll;
		bool bReportRawByDayByYearByAll;
		bool bReportRawByMonthByAll;
		bool bReportRawByMonthByYearByAll;
		bool bReportRawByYearByAll;

		bool bReportRawByDayByYear;
		bool bReportRawByDayByMonthByYear;
		bool bReportRawByMonthByYear;

		bool bReportRawByDayByMonth;

		// and the 4 Basic accumulations of Raw data
		bool bReportRawByDay;
		bool bReportRawByMonth;
		bool bReportRawByYear;
		bool bReportRawByAll;

		//    REQUIRE, as above
		bool bRequireRawByDayByAll;
		bool bRequireRawByDayByMonthByAll;
		bool bRequireRawByDayByMonthByYearByAll;
		bool bRequireRawByDayByYearByAll;
		bool bRequireRawByMonthByAll;
		bool bRequireRawByMonthByYearByAll;
		bool bRequireRawByYearByAll;

		bool bRequireRawByDayByYear;
		bool bRequireRawByDayByMonthByYear;
		bool bRequireRawByMonthByYear;

		bool bRequireRawByDayByMonth;

		bool bRequireRawByDay;
		bool bRequireRawByMonth;
		bool bRequireRawByYear;
		bool bRequireRawByAll;

		Options(bool bDA, bool bDMA, bool bDMYA, bool bDYA, bool bMA, bool bMYA, bool bYA
		       ,bool bDY, bool bDMY, bool bMY
		       ,bool bDM
		       ,bool bD, bool bM, bool bY, bool bA)
		:bReportRawByDayByAll(bDA)
		,bReportRawByDayByMonthByAll(bDMA)
		,bReportRawByDayByMonthByYearByAll(bDMYA)
		,bReportRawByDayByYearByAll(bDYA)
		,bReportRawByMonthByAll(bMA)
		,bReportRawByMonthByYearByAll(bMYA)
		,bReportRawByYearByAll(bYA)

		,bReportRawByDayByYear(bDY)
		,bReportRawByDayByMonthByYear(bDMY)
		,bReportRawByMonthByYear(bMY)

		,bReportRawByDayByMonth(bDM)

		,bReportRawByDay(bD)
		,bReportRawByMonth(bM)
		,bReportRawByYear(bY)
		,bReportRawByAll(bA)
		// duplicate Report as Require
		,bRequireRawByDayByAll(bDA)
		,bRequireRawByDayByMonthByAll(bDMA)
		,bRequireRawByDayByMonthByYearByAll(bDMYA)
		,bRequireRawByDayByYearByAll(bDYA)
		,bRequireRawByMonthByAll(bMA)
		,bRequireRawByMonthByYearByAll(bMYA)
		,bRequireRawByYearByAll(bYA)

		,bRequireRawByDayByYear(bDY)
		,bRequireRawByDayByMonthByYear(bDMY)
		,bRequireRawByMonthByYear(bMY)

		,bRequireRawByDayByMonth(bDM)

		,bRequireRawByDay(bD)
		,bRequireRawByMonth(bM)
		,bRequireRawByYear(bY)
		,bRequireRawByAll(bA)

		{
			// This is to ensure that all pre-requisits are used exactly ONCE
			// I expect that all of the first  group Require only members of the second and third groups or the Basic group
			//      and that all of the second group Require only members of the            third group  or the Basic group
			// etc

			if (bRequireRawByDayByAll)             {bRequireRawByDay = true;}
			if (bRequireRawByDayByMonthByAll)      {bRequireRawByDayByMonth = true;}
			if (bRequireRawByDayByMonthByYearByAll){bRequireRawByDayByMonthByYear = true;}
			if (bRequireRawByDayByYearByAll)       {bRequireRawByDayByYear = true;}
			if (bRequireRawByMonthByAll)           {bRequireRawByMonth = true;}
			if (bRequireRawByMonthByYearByAll)     {bRequireRawByMonthByYear = true;}
			if (bRequireRawByYearByAll)            {bRequireRawByYear = true;}

			if (bRequireRawByDayByYear)            {bRequireRawByDay = true;}
			if (bRequireRawByDayByMonthByYear)     {bRequireRawByDayByMonth = true;}
			if (bRequireRawByMonthByYear)          {bRequireRawByMonth = true;}

			if (bRequireRawByDayByMonth)           {bRequireRawByDay = true;}

			// The above is complicated but managable.
			// If more accumlation levels (say ByWeek) were added, this would get out of hand - another method would be required.
		}
};


void populateCommonSourceSegments( list<CommonSourceSegment>& dataSegments, MultiSource& dataA, MultiSource& dataB) {
	MultiSource::SourceIter sourceIterA; // A initialised in loop below
	vector<Data>::iterator dataIterA, aEnd;

	MultiSource::SourceIter sourceIterB;
	vector<Data>::iterator dataIterB, bEnd;

	if ((sourceIterB=dataB.dataList.begin()) != dataB.dataList.end()) {
		dataIterB = (*sourceIterB)->data.begin();
		bEnd      = (*sourceIterB)->data.end();
	}


	for (sourceIterA=dataA.dataList.begin(); sourceIterA != dataA.dataList.end(); ++sourceIterA) {
		dataIterA = (*sourceIterA)->data.begin();
		aEnd = (*sourceIterA)->data.end();

		while (dataIterA != aEnd) {                            // A start and finish are valid
			while (sourceIterB != dataB.dataList.end()) {
				if (  (dataIterB == bEnd)                      // neither empty nor exhausted
				    ||(dataIterA->t > (*sourceIterB)->last)    // definitely no overlap
				   ) {
					if (++sourceIterB != dataB.dataList.end()) {
						dataIterB = (*sourceIterB)->data.begin();
						bEnd      = (*sourceIterB)->data.end();
					}
				} else {
					break;
				}
			}

			if (sourceIterB == dataB.dataList.end()  ) break;  // nothing to match against
			if ((*sourceIterA)->last < dataIterB->t  ) break;  // all of A predates rest of B

			TimeInterval::DivType n=(dataIterB->t - dataIterA->t)/(*sourceIterA)->timeIncrement;
			if      (n>0) {dataIterA += n;}
			else if (n<0) {dataIterB -= n;}

			size_t nA = aEnd-dataIterA, nB = bEnd-dataIterB, nAB = nA<nB?nA:nB;
			if (nAB>0) {
				dataSegments.push_back(CommonSourceSegment(**sourceIterA,dataIterA,**sourceIterB,dataIterB, nAB));
				dataIterA += nAB;
				dataIterB += nAB;
			}
		}
	}
}

int QDC::main() {
	MultiSource dataSetA, dataSetB;

	//Data::Format format=Data::Unknown;
	for (vector<DataSource>::iterator i=_dsa.begin(); i!= _dsa.end(); ++i) extractDataFromSource((*i), dataSetA);
	for (vector<DataSource>::iterator i=_dsb.begin(); i!= _dsb.end(); ++i) extractDataFromSource((*i), dataSetB);


	if (_bXYZ) {
		dataSetA.convertToXYZ();
		dataSetB.convertToXYZ();
	}

	Data::Format formatA=Data::Unspecified, formatB=Data::Unspecified;
	TimeInterval intervalA, intervalB;

	if (dataSetA.dataList.size()) {
		list<SingleSource*>::iterator i=dataSetA.dataList.begin();  // Shouldn't need to know it is a LIST
		formatA = (*i)->format;
		intervalA = (*i)->timeIncrement;

		for (++i; i != dataSetA.dataList.end(); ++i) {
			if (formatA != (*i)->format)
				throw EMApplicationException( string("Incompatible Formats in Data Set A"));
			if (intervalA != (*i)->timeIncrement)
				throw EMApplicationException( string("Incompatible Time Increments in Data Set A"));
		}
	}
	if (dataSetB.dataList.size()) {
		list<SingleSource*>::iterator i=dataSetB.dataList.begin();
		formatB = (*i)->format;
		intervalB = (*i)->timeIncrement;

		for (++i; i != dataSetB.dataList.end(); ++i) {
			if (formatB != (*i)->format)          throw EMApplicationException( string("Incompatible Formats in Data Set B"));
			if (intervalB != (*i)->timeIncrement) throw EMApplicationException( string("Incompatible Time Increments in Data Set B"));
		}
	}

	if (formatA != formatB)     throw EMApplicationException( string("Incompatible Formats in Data Sets A/B") );
	if (intervalA != intervalB)	throw EMApplicationException( string("Incompatible Time Increments in Data Sets A/B") );


	dataSetA.dataList.sort(compareSourceStart());
	dataSetB.dataList.sort(compareSourceStart());

	if (false) { //testing only
		MultiSource::SourceIter sourceIterA=dataSetA.dataList.begin();
		MultiSource::SourceIter sourceIterB=dataSetB.dataList.begin();

		for (;sourceIterA != dataSetA.dataList.end(); ++sourceIterA) {
			cout<<"SourceA "<<(*sourceIterA)->format <<" "<< (*sourceIterA)->first <<" "<< (*sourceIterA)->last<<endl;
		}
		for (;sourceIterB != dataSetB.dataList.end(); ++sourceIterB) {
			cout<<"SourceB "<<(*sourceIterB)->format <<" "<< (*sourceIterB)->first <<" "<< (*sourceIterB)->last<<endl;
		}
	}

	// the following are fed Accumulations using passOn()
	ProcessAll     pRawByDayByAll(string("(Raw by Day) by All"));
	ProcessAll     pRawByDayByMonthByAll(string("((Raw by Day) by Month) by All"));
	ProcessAll     pRawByDayByMonthByYearByAll(string("(((Raw by Day) by Month) by Year) by All"));
	ProcessAll     pRawByDayByYearByAll(string("((Raw by Day) by Year) by All"));
	ProcessAll     pRawByMonthByAll(string("(Raw by Month) by All"));
	ProcessAll     pRawByMonthByYearByAll(string("((Raw by Month) by Year) by All"));
	ProcessAll     pRawByYearByAll(string("(Raw by Year) by All"));

	ProcessByYear  pRawByDayByYear(string("(Raw by Day) by Year"));
	ProcessByYear  pRawByDayByMonthByYear(string("((Raw by Day) by Month) by Year"));
	ProcessByYear  pRawByMonthByYear(string("(Raw by Month) by Year"));

	ProcessByMonth pRawByDayByMonth(string("(Raw by Day) by Month"));

	// the following are fed Raw data
	ProcessByDay   pRawByDay(string("Raw by Day"));
	ProcessByMonth pRawByMonth(string("Raw by Month"));
	ProcessByYear  pRawByYear(string("Raw by Year"));
	ProcessAll     pRawByAll(string("Raw by All"));

	list<CommonSourceSegment> dataSegments;
	populateCommonSourceSegments(dataSegments, dataSetA, dataSetB);

	if (false) {// testing only
		list<CommonSourceSegment>::iterator i;
		for (i = dataSegments.begin();i != dataSegments.end(); ++i) {
			cout<< "SourceA "<<i->sourceA.first<<" "<<i->dataIterA->t
			    <<" SourceB "<<i->sourceB.first<<" "<<i->dataIterB->t <<" "<< i->n <<endl;
		}
	}
	//cout<<endl;

	Options opt(_bRepByDayByAll,_bRepByDayByMonthByAll,_bRepByDayByMonthByYearByAll,_bRepByDayByYearByAll,_bRepByMonthByAll,_bRepByMonthByYearByAll,_bRepByYearByAll
	           ,_bRepByDayByYear,_bRepByDayByMonthByYear,_bRepByMonthByYear
	           ,_bRepByDayByMonth
	           ,_bRepByDay,_bRepByMonth,_bRepByYear,_bRepByAll);

	if (opt.bRequireRawByDayByAll)             {pRawByDay.passOn(pRawByDayByAll);                          }
	if (opt.bRequireRawByDayByMonthByAll)      {pRawByDayByMonth.passOn(pRawByDayByMonthByAll);            }
	if (opt.bRequireRawByDayByMonthByYearByAll){pRawByDayByMonthByYear.passOn(pRawByDayByMonthByYearByAll);}
	if (opt.bRequireRawByDayByYearByAll)       {pRawByDayByYear.passOn(pRawByDayByYearByAll);              }
	if (opt.bRequireRawByMonthByAll)           {pRawByMonth.passOn(pRawByMonthByAll);                      }
	if (opt.bRequireRawByMonthByYearByAll)     {pRawByMonthByYear.passOn(pRawByMonthByYearByAll);          }
	if (opt.bRequireRawByYearByAll)            {pRawByYear.passOn(pRawByYearByAll);                        }

	if (opt.bRequireRawByDayByYear)            {pRawByDay.passOn(pRawByDayByYear);                         }
	if (opt.bRequireRawByDayByMonthByYear)     {pRawByDayByMonth.passOn(pRawByDayByMonthByYear);           }
	if (opt.bRequireRawByMonthByYear)          {pRawByMonth.passOn(pRawByMonthByYear);                     }

	if (opt.bRequireRawByDayByMonth)           {pRawByDay.passOn(pRawByDayByMonth);                        }

	list<CommonSourceSegment>::iterator i, end=dataSegments.end();
	vector<Data>::iterator iterA, iterB;
	Time t;
	double diff[Data::DIMENSION];
	double A, B;
	for (i=dataSegments.begin(); i!= end; ++i) {
		iterA=i->dataIterA;
		iterB=i->dataIterB;
		for (size_t n=i->n; n; --n, ++iterA, ++iterB) {
			t=iterA->t;
			for (size_t i=0; i<Data::DIMENSION; ++i) {
				A = iterA->component[i];
				B = iterB->component[i];
				if ((A!=Data::INVALID)&&(B!=Data::INVALID)) diff[i]=A-B; else diff[i]=Data::INVALID;
			}
			if (opt.bRequireRawByAll  ) pRawByAll  (t, diff);
			if (opt.bRequireRawByDay  ) pRawByDay  (t, diff);
			if (opt.bRequireRawByMonth) pRawByMonth(t, diff);
			if (opt.bRequireRawByYear ) pRawByYear (t, diff);
		}
	}
	// finalise accumulations
	if (opt.bRequireRawByAll  ) pRawByAll();
	if (opt.bRequireRawByDay  ) pRawByDay();
	if (opt.bRequireRawByMonth) pRawByMonth();
	if (opt.bRequireRawByYear ) pRawByYear();

	// report all of the 15 options that were requested

	if (opt.bReportRawByDayByAll)             {pRawByDayByAll.report();}
	if (opt.bReportRawByDayByMonthByAll)      {pRawByDayByMonthByAll.report();}
	if (opt.bReportRawByDayByMonthByYearByAll){pRawByDayByMonthByYearByAll.report();}
	if (opt.bReportRawByDayByYearByAll)       {pRawByDayByYearByAll.report();}
	if (opt.bReportRawByMonthByAll)           {pRawByMonthByAll.report();}
	if (opt.bReportRawByMonthByYearByAll)     {pRawByMonthByYearByAll.report();}
	if (opt.bReportRawByYearByAll)            {pRawByYearByAll.report();}

	if (opt.bReportRawByDayByYear)            {pRawByDayByYear.report();}
	if (opt.bReportRawByDayByMonthByYear)     {pRawByDayByMonthByYear.report();}
	if (opt.bReportRawByMonthByYear)          {pRawByMonthByYear.report();}

	if (opt.bReportRawByDayByMonth)           {pRawByDayByMonth.report();}

	if (opt.bReportRawByDay)                  {pRawByDay.report();}
	if (opt.bReportRawByMonth)                {pRawByMonth.report();}
	if (opt.bReportRawByYear)                 {pRawByYear.report();}
	if (opt.bReportRawByAll)                  {pRawByAll.report();}

	return 0;
}

Process::~Process(){}

void reportHeader(){
	cout
	<<"Group"
	<<'\t'<<"Time" // <<": central time "
	<<'\t'<<"Begin"<<'\t'<<"End"
	;
	for (size_t j=0; j<ProcessStatistics::DIMENSION; ++j) {
		cout
		<<'\t'<<"Ch"
		<<'\t'<<"Min"<<'\t'<<"Max"
		<<'\t'<<"Mean"<<'\t'<<"Stdev "
		<<'\t'<<"N"
		; // <<'\n';
	}
	cout<<endl;
}

void Process::report(){
	reportHeader();

	ios_base::fmtflags f = cout.flags();
	cout.setf(ios_base::fixed, ios_base::floatfield);
	cout.setf(ios_base::right, ios_base::adjustfield);
	size_t p=cout.precision();
	cout.precision(4);

	for (vector<SummaryItem>::iterator i=summaryOfStatistics.begin(); i!=summaryOfStatistics.end(); ++i) {
		cout
		<<identifier
		<<'\t'<<i->midTime
		<<'\t'<<i->first<<'\t'<<i->last
		; // <<"\n\n";
		for (size_t j=0; j<ProcessStatistics::DIMENSION; ++j) {
			cout
			<<'\t'<<j
			<<'\t'<<setw(8)<<i->statistics.minimum(j)<<'\t'<<setw(8)<<i->statistics.maximum(j)
			<<'\t'<<setw(8)<<i->statistics.mean(j)   <<'\t'<<setw(8)<<i->statistics.sampleStandardError(j)
			<<'\t'<<setw(8)<<i->statistics.count(j)
			; // <<'\n';
		}
		cout<<endl;
//		cout
//		<<identifier
//		<<' ' // <<": central time "
//		<<i->midTime
//		<<" [ "<<i->first<<" : "<<i->last<<" ] "
//		; // <<"\n\n";
//		for (size_t j=0; j<ProcessStatistics::DIMENSION; ++j) {
//			cout
//			<<"\t["<<j<<"]\t"
//			<<"Range [ "<<setw(8)<<i->statistics.minimum(j)<<" to "<<setw(8)<<i->statistics.maximum(j)<<" ]"<<'\t'
//			<<"Mean "  <<setw(8)<<i->statistics.mean(j)   <<"\tStdev "<<setw(8)<<i->statistics.sampleStandardError(j)<<'\t'
//			<<"N="<<setw(8)<<i->statistics.count(j)
//			; // <<'\n';
//		}
//		cout<<endl;
	}
	cout.flags(f);
	cout.precision(p);
}

ProcessAll::~ProcessAll(){}
void ProcessAll::operator()(const Time& t, const double component[3]) {
			if (  (statisticsSummary.statistics.count(0)==0)
			    &&(statisticsSummary.statistics.count(1)==0)
			    &&(statisticsSummary.statistics.count(2)==0))
				statisticsSummary.first=t;
			statisticsSummary.last=t;
			statisticsSummary.statistics(component);
		}

ProcessByDay::~ProcessByDay(){}
void ProcessByDay::operator()(const Time& t, const double component[3]){
			if ((  (statisticsSummary.statistics.count(0)!=0)
			     ||(statisticsSummary.statistics.count(1)!=0)
			     ||(statisticsSummary.statistics.count(2)!=0)
			    )
			    && (t>=endTime))
				addToSummary(); // reset statistics and its count to 0

			if (  (statisticsSummary.statistics.count(0)==0)
			    &&(statisticsSummary.statistics.count(1)==0)
			    &&(statisticsSummary.statistics.count(2)==0)) {
				statisticsSummary.first=t;
				endTime = statisticsSummary.first.endOfDay();
			}
			statisticsSummary.last=t;
			statisticsSummary.statistics(component);
}

ProcessByMonth::~ProcessByMonth(){}
void ProcessByMonth::operator()(const Time& t, const double component[3]){
			if ((  (statisticsSummary.statistics.count(0)!=0)
			     ||(statisticsSummary.statistics.count(1)!=0)
			     ||(statisticsSummary.statistics.count(2)!=0)
			    )
			    && (t>=endTime))
				addToSummary(); // reset statistics and its count to 0

			if (  (statisticsSummary.statistics.count(0)==0)
			    &&(statisticsSummary.statistics.count(1)==0)
			    &&(statisticsSummary.statistics.count(2)==0)) {
				statisticsSummary.first=t;
				endTime = statisticsSummary.first.endOfMonth();
			}
			statisticsSummary.last=t;
			statisticsSummary.statistics(component);
}

ProcessByYear::~ProcessByYear(){}
void ProcessByYear::operator()(const Time& t, const double component[3]){
			if ((  (statisticsSummary.statistics.count(0)!=0)
			     ||(statisticsSummary.statistics.count(1)!=0)
			     ||(statisticsSummary.statistics.count(2)!=0)
			    )
			    && (t>=endTime))
				addToSummary(); // reset statistics and its count to 0

			if (  (statisticsSummary.statistics.count(0)==0)
			    &&(statisticsSummary.statistics.count(1)==0)
			    &&(statisticsSummary.statistics.count(2)==0)) {
				statisticsSummary.first=t;
				endTime = statisticsSummary.first.endOfYear();
			}
			statisticsSummary.last=t;
			statisticsSummary.statistics(component);
}

