#ifndef _MPB_ILOGIC_
#define _MPB_ILOGIC_

#include"common.hh"

namespace mpb {
  
class MPBILogic {
public:
  //  MPBILogic() {};
  virtual ~MPBILogic(){};
  virtual int execute() = 0;
  virtual result getResult() = 0;
};

}
#endif // _MPB_ILOGIC_
