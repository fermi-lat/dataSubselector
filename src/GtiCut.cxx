/**
 * @file GtiCut.cxx
 *
 */

#include "dataSubselector/Cuts.h"

namespace dataSubselector {

bool Cuts::GtiCut::accept(tip::ConstTableRecord & row) const {
   double time;
   row["TIME"].get(time);
   return accept(time);
}

bool Cuts::GtiCut::accept(const std::map<std::string, double> & params) const {
   std::map<std::string, double>::const_iterator it;
   if ( (it = params.find("TIME")) != params.end()) {
      return accept(it->second);
   }
   return true;
}

bool Cuts::GtiCut::operator==(const CutBase & arg) const {
   try {
      GtiCut & rhs = dynamic_cast<GtiCut &>(const_cast<CutBase &>(arg));
      return !(m_gti != rhs.m_gti);
   } catch (...) {
      return false;
   }
}

void Cuts::GtiCut::
getKeyValues(std::string & type, std::string & unit,
             std::string & value, std::string & ref) const {
   type = "TIME";
   unit = "s";
   value = "TABLE";
   ref = ":GTI";
}

bool Cuts::GtiCut::accept(double time) const {
   return m_gti.accept(time);
}

} // namespace dataSubselector
