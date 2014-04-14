/**
 * @file BitMaskCut.h
 * @brief Cuts based on a single bit mask applied to an unsigned long
 * column.  This is used for filtering on the EVENT_CLASS column for
 * Pass 7 IRFs and later.
 *
 * @author J. Chiang
 *
 * $Header: /nfs/slac/g/glast/ground/cvs/ScienceTools-scons/dataSubselector/dataSubselector/BitMaskCut.h,v 1.4 2014/04/14 16:12:44 jchiang Exp $
 */

#ifndef dataSubselector_BitMaskCut_h
#define dataSubselector_BitMaskCut_h

#include "dataSubselector/CutBase.h"

namespace dataSubselector {

/**
 * @class BitMaskCut
 *
 */

class BitMaskCut : public CutBase {

public:

   BitMaskCut(const std::string & colname,
              unsigned long bitPosition,
              const std::string & pass_ver="");

   virtual ~BitMaskCut() {}

   virtual bool accept(tip::ConstTableRecord & row) const;

   virtual bool accept(const std::map<std::string, double> & params) const;

   virtual CutBase * clone() const {return new BitMaskCut(*this);}

   virtual bool supercedes(const CutBase & cut) const;

   virtual std::string filterString() const;

   const std::string & colname() const {
      return m_colname;
   }
   
   unsigned long bitPosition() const {
      return m_bitPosition;
   }

   const std::string & pass_ver() const {
      return m_pass_ver;
   }

   static bool post_P7(const std::string & pass_ver);

protected:

   virtual bool equals(const CutBase & rhs) const;

   virtual void getKeyValues(std::string & type, std::string & unit,
                             std::string & value, std::string & ref) const;

private:

   std::string m_colname;
   unsigned long m_bitPosition;
   unsigned long m_mask;

   std::string m_pass_ver;

   bool m_post_P7;

   bool accept(unsigned long value) const;

};

} // namespace dataSubselector

#endif // dataSubselector_BitMaskCut_h
