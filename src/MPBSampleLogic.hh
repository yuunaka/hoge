#ifndef _MPB_SAMPLE_LOGIC_
#define _MPB_SAMPLE_LOGIC_
#include<string>
#include<vector>

#include"MPBILogic.hh"
#include"common.hh"

namespace mpb {
class SampleLogic : public MPBILogic {
public:
  SampleLogic(const std::vector<std::string> &requests);
  ~SampleLogic();
  int execute();
  result getResult();
private:
  std::vector<std::string> _requests;
};

}
#endif // _MPB_SAMPLE_LOGIC_
