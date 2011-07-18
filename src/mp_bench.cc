#include"MPBController.hh"

int main(int argc, char** argv) {
  mpb::MPBController* ctl = new mpb::MPBController(argc, argv);
  ctl->ready();
  ctl->go();
  return 1;

}
