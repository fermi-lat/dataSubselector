/**
 * @file Gti.h
 * @brief Handle GTIs. Derives from evtbin::Gti, adding a constructor and 
 * an accept() method.
 * @author J. Chiang
 *
 * $Header: /nfs/slac/g/glast/ground/cvs/dataSubselector/dataSubselector/Gti.h,v 1.2 2004/12/06 23:25:21 jchiang Exp $
 */

#ifndef dataSubselector_Gti_h
#define dataSubselector_Gti_h

#include "evtbin/Gti.h"

namespace dataSubselector {

/**
 * @class Gti
 *
 */

class Gti : public evtbin::Gti {

public:

   Gti() : evtbin::Gti() {}

   Gti(const std::string & filename, const std::string & extension="GTI") 
      : evtbin::Gti(filename, extension) {}

   Gti(const tip::Table & gtiTable);

   bool accept(double time) const;

   void writeExtension(const std::string & filename) const;

};

} // namespace dataSubselector

#endif // dataSubselector_Gti_h
