/**
 * @file CutController.h
 * @brief Manage the cuts specified in the par file.
 * @author J. Chiang
 *
 * $Header: /nfs/slac/g/glast/ground/cvs/dataSubselector/src/dataSubselector/CutController.h,v 1.5 2005/09/13 00:44:25 jchiang Exp $
 */

#ifndef dataSubselector_CutController_h
#define dataSubselector_CutController_h

#include <string>

#include "dataSubselector/Cuts.h"

namespace st_app {
   class AppParGroup;
}

namespace tip {
   class ConstTableRecord;
}

namespace dataSubselector {

/**
 * @class CutController
 * @brief Controller interface between application and CutBase hierarchy.
 * @author J. Chiang
 *
 * $Header: /nfs/slac/g/glast/ground/cvs/dataSubselector/src/dataSubselector/CutController.h,v 1.5 2005/09/13 00:44:25 jchiang Exp $
 */

class CutController {

public:

   static CutController * instance(st_app::AppParGroup & pars,
                                   const std::string & eventFile,
                                   const std::string & evtable);

   static void delete_instance();

   bool accept(tip::ConstTableRecord & row) const;

   void writeDssKeywords(tip::Header & header) const {
      m_cuts.writeDssKeywords(header);
   }

   void updateGti(const std::string & filename) const;

   std::string filterString() const;

protected:

   CutController::CutController(st_app::AppParGroup & pars,
                                const std::string & eventFile,
                                const std::string & evtable);

private:

   st_app::AppParGroup & m_pars;
   Cuts m_cuts;

   static CutController * s_instance;

   void addRangeCut(const std::string & colname, const std::string & unit,
                    double minVal, double maxVal, unsigned int indx=0,
                    bool force=false);

   bool CutController::withinCoordLimits(double ra, double dec) const;

};

} // namespace dataSubselector

#endif // dataSubselector_CutController_h
