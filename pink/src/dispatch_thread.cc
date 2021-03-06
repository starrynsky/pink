#include "pink/src/dispatch_thread.h"

#include "pink/src/pink_item.h"
#include "pink/src/pink_epoll.h"
#include "pink/src/worker_thread.h"

namespace pink {

DispatchThread::DispatchThread(int port,
    int work_num, Thread **worker_thread,
    int cron_interval, const ServerHandle* handle) :
  ServerThread::ServerThread(port, cron_interval, handle),
  last_thread_(0),
  work_num_(work_num),
  worker_thread_(reinterpret_cast<WorkerThread **>(worker_thread)) {
  StartWorkerThreads();
}

DispatchThread::DispatchThread(const std::string &ip, int port,
    int work_num, Thread **worker_thread,
    int cron_interval, const ServerHandle* handle) :
  ServerThread::ServerThread(ip, port, cron_interval, handle),
  last_thread_(0),
  work_num_(work_num),
  worker_thread_(reinterpret_cast<WorkerThread **>(worker_thread)) {
  StartWorkerThreads();
}

DispatchThread::DispatchThread(const std::set<std::string>& ips, int port,
    int work_num, Thread **worker_thread,
    int cron_interval, const ServerHandle* handle) :
  ServerThread::ServerThread(ips, port, cron_interval, handle),
  last_thread_(0),
  work_num_(work_num),
  worker_thread_(reinterpret_cast<WorkerThread **>(worker_thread)) {
  StartWorkerThreads();
}

void DispatchThread::HandleNewConn(const int connfd, const std::string& ip_port) {
  std::queue<PinkItem> *q = &(worker_thread_[last_thread_]->conn_queue_);
  PinkItem ti(connfd, ip_port);
  {
    slash::MutexLock l(&worker_thread_[last_thread_]->mutex_);
    q->push(ti);
  }
  write(worker_thread_[last_thread_]->notify_send_fd(), "", 1);
  last_thread_++;
  last_thread_ %= work_num_;
}

void DispatchThread::StartWorkerThreads() {
  for (int i = 0; i < work_num_; i++) {
    worker_thread_[i]->StartThread();
  }
}

extern ServerThread *NewDispatchThread(
    int port,
    int work_num, Thread **worker_thread,
    int cron_interval,
    const ServerHandle* handle) {
  return new DispatchThread(port, work_num, worker_thread,
      cron_interval, handle);
}
extern ServerThread *NewDispatchThread(
    const std::string &ip, int port,
    int work_num, Thread **worker_thread,
    int cron_interval,
    const ServerHandle* handle) {
  return new DispatchThread(ip, port, work_num, worker_thread,
      cron_interval, handle);
}
extern ServerThread *NewDispatchThread(
    const std::set<std::string>& ips, int port,
    int work_num, Thread **worker_thread,
    int cron_interval,
    const ServerHandle* handle) {
  return new DispatchThread(ips, port, work_num, worker_thread,
      cron_interval, handle);
}


};  // namespace pink
