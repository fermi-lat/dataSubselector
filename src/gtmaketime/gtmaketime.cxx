/**
 * @file gtmaketime.cxx
 * @brief Create a GTI extension based on a filter expression applied
 * to a spacecraft data file and merge with the existing GTI in the
 * event data file.
 * @author J. Chiang
 *
 * $Header: /nfs/slac/g/glast/ground/cvs/dataSubselector/src/gtmaketime/gtmaketime.cxx,v 1.15 2007/06/18 18:07:02 jchiang Exp $
 */

#include <iomanip>
#include <iostream>
#include <memory>

#include "st_stream/StreamFormatter.h"

#include "st_app/AppParGroup.h"
#include "st_app/StApp.h"
#include "st_app/StAppFactory.h"

#include "tip/IFileSvc.h"
#include "tip/Table.h"

#include "st_facilities/Util.h"

#include "dataSubselector/Cuts.h"
#include "dataSubselector/Gti.h"
#include "dataSubselector/GtiCut.h"

/**
 * @class MakeTime
 * @author J. Chiang
 *
 */

class MakeTime : public st_app::StApp {
public:
   MakeTime() : st_app::StApp(),
                m_pars(st_app::StApp::getParGroup("gtmktime")) {
      try {
         setVersion(s_cvs_id);
      } catch (std::exception & eObj) {
         std::cerr << eObj.what() << std::endl;
         std::exit(1);
      } catch (...) {
         std::cerr << "Caught unknown exception in Maketime constructor." 
                   << std::endl;
         std::exit(1);
      }
   }

   virtual ~MakeTime() throw() {
      try {
      } catch (std::exception &eObj) {
         std::cerr << eObj.what() << std::endl;
      } catch (...) {
      }
   }

   virtual void run();

   virtual void banner() const;

private:
   
   st_app::AppParGroup & m_pars;
   dataSubselector::Gti m_gti;
   double m_tmin;
   double m_tmax;
   std::string m_evfile;
   std::string m_outfile;

   void check_outfile();
   void findTimeLims();
   void createGti();
   void mergeGtis();
   void makeUserGti(std::vector<const dataSubselector::GtiCut*>&gtiCuts) const;
   void copyTable() const;

   static std::string s_cvs_id;
};

std::string MakeTime::s_cvs_id("$Name:  $");

st_app::StAppFactory<MakeTime> myAppFactory("gtmktime");

void MakeTime::banner() const {
   int verbosity = m_pars["chatter"];
   if (verbosity > 2) {
      st_app::StApp::banner();
   }
}

void MakeTime::run() {
   m_pars.Prompt();
   m_pars.Save();
   check_outfile();
   findTimeLims();
   createGti();
   mergeGtis();
   copyTable();
}

void MakeTime::check_outfile() {
   st_stream::StreamFormatter formatter("MakeTime", "run", 2);
   bool clobber = m_pars["clobber"];
   std::string outfile = m_pars["outfile"];
   m_outfile = outfile;
   if (outfile == "") {
      formatter.err() << "Please specify an output file name." 
                      << std::endl;
      std::exit(1);
   }
   if (!clobber && st_facilities::Util::fileExists(outfile)) {
      formatter.err() << "Output file " << outfile 
                      << " exists and clobber is set to 'no'.  Exiting."
                      << std::endl;
      std::exit(1);
   }
}

void MakeTime::findTimeLims() {
   std::string evfile = m_pars["evfile"];
   std::string evtable = m_pars["evtable"];
   const tip::Table * evTable
      = tip::IFileSvc::instance().readTable(evfile, evtable);
   const tip::Header & header(evTable->getHeader());
   header["TSTART"].get(m_tmin);
   header["TSTOP"].get(m_tmax);
}

void MakeTime::createGti() {
   std::string scfile = m_pars["scfile"];
   std::vector<std::string> scfiles;
   st_facilities::Util::resolve_fits_files(scfile, scfiles);
   std::string sctable = m_pars["sctable"];
   std::string filter = m_pars["filter"];

   std::ostringstream event_time_range;
   event_time_range << std::setprecision(20);
   event_time_range << " && (START >= " << m_tmin
                    << ") && (STOP <= " << m_tmax << ")";
   filter += event_time_range.str();

   st_stream::StreamFormatter formatter("MakeTime", "createGti", 3);
   formatter.info() << "Applying GTI filter:\n" << filter << std::endl;

   for (size_t i = 0; i < scfiles.size(); i++) {
      std::auto_ptr<const tip::Table> 
         in_table(tip::IFileSvc::instance().readTable(scfiles.at(i), 
                                                      sctable, filter));
   
      tip::Table::ConstIterator input = in_table->begin();
      tip::ConstTableRecord & in = *input;

      double start_time;
      double stop_time;
      std::vector<double> tstart;
      std::vector<double> tstop;
// Initialize arrays with the first interval that ends after the first
// event time, m_tmin
      for (; input != in_table->end(); ++input) {
         in["START"].get(start_time);
         in["STOP"].get(stop_time);
         if (stop_time > m_tmin) {
            tstart.push_back(start_time);
            tstop.push_back(stop_time);
            break;
         }
      }
      ++input;
// Gather remaining intervals, consolidating adjacent ones.
      for (; input != in_table->end(); ++input) {
         in["START"].get(start_time);
         in["STOP"].get(stop_time);
         if (start_time > m_tmax) { // break out if past end of evfile
            break;
         }
         if (start_time == tstop.back()) {
            tstop.back() = stop_time;
         } else {
            tstart.push_back(start_time);
            tstop.push_back(stop_time);
         }
      }
// Insert each contiguous interval in the Gti object
      for (size_t i(0); i < tstart.size(); i++) {
         dataSubselector::Gti gti;
         gti.insertInterval(tstart.at(i), tstop.at(i));
         m_gti = m_gti | gti;
      }
   }
}

void MakeTime::mergeGtis() {
   std::string evfile = m_pars["evfile"];
   m_evfile = evfile;
   std::string evtable = m_pars["evtable"];
   
   bool checkColumns = m_pars["apply_filter"];
   dataSubselector::Cuts cuts(evfile, evtable, checkColumns);

   std::vector<const dataSubselector::GtiCut *> gtiCuts;

   bool overwrite = m_pars["overwrite"];
   if (overwrite) {
      makeUserGti(gtiCuts);
   } else {
      cuts.getGtiCuts(gtiCuts);
   }

   for (size_t i = 0; i < gtiCuts.size(); i++) {
      m_gti = m_gti & gtiCuts.at(i)->gti();
   }
}

void MakeTime::
makeUserGti(std::vector<const dataSubselector::GtiCut *> & gtiCuts) const {
   double tstart = m_pars["tstart"];
   double tstop = m_pars["tstop"];
   bool useHeader = m_pars["header_obstimes"];
   std::string extension = m_pars["evtable"];
   if (useHeader) {
      const tip::Table * evTable 
         = tip::IFileSvc::instance().readTable(m_evfile, extension);
      const tip::Header & header(evTable->getHeader());
      header["TSTART"].get(tstart);
      header["TSTOP"].get(tstop);
   }
   dataSubselector::Gti myGti;
   myGti.insertInterval(tstart, tstop);
   gtiCuts.clear();
   gtiCuts.push_back(new dataSubselector::GtiCut(myGti));
}

void MakeTime::copyTable() const {
   tip::IFileSvc::instance().createFile(m_outfile, m_evfile);

   std::string extension = m_pars["evtable"];
   const tip::Table * inputTable 
      = tip::IFileSvc::instance().readTable(m_evfile, extension);
   
   tip::Table * outputTable 
      = tip::IFileSvc::instance().editTable(m_outfile, extension);

   outputTable->setNumRecords(inputTable->getNumRecords());

   tip::Table::ConstIterator inputIt = inputTable->begin();
   tip::Table::Iterator outputIt = outputTable->begin();

   tip::ConstTableRecord & input = *inputIt;
   tip::Table::Record & output = *outputIt;

   long npts(0);

   dataSubselector::Cuts my_cuts;
   my_cuts.addGtiCut(m_gti);

   bool applyFilter = m_pars["apply_filter"];
   for (; inputIt != inputTable->end(); ++inputIt) {
      if (!applyFilter || my_cuts.accept(input)) {
         output = input;
         ++outputIt;
         npts++;
      }
   }
   delete inputTable;

// Resize output table to account for filtered rows.
   outputTable->setNumRecords(npts);

   m_gti.writeExtension(m_outfile);

//    bool notPhdu;
//    st_facilities::Util::writeDateKeywords(outputTable, m_gti.minValue(),
//                                           m_gti.maxValue(), 
//                                           notPhdu=true);

//    tip::Image * phdu(tip::IFileSvc::instance().editImage(m_outfile, ""));
//    st_facilities::Util::writeDateKeywords(phdu, m_gti.minValue(),
//                                           m_gti.maxValue(), 
//                                           notPhdu=false);
//    delete phdu;

//    tip::Table * gtiTable 
//       = tip::IFileSvc::instance().editTable(m_outfile, "GTI");
//    st_facilities::Util::writeDateKeywords(gtiTable, m_gti.minValue(),
//                                           m_gti.maxValue(), 
//                                           notPhdu=true);
//    delete gtiTable;

   delete outputTable;
}

/** 
 * June 18, 2007:  optimization of GTI accumulation in run() method.

Timing before optimization: 

ki-rh2[jchiang] time ../../rhel4_gcc34/gtmktime.exe 
Spacecraft data file [test_scData.fits] : 
Filter expression [IN_SAA!=T] : 
Event data file [test_events_0000.fits] : 
Output event file name [filtered.fits] : 
229.168u 0.182s 3:57.25 96.6%   0+0k 0+0io 0pf+0w

After optimization to createGti():
ki-rh2[jchiang] time ../../rhel4_gcc34/gtmktime.exe
Spacecraft data file [test_scData.fits] : 
Filter expression [IN_SAA!=T] : 
Event data file [test_events_0000.fits] : 
Output event file name [filtered.fits] : filtered_new.fits
4.693u 0.173s 0:10.87 44.7%     0+0k 0+0io 0pf+0w

 */
