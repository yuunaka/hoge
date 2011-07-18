#ifndef _MPB_CONTROLLER_
#define _MPB_CONTROLLER_

#include<string>
#include<map>
#include<vector>

#include"common.hh"
// #include"MPBLogicFactory.hh"
// #include"MPBILogic.hh"

namespace mpb {

  /*!\brief benchプロセスを制御するクラス
   *
   *
   */
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

    // pipes foreach children(child to parent)
    std::map<int, int*> _pfds_c_p;
    // pipes foreach children(parent to child)
    std::map<int, int*> _pfds_p_c;
    // states of children
    std::map<int, state> _c_states;
    // requests foreach children
    std::vector<std::vector<std::string> > _requests;
    // bench results foreach children
    result* _results;

    void _usage(char** argv);
    int _init(int argc, char** argv);

    int _child(const std::vector<std::string> &reqs, int* pfd_c_p, int* pfd_p_c);
    int _child_sendReady(const int* pfd);
    int _child_sendResult(const int* pfd, const result &rt);

    int _parent_waitReady();
    int _parent_waitResult();
    int _parent_sendEvent();
  };

}
#endif // _MPB_CONTROLLER_

