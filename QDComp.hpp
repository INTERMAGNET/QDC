#ifndef QDCComp_hpp
#define QDCComp_hpp

#include <stdlib.h>
#include <math.h>
#include <limits>

#include <iostream>
#include <iomanip>
#include <list>
#include <vector>
#include <algorithm>
#include <functional>

using namespace std;
extern "C" {
#include "gmday.h"
}


class Time;

class TimeInterval {
	friend class Time;
	friend ostream& operator <<(ostream& o, TimeInterval &t);
	public:
		TimeInterval(long t):time(t){}
		TimeInterval():time(0){}
		TimeInterval(int theYear, int theMonth, int theDay, int theHour, int theMinute, int theSecond){
			time = theDay*86400 + theHour*3600 + theMinute*60 + theSecond;
			// ignore months and days at the moment until TimeInterval is generalised for irregular time series
		}
		TimeInterval& operator=(long d){time=d; return *this;}
		bool operator> (const TimeInterval& rhs) const { if (time> rhs.time) return true; return false;}
		bool operator<=(const TimeInterval& rhs) const { return !operator>(rhs);}
		bool operator< (const TimeInterval& rhs) const { if (time< rhs.time) return true; return false;}
		bool operator>=(const TimeInterval& rhs) const { return !operator<(rhs);}
		bool operator==(const TimeInterval& rhs) const { if (time==rhs.time) return true; return false;}
		bool operator!=(const TimeInterval& rhs) const { return !operator==(rhs);}

		typedef long DivType;
		DivType operator/(TimeInterval denominator)    {return time/denominator.time;}
		TimeInterval operator/(long d)                 {return TimeInterval(time/d);}
		TimeInterval operator*(long d)                 {return TimeInterval(time*d);}
	private:
		long time;
};

inline ostream& operator <<(ostream& o, TimeInterval &t) {
	long l=0, r=t.time;
	if (r) {
		l=r/86400; r%= 86400; /*if (l)     */o<<l<<"T";
		l=r/3600;  r%= 3600;  /*if (l)     */o<<l<<":";
		l=r/60;    r%= 60;    /*if (l || r)*/o<<l<<":"<<r;
	} else {
		o<<"0T0:0:0";
	}
	return o;
}

class Time {

	// untested for times < 2000/1/1T0:0:0

	friend class TimeInterval;
	//friend ostream& operator <<(ostream& o, const Time &t);
	friend ostream& operator <<(ostream& o, Time t);
	public:
		Time(long t):time(t){}
		Time():time(0){}
		Time(int theYear, int theMonth, int theDay, int theHour, int theMinute, int theSecond){
			long e2Day = gm_day_e2(theYear, theMonth, theDay);
			time = e2Day*86400 + theHour*3600 + theMinute*60 + theSecond;
		}

		int year(){return 0;}
		int month(){return 0;}
		int day(){return 0;}
		int hour(){return 0;}
		int minute(){return 0;}
		int second(){return 0;}

		Time endOfDay(){
			return Time(((time/86400)+1)*86400);  //start of next day
		}
		Time endOfMonth(){
			int year, month, day;
			gm_day_e2r (time/86400, &year, &month, &day);
			if (month==12) {++year; month=1;} else {++month;}
			//cout <<"EOM:Y"<<year<<"M"<<month<<"D"<<day<<";";
			return Time(year,month,1,0,0,0); // start of next month
		}
		Time endOfYear(){
			int year, month, day;
			gm_day_e2r (time/86400, &year, &month, &day);
			++year;
			//cout <<"EOY:Y"<<year<<"M"<<month<<"D"<<day<<";";
			return Time(year,1,1,0,0,0); // start of next year
		}


		Time& operator=(double d){time=d; return *this;}
		bool operator> (const Time& rhs) const { if (time> rhs.time) return true; return false;}
		bool operator<=(const Time& rhs) const { return !operator>(rhs);}
		bool operator< (const Time& rhs) const { if (time< rhs.time) return true; return false;}
		bool operator>=(const Time& rhs) const { return !operator<(rhs);}
		bool operator==(const Time& rhs) const { if (time==rhs.time) return true; return false;}
		bool operator!=(const Time& rhs) const { return !operator==(rhs);}

		TimeInterval operator-(const Time& rhs) const {return TimeInterval(time-rhs.time);}
		Time operator+(TimeInterval& rhs) const {return Time(time+rhs.time);}
		Time operator-(TimeInterval& rhs) const {return Time(time-rhs.time);}
		Time& operator+=(TimeInterval& rhs) {time+=rhs.time; return *this;}
		Time& operator-=(TimeInterval& rhs) {time-=rhs.time; return *this;}
	private:
		long time;
};

inline ostream& operator <<(ostream& o, Time t) {
	int year, month, day;
	long l=0, r=t.time;

	l=r/86400; r%= 86400; gm_day_e2r (l, &year, &month, &day);
	                      o<<year<<'-'<<month<<'-'<<day<<'T';
	l=r/3600;  r%= 3600;  o<<l<<":";
	l=r/60;    r%= 60;    o<<l<<":"<<r;
	return o;
}

class Data {
	public:
		static const size_t DIMENSION=3;
		static constexpr double INVALID=99999.9L;

		typedef enum {Unspecified, XYZ, HDZ, Unknown} Format;

		Time t;
		double component[3];

		Data(Time time, double a, double b, double c): t(time) {
			component[0]=a; /*X nT            */
			component[1]=b; /*Y nT, D degrees */
			component[2]=c; /*Z nT            */
		}
		bool equalTime(Data d)  {return t == d.t;}
};

inline bool equalTime(Data d, Time t){return d.t == t;}

class SingleSource {
	public:
		Data::Format format;
		Time first;
		Time last;
		TimeInterval timeIncrement;
		vector<Data> data;
		typedef vector<Data>::iterator iterator;

		SingleSource(Data::Format fmt, size_t n, TimeInterval inc=TimeInterval(0,0,0,0,1,0)):format(fmt), timeIncrement(inc){
			data.reserve(n);
		}

		SingleSource(Data::Format fmt, Time t1, Time t2, TimeInterval inc=TimeInterval(0,0,0,0,1,0))
		:format(fmt), first(t1), last(t2), timeIncrement(inc) {

			data.reserve((last-first)/timeIncrement);
			nIdent=++N;
			while (t1 <= t2) {
				data.push_back(Data(t1,nIdent+10+rand(),2.5*nIdent-1.3*rand(),-9*nIdent+2.4*rand())); t1+=timeIncrement;
			}
			cout<<"SingleSource "<<first<<" "<<last<<" "<<format<<" "<<data.begin()->t<<" "<<(data.end()-1)->t<<endl;
		}

		void truncate(){/* truncate the data vector somehow */}
		void convertToXYZ(){
			// M_PI is POSIX but not C++ or C++11, so for C++11 use this  instead
          constexpr long double M_PI=4.L*atan(1.L);
			switch (format) {
				case Data::HDZ:
					for (vector<Data>::iterator i=data.begin(); i!=data.end(); ++i) {
						double H = i->component[0], D= (i->component[1]/60.)*(M_PI/180.); // sometimes M_TWOPI is defined, sometimes not!
						i->component[0] = H * cos(D);
						i->component[1] = H * sin(D);
					}
					format=Data::XYZ;
					break;
				case Data::XYZ:
				default:
					break;
			}
		}

		static int N;
	private:
		int nIdent;
};

class MultiSource {
	public:

		list<SingleSource*> dataList;
		typedef list<SingleSource*>::iterator SourceIter;
		typedef SingleSource::iterator       DataIter;

		void convertToXYZ() {
			for (list<SingleSource*>::iterator i = dataList.begin(); i != dataList.end(); ++i) (*i)->convertToXYZ();
		}

	private:


};


class CommonSourceSegment {
	public:
		CommonSourceSegment(SingleSource& sA, MultiSource::DataIter dA, SingleSource& sB, MultiSource::DataIter dB, size_t nInCommon)
		:sourceA(sA), dataIterA(dA), sourceB(sB), dataIterB(dB), n(nInCommon) {}

		vector<Data>::iterator dataIterA;
		vector<Data>::iterator dataIterB;
		size_t n;

		SingleSource& sourceA; // for metadata
		SingleSource& sourceB; // for metadata
};









class ProcessStatistics {
	public:
		static const size_t DIMENSION=3;

		void reset() {
			for (size_t i=0; i<DIMENSION; ++i) {
				A[i]=Q[i]=0;
				maximumValue[i]=-numeric_limits<double>::max();
				minimumValue[i]= numeric_limits<double>::max(); // not all that satisfactory, but min() is minimum representable positive value
				n[i]=0;
			}
		}
		ProcessStatistics(){ reset(); };

		double minimum         (size_t i){ return minimumValue[i]; }
		double maximum         (size_t i){ return maximumValue[i]; }
		double mean            (size_t i){ return A[i]; }
		double sampleVariance  (size_t i){ if (n[i]>1) return Q[i]/(n[i]-1); else return 0;}
		double standardVariance(size_t i){ if (n[i]>0) return Q[i] /n[i]   ; else return 0;}

		double sampleStandardError(size_t i) {return sqrt(sampleVariance  (i));}
		double standardError(size_t i)       {return sqrt(standardVariance(i));}

		size_t count(size_t i) { return n[i]; }
		void operator()(const double component[3]) {
			//++n; wrong now that individual channels may have INVALID values
			double A_, Q_;
			for (size_t i=0; i<DIMENSION; ++i) {
				if (component[i]!=Data::INVALID) {
					++n[i];
					A_=A[i]; A[i] = A_ + (component[i]-A_)/n[i];
					Q_=Q[i]; Q[i] = Q_ + (component[i]-A_)*(component[i]-A[i]);
					if (component[i]>maximumValue[i]) maximumValue[i] = component[i];
					if (component[i]<minimumValue[i]) minimumValue[i] = component[i];
				}
			}
		}

	private:
		double A [DIMENSION];
		double Q [DIMENSION];
		double maximumValue[DIMENSION];
		double minimumValue[DIMENSION];
		size_t n[DIMENSION];
};

class Process {
	// process a data stream in supplied in pieces
	public:
		virtual void operator()(const Time& t, const double component[3])=0;
		void operator()(){
			addToSummary();
			for (list<Process *>::iterator i=passOnList.begin(); i!= passOnList.end(); ++i) {
				(*i)->operator()();
			}

		}
		void report();

		Process(string id):identifier(id){}
		virtual ~Process();

		Process& passOn(Process& p){passOnList.push_back(&p);}

	protected:
		const string identifier;
		//void reportPassOn() {
		//	for (list<Process *>::iterator i=passOnList.begin(); i!= passOnList.end(); ++i) (*i)->report();
		//}
		class SummaryItem{
			public:
				//SummaryItem(Time f, Time l, ProcessStatistics s):first(f), last(l), statistics(s){}//???

				Time first;
				Time last;
				Time midTime;
				ProcessStatistics statistics;
				void setMidTime() {
					TimeInterval interval= (last-first)/2;  // const stops this in 1 line ???
					midTime = first + interval;
				}

		} statisticsSummary;

		void addToSummary(){
			if (  statisticsSummary.statistics.count(0)
			    ||statisticsSummary.statistics.count(1)
			    ||statisticsSummary.statistics.count(2) // Fix this to use DIMENSION instead of 0 1 and 2
			   ) {
				statisticsSummary.setMidTime();
				summaryOfStatistics.push_back(statisticsSummary);

				for (list<Process *>::iterator i=passOnList.begin(); i!= passOnList.end(); ++i) {
					double mean[Data::DIMENSION];
					for (size_t j=0; j<Data::DIMENSION; ++j) mean[j]=statisticsSummary.statistics.mean(j);
					(*i)->operator()(statisticsSummary.midTime, mean);
				}
			}
			statisticsSummary.statistics.reset();
		}

	private:
		list<Process *> passOnList;
		vector<SummaryItem> summaryOfStatistics;


};





class ProcessAll : public Process {
	public:
		ProcessAll(string id): Process(id){}
		virtual ~ProcessAll();
		virtual void operator()(const Time& tm, const double component[3]);
		void operator()() { Process::operator()();} // why is this required by compiler???
};

class ProcessByDay : public Process {
	public:
		ProcessByDay(string id):Process(id){}
		virtual ~ProcessByDay();
		virtual void operator()(const Time& t, const double component[3]);
		void operator()() { Process::operator()();}

	private:
		Time endTime;
};

class ProcessByMonth : public Process {
	public:
		ProcessByMonth(string id):Process(id){}
		virtual ~ProcessByMonth();
		virtual void operator()(const Time& t, const double component[3]);
		void operator()() { Process::operator()();}

	private:
		Time endTime;
};

class ProcessByYear : public Process {
	public:
		ProcessByYear(string id):Process(id){}
		virtual ~ProcessByYear();
		virtual void operator()(const Time& t, const double component[3]);
		void operator()() { Process::operator()();}

	private:
		Time endTime;
};


#endif
