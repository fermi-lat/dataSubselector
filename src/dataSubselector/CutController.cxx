/**
 * @file CutController.cxx
 *
 * $Header$
 */

#include <stdexcept>

#include "facilities/Util.h"
#include "st_app/AppParGroup.h"
#include "tip/Table.h"
#include "astro/SkyDir.h"

#include "dataSubselector/CutController.h"
#include "dataSubselector/Gti.h"

namespace dataSubselector {

CutController * CutController::s_instance(0);

CutController * 
CutController::instance(st_app::AppParGroup & pars, 
                        const std::vector<std::string> & eventFiles,
                        const std::string & evtable) {
   if (!s_instance) {
      s_instance = new CutController(pars, eventFiles, evtable);
   }
   return s_instance;
}

void CutController::delete_instance() {
   delete s_instance;
   s_instance = 0;
}

CutController::CutController(st_app::AppParGroup & pars, 
                             const std::vector<std::string> & eventFiles,
                             const std::string & evtable) 
   : m_pars(pars), m_cuts(eventFiles, evtable, true, true) {
   double ra = pars["ra"];
   double dec = pars["dec"];
   double radius = pars["rad"];
   double max_rad = 180.;
   if (radius < max_rad) {
      m_cuts.addSkyConeCut(ra, dec, radius);
   }
   addRangeCut("TIME", "s", pars["tmin"], pars["tmax"]);
   addRangeCut("ENERGY", "MeV", pars["emin"], pars["emax"]);
   addRangeCut("PHI", "deg", pars["phimin"], pars["phimax"]);
   addRangeCut("THETA", "deg", pars["thetamin"], pars["thetamax"]);
   addRangeCut("ZENITH_ANGLE", "deg", pars["zmin"], pars["zmax"]);
   int eventClass = pars["eventClass"];
   if (eventClass >= 0 && eventClass < 4) {
      addRangeCut("EVENT_CLASS", "dimensionless", eventClass, eventClass,
                  0, true);
   }
   if (eventClass == 4) { // Class A events only
      addRangeCut("EVENT_CLASS", "dimensionless", 0, 1, 0, true);
   }
   double phasemin = pars["phasemin"];
   double phasemax = pars["phasemax"];
   if (phasemin !=0 || phasemax !=1) {
      addRangeCut("PULSE_PHASE", "dimensionless", phasemin, phasemax);
   }
   m_cuts.mergeRangeCuts();
}

bool CutController::accept(tip::ConstTableRecord & row) const {
   double ra, dec;
   row["RA"].get(ra);
   row["DEC"].get(dec);
   return m_cuts.accept(row) && withinCoordLimits(ra, dec);
}

void CutController::addRangeCut(const std::string & colname,
                                const std::string & unit,
                                double minVal, double maxVal, 
                                unsigned int indx, bool force) {
   RangeCut::IntervalType type(RangeCut::CLOSED);
   if (!force && minVal == 0 && maxVal == 0) {
      /// don't apply any range cut
      return;
   }
   if (minVal > maxVal) {
      std::ostringstream message;
      message << "minimum requested value, " << minVal 
              << ", is greater than the maximum requested, "
              << maxVal << ", for field " << colname << "\n";
      throw std::runtime_error(message.str());
   }
   std::vector<std::string> tokens;
   facilities::Util::stringTokenize(colname, "[]", tokens);
   m_cuts.addRangeCut(tokens.at(0), unit, minVal, maxVal, type, indx);
}

bool CutController::withinCoordLimits(double ra, double dec) const {
   double lonMin = m_pars["lonMin"], lonMax = m_pars["lonMax"],
      latMin = m_pars["latMin"], latMax = m_pars["latMax"];
   if (m_pars["coordSys"] == "GAL") {
      astro::SkyDir dir(ra, dec);
      return (lonMin <= dir.l() && dir.l() <= lonMax && 
              latMin <= dir.b() && dir.b() <= latMax);
   } else {
      return (lonMin <= ra && ra <= lonMax && 
              latMin <= dec && dec <= latMax);
   }
   return false;
}

void CutController::updateGti(const std::string & eventFile) const {
   Gti gti(eventFile);
   for (unsigned int i = 0; i < m_cuts.size(); i++) {
      if (m_cuts[i].type() == "range") {
         const RangeCut & my_cut = 
            dynamic_cast<RangeCut &>(const_cast<CutBase &>(m_cuts[i]));
         if (my_cut.colname() == "TIME") {
            if (my_cut.intervalType() == RangeCut::CLOSED) {
               gti = gti.applyTimeRangeCut(my_cut.minVal(), my_cut.maxVal());
            } else if (my_cut.intervalType() == RangeCut::MINONLY) {
               gti = gti.applyTimeRangeCut(my_cut.minVal(), gti.maxValue());
            } else if (my_cut.intervalType() == RangeCut::MAXONLY) {
               gti = gti.applyTimeRangeCut(gti.minValue(), my_cut.maxVal());
            }
         }
      }
   }
   gti.writeExtension(eventFile);
}

std::string CutController::filterString() const {
   std::string filter(m_cuts.filterString());
   if (filter != "") {
      filter += " && ";
   }
   filter += "gtifilter()";
   return filter;
}

} // namespace dataSubselector
