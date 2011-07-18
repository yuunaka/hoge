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


  /*!\brief コンストラクタ
   * コマンドラインパラメータを渡す
   * \param[in] argc
   * \param[in] argv
   */
  MPBController::MPBController(int argc, char** argv) :
    _concurrency(1),
    _num_request(10),
    _type(std::string("sample")) {

    _init(argc, argv);
  }


  /*!\brief デストラクタ
   *
   *
   */
  MPBController::~MPBController() {
  }


  /*!\brief bench処理を行う子プロセスの準備
   * - forkする
   * - named pipe を生成する
   * - 全ての子プロセスからの準備完了通知を待つ
   * \retval 1 成功
   * \retval <0 失敗
   */
  int MPBController::ready() {

    std::vector<std::vector<std::string> >::iterator it = _requests.begin();
    for(int i=0; i<_concurrency; i++) {

      // 双方向ipc用pipe
      _pfds_c_p[i] = new int[2];
      _pfds_p_c[i] = new int[2];
      pipe(_pfds_c_p[i]);
      pipe(_pfds_p_c[i]);

      int pid;
      pid = fork();
      assert(pid > -1);
      if(pid == 0) { // child
        _child(*(it + i),_pfds_c_p[i],_pfds_p_c[i]);
      } else {       // parent
        close(_pfds_c_p[i][1]); // close write fd
        close(_pfds_p_c[i][0]); // close read fd
      }
    }

    // wait ready to go
    return _parent_waitReady();
  }

  /*!\brief benchを実行し集計
   * - 子プロセスが監視(epoll)しているfdにイベントを送る
   * - 子プロセスからの結果を待つ
   * \retval 1 成功
   * \retval <0 失敗
   */
  int MPBController::go() {
    // sleep(10);
    int ret;
    ret = _parent_sendEvent();
    if(ret < 0) {
      return ret;
    }

    ret = _parent_waitResult();
    if(ret < 0) {
      return ret;
    }

    // show results
    return 1;
  }

  /*!\brief メンバの初期化
   * - コマンドラインオプションの解析
   * - メンバ変数の初期化
   * - inputファイルの読み込み
   */
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
    // 同時に_c_statesを初期化する
    std::vector<std::string>::iterator it = requests.begin();
    for(int i=0; i<_concurrency; i++) {
      INIT_STATE(_c_states[i]);
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

  /*!\brief usage    */
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

  /*!\brief 子プロセスが準備完了状態になるのを待つ
   * 
   *
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


  /*!\brief 
   *
   *
   */
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

  /*!\brief 
   *
   *
   */
  int MPBController::_parent_sendEvent() {
    int ret = 0;
    char event = '"';
    for(int i=0; i < _concurrency; ++i) {
      ret = write(_pfds_p_c[i][1],&event,sizeof(event));
      assert(ret > -1);
    }
    return 1;
  }


  /*
  **
  ** for child process
  **
  */

  /*!\brief 
   *
   *
   */
  int MPBController::_child(const std::vector<std::string> &reqs,
                            int* pfd_c_p,
                            int* pfd_p_c) {

    close(pfd_c_p[0]); // close read fd
    close(pfd_p_c[1]); // close write fd

    MPBLogicFactory factory;
    MPBILogic* logic = factory.createInstance(_type, reqs);

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
    delete logic;
    exit(0);
  }


  /*!\brief 
   *
   *
   */
  int MPBController::_child_sendReady(const int* pfd) {
    struct state wbuf;
    INIT_STATE(wbuf);

    wbuf.ready = 1;
    write(pfd[1],&wbuf,sizeof(wbuf));
    return 1;
  }

  /*!\brief 
   *
   *
   */
  int MPBController::_child_sendResult(const int* pfd, const result &rt) {
    write(pfd[1],&rt,sizeof(rt));
    return 1;
  }

















}
