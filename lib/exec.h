#pragma once

#include <atomic>
#include <csignal>
#include <cstdio>
#include <fcntl.h>
#include <fstream>
#include <functional>
#include <memory>
#if defined(_MSC_VER)
#include <Windows.h>
#else
#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/stream.hpp>
#include <iostream>
#include <sys/select.h>
#include <unistd.h>
#endif

namespace Exec
{
extern std::atomic_bool now_exec;
using CloseFunc = std::function<void()>;

#if defined(_MSC_VER)
using IStream = std::ifstream;
#else
namespace io  = boost::iostreams;
using IStream = io::stream<io::file_descriptor_source>;
#endif
using IStreamPtr = std::shared_ptr<IStream>;

//
class Handle
{
public:
#if defined(_MSC_VER)
  using Type                 = HANDLE;
  static constexpr Type Null = nullptr;
#else
  using Type                 = int;
  static constexpr Type Null = 0;
#endif
  virtual ~Handle()               = default;
  virtual Type getRead() const    = 0;
  virtual Type getWrite() const   = 0;
  virtual void closeRead()        = 0;
  virtual void closeWrite()       = 0;
  virtual bool validRead() const  = 0;
  virtual bool validWrite() const = 0;

  virtual IStreamPtr getIStream() const = 0;
};
using HandlePtr = std::shared_ptr<Handle>;

//
HandlePtr createPipe();
//
void setup();
//
void setCloseFunc(CloseFunc);
//
bool check();
//
#if defined(_MSC_VER)
namespace
{
std::string
getCommandLine()
{
  return std::string{};
}
template <typename... Args>
std::string
getCommandLine(const char* e, Args const&... args)
{
  return std::string{e ? e : ""} + " " + getCommandLine(args...);
}
} // namespace
//
bool execute(HandlePtr, std::string);
#else
void setlaunchpid(int);
#endif
//
template <typename... Args>
HandlePtr
run(const char* e, Args const&... args)
{
  if (now_exec)
    return HandlePtr();
  auto p = createPipe();
  if (p->validRead() == false)
    return HandlePtr();
#if defined(_MSC_VER)
  //
  // for Windows
  //
  if (!execute(p, getCommandLine(e, args...)))
    return HandlePtr();
#else
  //
  // for UNIX/Mac
  //
  int pid = fork();
  if (pid == 0)
  {
    // child
    p->closeRead();
    dup2(p->getWrite(), 1);
    p->closeWrite();
    execl(e, e, args...);
    if (errno != 0)
      perror(strerror(errno));
    exit(0);
  }
  else if (pid < 0)
    return HandlePtr();

  setlaunchpid(pid);
  p->closeWrite();
#endif
  now_exec = true;
  return p;
}
//
template <typename Func>
bool
input(HandlePtr h, Func func)
{
  if (now_exec)
  {
#if defined(_MSC_VER)
    auto r = WaitForSingleObject(h->getRead(), 0);
    if (r == WAIT_OBJECT_0)
    {
      func();
      return true;
    }
#else
    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(h->getRead(), &rfds);
    timeval tv;
    tv.tv_sec = 0;
    tv.tv_sec = 0;
    if (select(1, &rfds, nullptr, nullptr, &tv) != 0)
    {
      func();
      return true;
    }
#endif
  }
  return false;
}

} // namespace Exec
