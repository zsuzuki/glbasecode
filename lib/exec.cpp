#include "exec.h"
#if defined(_MSC_VER)
#include <Windows.h>
#include <chrono>
#include <io.h>
#include <iostream>
#include <share.h>
#include <stdio.h>
#include <stdlib.h>
#include <thread>
#endif

namespace Exec
{
std::atomic_bool now_exec{false};

bool initialized = false;

namespace
{
CloseFunc close_func;

#if defined(_MSC_VER)
void
perror(const char* msg)
{
  LPVOID buffer;
  FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                nullptr, GetLastError(),
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&buffer, 0,
                nullptr);
  std::cerr << msg << ": " << buffer << std::endl;
  LocalFree(buffer);
}
#endif

//
// ハンドル実体
//
class HandleImpl : public Handle
{
protected:
  Type read;
  Type write;

public:
  HandleImpl(Type r = Null, Type w = Null) : read(r), write(w) {}
  ~HandleImpl() override
  {
    closeRead();
    closeWrite();
  }

  //
  void closeRead() override
  {
    if (read != Null)
    {
#if defined(_MSC_VER)
      CloseHandle(read);
#else
      close(read);
#endif
    }
    read = Null;
  }
  //
  void closeWrite() override
  {
    if (write != Null)
    {
#if defined(_MSC_VER)
      CloseHandle(write);
#else
      close(write);
#endif
    }
    write = Null;
  }
  //
  Type getRead() const override { return read; }
  Type getWrite() const override { return write; }
  bool validRead() const override { return read != Null; }
  bool validWrite() const override { return write != Null; }

  IStreamPtr getIStream() const override
  {
#if defined(_MSC_VER)
    auto  fid = _open_osfhandle((intptr_t)getRead(), _O_RDONLY);
    auto* fd  = _fdopen(fid, "r");
    return std::make_shared<std::ifstream>(fd);
#else
    return std::make_shared<IStream>(getRead(), io::close_handle);
#endif
  }
};

#if defined(_MSC_VER)
//
// for Windows
//
HANDLE child_proc = nullptr;

std::shared_ptr<std::thread> check_thread;

wchar_t*
s2ws(const std::string& s)
{
  int len;
  int slength  = (int)s.length() + 1;
  len          = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
  wchar_t* buf = new wchar_t[len];
  MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
  // std::wstring r(buf);
  // delete[] buf;
  // return r;
  return buf;
}

void
setup_local()
{
}
void
check_local()
{
  if (check_thread)
  {
    check_thread->join();
    check_thread.reset();
    std::cout << "terminate" << std::endl;
  }
}
#else
//
// for UNIX/Mac
//
int my_pid = -1;
void
sigchild(int signo)
{
  pid_t pid = 0;
  do
  {
    int ret;
    pid = waitpid(-1, &ret, WNOHANG);
    if (my_pid != -1 && pid == my_pid)
    {
      close_func();
      now_exec = false;
      my_pid = -1;
    }
  } while (pid > 0);
}
void
setup_local()
{
  if (initialized)
    return;
  struct sigaction act;
  memset(&act, 0, sizeof(act));
  act.sa_handler = sigchild;
  sigemptyset(&act.sa_mask);
  act.sa_flags = SA_NOCLDSTOP | SA_RESTART;
  sigaction(SIGCHLD, &act, nullptr);
  initialized = true;
}
void
check_local()
{
}
#endif
} // namespace

#if defined(_MSC_VER)
//
bool
execute(HandlePtr handle, std::string commandLine)
{
  BOOL  bInheritHandles = TRUE;
  DWORD creationFlags   = 0;

  STARTUPINFO si = {};
  si.cb          = sizeof(STARTUPINFO);
  si.dwFlags     = STARTF_USESTDHANDLES;
  si.hStdInput   = GetStdHandle(STD_INPUT_HANDLE);
  si.hStdOutput  = handle->getWrite();
  // si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
  si.hStdError = GetStdHandle(STD_ERROR_HANDLE);

  if (si.hStdOutput == INVALID_HANDLE_VALUE)
  {
    perror("GetStdHandle(STD_OUTPUT_HANDLE)");
    handle->closeRead();
    handle->closeWrite();
    return false;
  }
  if (si.hStdError == INVALID_HANDLE_VALUE)
  {
    perror("GetStdHandle(STD_ERROR_HANDLE)");
    handle->closeRead();
    handle->closeWrite();
    return false;
  }

  // 子プロセスを起動
  PROCESS_INFORMATION pi = {};
  if (!CreateProcess(nullptr, const_cast<char*>(commandLine.c_str()),
                     nullptr, //プロセスのセキュリティー記述子
                     nullptr, //スレッドのセキュリティー記述子
                     bInheritHandles, creationFlags,
                     nullptr, //環境変数は引き継ぐ
                     nullptr, //カレントディレクトリーは同じ
                     &si, &pi))
  {
    perror("CreateProcess");
    handle->closeRead();
    handle->closeWrite();
    return false;
  }
  child_proc = pi.hProcess;
  if (!CloseHandle(pi.hThread))
  {
    perror("CloseHandle(hThread)");
  }

  handle->closeWrite();

  check_thread = std::make_shared<std::thread>([]() {
    auto rp = WaitForSingleObject(child_proc, INFINITE);
    if (rp == WAIT_OBJECT_0)
    {
      std::cout << "wait done." << std::endl;
    }
    else
    {
      perror("wait failed");
    }
    CloseHandle(child_proc);
    child_proc = nullptr;
    now_exec   = false;
    close_func();
  });

  return true;
}
#else
//
void
setlaunchpid(int p)
{
  my_pid = p;
}
#endif
//
void
setup()
{
  setup_local();
}
//
bool
check()
{
  if (now_exec)
    return true;
  check_local();
  return false;
}
//
void
setCloseFunc(CloseFunc cf)
{
  close_func = cf;
}

//
HandlePtr
createPipe()
{
  Handle::Type p[2];
#if defined(_MSC_VER)
  HANDLE temp[2];

  if (CreatePipe(&temp[0], &temp[1], nullptr, 0))
  {
    if (!DuplicateHandle(GetCurrentProcess(), temp[1], GetCurrentProcess(),
                         &p[1], 0, TRUE, DUPLICATE_SAME_ACCESS))
    {
      perror("DuplicateHandle(write)");
    }
    if (!CloseHandle(temp[1]))
    {
      perror("CloseHandle(write temp)");
      p[1] = Handle::Null;
    }
    //
    p[0] = temp[0];
  }
  else
  {
    perror("CreatePipe");
    p[0] = Handle::Null;
    p[1] = Handle::Null;
  }
#else
  if (pipe(p) < 0)
  {
    p[0] = Handle::Null;
    p[1] = Handle::Null;
  }
#endif
  return std::make_shared<HandleImpl>(p[0], p[1]);
}

} // namespace Exec
