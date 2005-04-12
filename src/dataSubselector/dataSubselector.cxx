/** 
 * @file dataSubselector.cxx
 * @brief Filter FT1 data.
 * @author J. Chiang
 *
 *  $Header: /nfs/slac/g/glast/ground/cvs/dataSubselector/src/dataSubselector/dataSubselector.cxx,v 1.14 2005/04/06 20:33:04 jchiang Exp $
 */

#include "facilities/Util.h"

#include "st_facilities/Util.h"

#include "st_app/AppParGroup.h"
#include "st_app/StApp.h"
#include "st_app/StAppFactory.h"

#include "st_facilities/FitsUtil.h"

#include "tip/IFileSvc.h"
#include "tip/Table.h"

#include "CutController.h"

using dataSubselector::CutController;

/**
 * @class DataFilter
 * @author J. Chiang
 *
 * $Header: /nfs/slac/g/glast/ground/cvs/dataSubselector/src/dataSubselector/dataSubselector.cxx,v 1.14 2005/04/06 20:33:04 jchiang Exp $
 */

class DataFilter : public st_app::StApp {
public:
   DataFilter() : st_app::StApp(), 
                  m_pars(st_app::StApp::getParGroup("gtselect")) {
      try {
         m_pars.Prompt();
         m_pars.Save();
         setName("gtselect");
         setVersion(s_cvs_id);
  } catch (std::exception & eObj) {
         std::cerr << eObj.what() << std::endl;
         std::exit(1);
      } catch (...) {
         std::cerr << "Caught unknown exception in DataFilter constructor." 
                   << std::endl;
         std::exit(1);
      }
   }

   virtual ~DataFilter() throw() {
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

   std::string m_inputFile;
   std::string m_outputFile;

   void copyTable(const std::string & extension,
                  CutController * cutController=0) const;

   static std::string s_cvs_id;

};

std::string DataFilter::s_cvs_id("$Name:  $");

st_app::StAppFactory<DataFilter> myAppFactory("gtselect");

void DataFilter::banner() const {
   int verbosity = m_pars["chatter"];
   if (verbosity > 2) {
      st_app::StApp::banner();
   }
}

void DataFilter::run() {
   std::string inputFile = m_pars["input_file"];
   m_inputFile = inputFile;
   facilities::Util::expandEnvVar(&m_inputFile);

   std::string outputFile = m_pars["output_file"];
   m_outputFile = outputFile;
   facilities::Util::expandEnvVar(&m_outputFile);
   bool clobber = m_pars["clobber"];
   if (!clobber && st_facilities::Util::fileExists(m_outputFile)) {
      std::cout << "Output file, " << outputFile << ", already exists, "
                << "and you have specified 'clobber' as 'no'.\n"
                << "Please provide a different file name." << std::endl;
   } 

   tip::IFileSvc::instance().createFile(m_outputFile, m_inputFile);

   CutController * cuts = CutController::instance(m_pars, m_inputFile);
   copyTable("EVENTS", cuts);
   copyTable("gti");
   cuts->updateGti(m_outputFile);
   unsigned int verbosity = m_pars["chatter"];
   if (verbosity > 1) {
      std::cout << "Done." << std::endl;
   }
   CutController::delete_instance();

   st_facilities::FitsUtil::writeChecksums(m_outputFile);
}

void DataFilter::copyTable(const std::string & extension,
                           CutController * cuts) const {
   const tip::Table * inputTable 
      = tip::IFileSvc::instance().readTable(m_inputFile, extension);
   
   tip::Table * outputTable 
      = tip::IFileSvc::instance().editTable(m_outputFile, extension);

   outputTable->setNumRecords(inputTable->getNumRecords());

   tip::Table::ConstIterator inputIt = inputTable->begin();
   tip::Table::Iterator outputIt = outputTable->begin();

   tip::ConstTableRecord & input = *inputIt;
   tip::Table::Record & output = *outputIt;

   long npts(0);

   for (; inputIt != inputTable->end(); ++inputIt) {
      if (!cuts || cuts->accept(input)) {
         output = input;
         ++outputIt;
         npts++;
      }
   }
// Resize output table to account for filtered rows.
   outputTable->setNumRecords(npts);

   if (cuts) {
      cuts->writeDssKeywords(outputTable->getHeader());
   }

   delete inputTable;
   delete outputTable;
}
