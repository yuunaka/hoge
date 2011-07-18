#include<iostream>

#include"MPBSampleLogic.hh"
#include"common.hh"

namespace mpb {
  
SampleLogic::SampleLogic(const std::vector<std::string> &requests) {};

SampleLogic::~SampleLogic() {};

int SampleLogic::execute() {
  DP("SampleLogic::execute()\n");
  std::cout << "call execute" << std::endl;
  return 1;
}

result SampleLogic::getResult() {
  DP("SampleLogic::getResult()\n");
  struct result test = {
    (int)1,
    (int)10,
    (int)10,
    (float)10,
    (float)10,
    (float)10,
  };
  return test;
}

}
