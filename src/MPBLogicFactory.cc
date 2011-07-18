#include<string>
#include<vector>

#include"MPBLogicFactory.hh"
// Logics should be included here.
#include"MPBSampleLogic.hh"

namespace mpb {

MPBLogicFactory::MPBLogicFactory(){}
MPBLogicFactory::~MPBLogicFactory(){}

MPBILogic* MPBLogicFactory::createInstance(const std::string &type,
                          const std::vector<std::string> &requests) {
  if (! type.compare("sample")) {
    return new SampleLogic(requests);
  } else {
    return new SampleLogic(requests);
  }
}

}
