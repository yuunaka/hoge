#include"MPBController.hh"
#include"MPBLogicFactory.hh"
#include"MPBILogic.hh"
#include<cstdio>
#include<unistd.h>
#include<fstream>
#include<iostream>
#include<sys/epoll.h>
#include<fcntl.h>
#include<errno.h>
#include<assert.h>

namespace mpb {

  MPBController::MPBController(int argc, char** argv) :
    _concurrency(1),
    _num_request(10),
    _type(std::string("sample")) {

    _init(argc, argv);
  }

// int MPBController::setLogic(MPBLogic logic) {
//   _logic = logic;
//   return 1;
// }

int MPBController::ready() {
  int pid;
  std::vector<std::vector<std::string> >::iterator it = _requests.begin();
  for(int i=0; i<_concurrency; i++) {
    // 双方向ipc用pipe
    _pfds_c_p[i] = new int[2];
    _pfds_p_c[i] = new int[2];
    pipe(_pfds_c_p[i]);
    pipe(_pfds_p_c[i]);

    // state 初期化
    INIT_STATE(_c_states[i]);
    // fork
    DP("fork\n");
    pid = fork();
    assert(pid < 0);
    if(pid == 0) {
      _child(*(it + i),_pfds_c_p[i],_pfds_p_c[i]);
    } else {
      close(_pfds_c_p[i][1]); // close write fd
      close(_pfds_p_c[i][0]); // close read fd
    }
  }

  // parent
  return _parent_waitReady();
}

int MPBController::go() {
  // epoll event
  // sleep(10);
  _parent_sendEvent();

  //
  _parent_waitResult();

  // show results
  return 1;
}


int MPBController::_init(int argc, char** argv) {
  int opt;

  while(( opt = getopt(argc, argv, "c:i:n:t:")) != -1) {
    switch(opt) {
    case 'c':
      _concurrency = atoi(optarg);
      break;
    case 'i':
      _inputfile = std::string(optarg);
      break;
    case 'n':
      _num_request = atoi(optarg);
      break;
    case 't':
      _type = std::string(optarg);
      break;
    // case 'd':
    //   debug = true;
    //   break;
    default:
      _usage(argv);
    }
  }

  if(_inputfile.empty()) {
    _usage(argv);
  }

  //logic factory
  _lfactory = new MPBLogicFactory();
  //results
  _results = new result[_concurrency];

  // ファイルからリクエストデータを読み込む
  // リクエスト内容を重複させないために、十分にたくさんのデータを読み込ませた方が良い
  std::ifstream ifs;
  ifs.open(_inputfile.c_str());
  std::string line;
  std::vector<std::string> requests;
  while(std::getline(ifs,line)) {
    requests.push_back(line);
  }
  ifs.close();

  // _num_request個のrequestを_concurrencyセット作る
  std::vector<std::string>::iterator it = requests.begin();
  for(int i=0; i<_concurrency; i++) {
    std::vector<std::string> tmp_reqs;
    int j = 0;
    while (j < _num_request) {
      if (it==requests.end()) {
        it = requests.begin();
      }
      tmp_reqs.push_back(*it);
      ++it;
      j++;
    }
    _requests.push_back(tmp_reqs);
  }
  return 1;
}

void MPBController::_usage(char** argv) {
  std::cerr << "usage:" << std::endl;
  std::cerr << argv[0] << " -c concurrency -n num_request per process -t logic type -i inputfile" << std::endl;
  exit(1);
}

/*
**
** for parent process
**
*/
int MPBController::_parent_waitReady() {
  while(1) {
    for(int i=0; i < _concurrency; ++i) {
      read(_pfds_c_p[i][0],&_c_states[i],sizeof(_c_states[i]));
    }
    bool flag = true;
    for(int i=0; i < _concurrency; ++i) {
      DP("ready %d\n",_c_states[i].ready);
      if(_c_states[i].ready < 1) {
        flag = false;
      }
    }
    if (flag) {
      break;
    }
    sleep(1);
  }
  return 1;
}

int MPBController::_parent_waitResult() {
  while(1) {
    int sum = 0;
    for(int i=0; i < _concurrency; ++i) {
      read(_pfds_c_p[i][0],&_results[i],sizeof(_results[i]));
      sum += _results[i].flag;
    }
    if (sum >= _concurrency) {
      break;
    }
    sleep(1);
  }
  return 1;
}

int MPBController::_parent_sendEvent() {
  int ret = 0;
  char event = '"';
  for(int i=0; i < _concurrency; ++i) {
    ret = write(_pfds_p_c[i][1],&event,sizeof(event));
    assert(ret < 0);
  }
  return 1;
}


/*
**
** for child process
**
*/

  int MPBController::_child(const std::vector<std::string> &reqs, int* pfd_c_p, int* pfd_p_c) {
    close(pfd_c_p[0]); // close read fd
    close(pfd_p_c[1]); // close write fd

    MPBILogic* logic = _lfactory->createInstance(_type, reqs);

    // epoll ready
    int epfd = epoll_create(1);

    // set nonblock
    int flags;
    flags = fcntl(pfd_p_c[0], F_GETFL);
    flags |= O_NONBLOCK;
    fcntl(pfd_p_c[0], F_SETFL, flags);

    // edge trigger
    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = pfd_p_c[0];
    epoll_ctl(epfd, EPOLL_CTL_ADD, pfd_p_c[0], &ev);

    // send ready
    _child_sendReady(pfd_c_p);

    // epoll
    DP("wait epoll event\n");
    int ret = epoll_wait(epfd, &ev, 1, -1);
    if(ret < 0) {
      DP("failed to epoll:%d errno:%d\n",ret,errno);
    }
    DP("recv a event\n");
    // execute
    logic->execute();
    struct result rt = logic->getResult();
    _child_sendResult(pfd_c_p,rt);

    close(pfd_c_p[1]); // close write fd
    close(pfd_p_c[0]); // close read fd
    exit(0);
  }

int MPBController::_child_sendReady(const int* pfd) {
      struct state wbuf;
      INIT_STATE(wbuf);

      wbuf.ready = 1;
      write(pfd[1],&wbuf,sizeof(wbuf));
      return 1;
}

int MPBController::_child_sendResult(const int* pfd, const result &rt) {
      write(pfd[1],&rt,sizeof(rt));
      return 1;
}

}
