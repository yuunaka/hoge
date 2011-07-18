#ifndef _MPB_CONTROLLER_
#define _MPB_CONTROLLER_

class MPBController {
public:
  Controlller::Controlller(int argc, char** argv);
  ~Controlller::Controlller();

  int setLogic(MPBLogic logic);
  int ready();
  int go();

private:
  unsigned int _concurrency;
  MPBLogic _logic;

  int _init(int argc, char** argv);

}
#endif // _MPB_CONTROLLER_
