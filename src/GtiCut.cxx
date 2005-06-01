/**
 * @file GtiCut.cxx
 * @brief Encapsulate a GTI.
 * @author J. Chiang
 *
 * $Header: /nfs/slac/g/glast/ground/cvs/dataSubselector/src/GtiCut.cxx,v 1.3 2004/12/08 20:40:23 jchiang Exp $
 */

#include "dataSubselector/GtiCut.h"

namespace dataSubselector {

bool GtiCut::accept(tip::ConstTableRecord & row) const {
   double time;
   row["TIME"].get(time);
   return accept(time);
}

bool GtiCut::accept(const std::map<std::string, double> & params) const {
   std::map<std::string, double>::const_iterator it;
   if ( (it = params.find("TIME")) != params.end()) {
      return accept(it->second);
   }
   return true;
}

bool GtiCut::equals(const CutBase & arg) const {
   try {
      GtiCut & rhs = dynamic_cast<GtiCut &>(const_cast<CutBase &>(arg));
      return !(m_gti != rhs.m_gti);
   } catch (...) {
      return false;
   }
}

void GtiCut::getKeyValues(std::string & type, std::string & unit,
                          std::string & value, std::string & ref) const {
   type = "TIME";
   unit = "s";
   value = "TABLE";
   ref = ":GTI";
}

bool GtiCut::accept(double time) const {
   return m_gti.accept(time);
}

} // namespace dataSubselector
