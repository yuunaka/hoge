#ifndef _MPB_LOGIC_FACTORY_
#define _MPB_LOGIC_FACTORY_

#include<string>
#include<vector>

#include"MPBILogic.hh"

namespace mpb {

class MPBLogicFactory {
public:
  MPBLogicFactory();
  ~MPBLogicFactory();

  MPBILogic* createInstance(const std::string &type,
                            const std::vector<std::string> &requests);

};

}
#endif // _MPB_LOGIC_FACTORY_
