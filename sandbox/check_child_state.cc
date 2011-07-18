#include<iostream>
#include<map>

#define NUM_FORKS 5
struct state {
  int ready;
};

#define INIT_STATE(s) s.ready = -1

bool children_are_ready(std::map<int, int*> &pfds,
                        std::map<int, state> &states) {
  while(1) {
    int sum = 0;
    for(int i=0; i < NUM_FORKS; ++i) {
      read(pfds[i][0],&states[i],sizeof(states[i]));
      sum += states[i].ready;
    }
    if (sum >= NUM_FORKS) {
      break;
    }
    sleep(1);
  }
  return true;
}


int main(int argc, char **argv) {

  int pid;
  std::map<int, int*> pfds;
  std::map<int, state> states;

  for(int i=0; i < NUM_FORKS; ++i) {
    // pipe
    pfds[i] = (int*)malloc(sizeof(int) * 2);
    pipe(pfds[i]);

    // state 初期化
    INIT_STATE(states[i]);

    pid = fork();
    if(pid == 0) { //子プロセス

      close(pfds[i][0]); // close read fd
      sleep(1);
      struct state wbuf = {-1};
      wbuf.ready = 1;
      write(pfds[i][1],&wbuf,sizeof(wbuf));
      close(pfds[i][1]); // close write fd
      sleep(10);
      exit(0);
    } else {
      close(pfds[i][1]); // close write fd
      continue;
    }
  }

  if (children_are_ready(pfds, states)) {
    std::cout << "ready" << std::endl;
  }

  for(int i=0; i < NUM_FORKS; ++i) {
    close(pfds[i][0]);
  }



  return 0;
}

