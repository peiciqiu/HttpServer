/*
 * Copyright ©2023 Travis McGaha.  All rights reserved.  Permission is
 * hereby granted to students registered for University of Pennsylvania
 * CIT 5950 for use solely during Spring Semester 2023 for purposes of
 * the course.  No other use, copying, distribution, or modification
 * is permitted without prior written consent. Copyrights for
 * third-party components of this work must be honored.  Instructors
 * interested in reusing these course materials should contact the
 * author.
 */

#include <unistd.h>

#include "gtest/gtest.h"
#include "../src/server/ThreadPool.h"
#include "../src/server/test_suite.h"


namespace searchserver {

uint32_t workcount = 0;
static pthread_mutex_t mtx;

// This is the function that each dispatched thread from the thread
// pool is sent to execute.
void TestTaskFn(ThreadPool::Task *t) {
  pthread_mutex_lock(&mtx);
  workcount++;
  pthread_mutex_unlock(&mtx);

  if (workcount % 5 == 1) {
    usleep(250000);  // 0.25s
  }
  delete t;
}

TEST(Test_ThreadPool, Basic) {
  pthread_mutex_init(&mtx, nullptr);
  ThreadPool *tp = new ThreadPool(10);

  // Try dispatching some work.  Make sure we dispatch enough that
  // there will be a queue of pending tasks in the threadpool, so
  // that we can test the "delete before all tasks are done" case.
  for (int i = 0; i < 300; i++) {
    ThreadPool::Task *next_t = new ThreadPool::Task(TestTaskFn);

    tp->dispatch(next_t);
  }
  usleep(1250000);  // 1.25s

  // Make sure that there are still tasks pending.
  ASSERT_GT((uint32_t) 300, workcount);

  // Kill off the threadpool, which should force the rest of the
  // pending tasks to be finished serially.
  delete tp;

  // Make sure all 300 tasks finished successfully.
  ASSERT_EQ((uint32_t) 300, workcount);
}

}  // namespace searchserver
