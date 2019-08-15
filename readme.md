QDC "Quasi-Definitive Compare"
=============================

Application QDC
Version     0.1.0
Author      Peter Crosthwaite; for INTERMAGNET http:://www.intermagnet.org
Copyright   Geoscience Australia

QDC reports statistics of the difference between DataSetA and
DataSetB, where DataSetA and DataSetB are geomagnetic observatory 1-minute
vector data sets in INTERMAGNET IAF binary file format or IAGA-2002 format.

This repository includes a pre-compiled 64 bit windows executable (QDC.exe),
code and makefile for compiliation on other systems. To compile the
software the Boost "program-options" library is  required. See "makefile" 
for some further discussion.

Usage
-----
QDC
<<Command Line options>>:
  -? [ --help ]               Show all help (version, usage, about, example)
  --version                   Show version of this program
  --usage                     Show how to invoke this program
  --about                     Show information about this program
  --example                   Show an example invocation of this program
  -E [ --environment ] arg    Use prefixed environment variables

<<General options>>:
  -F [ --configuration ] arg  Use configuration file
  -a [ --dsa ] arg            data set A source file format:filename
  -b [ --dsb ] arg            data set B source file format:filename
  --ByDayByAll                Report data By Day By All
  --ByDayByMonthByAll         Report data By Day By Month By All
  --ByDayByMonthByYearByAll   Report data By Day By Month By Year By All
  --ByDayByYearByAll          Report data By Day By Year All
  --ByMonthByAll              Report data By Month By All
  --ByMonthByYearByAll        Report data By Month By Year By All
  --ByYearByAll               Report data By Year By All
  --ByDayByYear               Report data By Day By Year
  --ByDayByMonthByYear        Report data By Day By Month By Year
  --ByMonthByYear             Report data By Month By Year
  --ByDayByMonth              Report data By Day By Month
  --ByDay                     Report data By Day
  --ByMonth                   Report data By Month
  --ByYear                    Report data By Year
  --ByAll                     Report data By All
  -x [ --XYZ ]                Compare in XYZ

About
-----
QDC = Quasi Definitive Comparison; written for geomagnetic observatory data comparison.

QDC reports statistics of the vector difference DataSetA-DataSetB.

Each data set consists of 1 or more files specified as "format:filename".
DataSetA might be DEFINITIVE data and DataSetB might be QUASIDEFINITIVE data,
however they are only required to have the same recording interval (e.g. 1-minute).
The data sets must also have the same recorded elements (e.g. "HDZ*") unless they
are converted to "XYZ" using the --XYZ option.

Recognised file formats      are {iaga2002, iaf}.
Recognised recorded elements are {XYZ*, HDZ*}.

There should be no data gaps (except those represented by MISSING data values).

Other uses:
        Compare preliminary data from two variometers at one station.
        Compare definitive data from two nearby stations.
        etc.

Please report BUGS and modification requests to geomag@ga.gov.au

Example
-------
QDC -a iaf:cnb09dec.bin -b iaga2002:CNB200912010000pmin.min -b iaga2002:CNB200912020000pmin.min --ByDayByMonth --XYZ

        Convert the data to XYZ, and report the Monthly statistics of the Daily averages.
        All days with any amount of data are used.  NO "90%" RULE IS APPLIED, nor is any other rule.

QDC -FF -EQDC
        where a text file named F contains
                dsa=iaf:cnb09dec.bin
                dsb=iaga2002:CNB200912010000pmin.min
                dsb=iaga2002:CNB200912020000pmin.min
                XYZ=true
        and where an environment variable is set as follows
                set QDCByDayByMonth=true

        Same as previous example.

QDC -a iaf:cnb09jan.bin -b iaga2002:CNB200901010000pmin.min --ByDayByMonthbyYear

        Report the Yearly statistics of the Monthly average of the Daily averages.
        All months with any amount of data are used, as are all days with any amount of data.
        NO "90%" RULE IS APPLIED, nor is any other rule.
