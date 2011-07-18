#ifndef _MPB_COMMON_
#define _MPB_COMMON_

#include<iostream>

namespace mpb {

struct state {
  int ready; // ready:1
};

#define INIT_STATE(s) s.ready = -1

struct result {
  int flag;
  int num_reqs;
  int total_time;
  float min;
  float max;
  float ave;
};
#define DEBUG
#ifdef DEBUG
#define DP(fmt, args...)    fprintf(stderr, fmt, ## args)
#else
#define DP(fmt, args...)    /* Don't do anything in release builds */
#endif

}

#endif //_MPB_COMMON_


