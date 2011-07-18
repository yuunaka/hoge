#ifndef _MPB_CONTROLLER_
#define _MPB_CONTROLLER_

#include<string>
#include<map>
#include<vector>

#include"common.hh"
#include"MPBLogicFactory.hh"
#include"MPBILogic.hh"

namespace mpb {

class MPBController {
public:
  MPBController(int argc, char** argv);
  ~MPBController();

  //  int setLogic(MPBLogic logic);
  int ready();
  int go();

private:
  int _concurrency;
  int _num_request;
  std::string _inputfile;
  std::string _type;
  std::map<int, int*> _pfds_c_p;
  std::map<int, int*> _pfds_p_c;
  std::map<int, state> _c_states;
  std::vector<std::vector<std::string> > _requests;
  MPBILogic* _logic;
  MPBLogicFactory* _lfactory;

  result* _results;

  int _init(int argc, char** argv);
  int _child(const std::vector<std::string> &reqs, int* pfd_c_p, int* pfd_p_c);
  int _child_sendReady(const int* pfd);
  int _child_sendResult(const int* pfd, const result &rt);

  int _parent_waitReady();
  int _parent_waitResult();
  int _parent_sendEvent();
  void _usage(char** argv);

};

}
#endif // _MPB_CONTROLLER_

