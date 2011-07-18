#include<iostream>

#include"MPBSampleLogic.hh"
#include"common.hh"

namespace mpb {
    SampleLogic::SampleLogic(const std::vector<std::string> &requests) : _requests(requests) {
      // prepare someting
    };

  /*!\brief 
   *
   *
   */
  SampleLogic::~SampleLogic() {};

  /*!\brief 
   *
   *
   */
  int SampleLogic::execute() {
    sleep(1);
    DP("SampleLogic::execute()\n");
    std::vector<std::string>::iterator it = _requests.begin();
    for(;it!=_requests.end();++it) {
      std::cout << *it << std::endl;
    }
    return 1;
  }

  /*!\brief 
   *
   *
   */
  result SampleLogic::getResult() {
    DP("SampleLogic::getResult()\n");
    struct result test = {
      (int)1,
      (int)10,
      (int)10,
      (float)0.1,
      (float)3.5,
      (float)1,
    };
    return test;
  }

}
