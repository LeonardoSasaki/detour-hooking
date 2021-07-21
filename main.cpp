// Reminder: _WIN32 is still defined in x64 projects (at least in MSVC)

#include <iostream>
#include <stdlib.h>
#include <cstdint>

#ifdef __GNUC__
#define NOINLINE __attribute__((noinline))
#elif defined _MSC_VER
#define NOINLINE __declspec(noinline)
#endif

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#elif defined(__linux__)
#include <sys/mman.h>
#include <unistd.h>
#else
#error "This code is meant to run on Windows/Linux"
#endif

bool 
jmp_hook (unsigned char *func, unsigned char *dst)
{
#if defined(_WIN32)
  char original_bytes[16];
  DWORD old_protection;

  if (0 == VirtualProtect (func, 1024, PAGE_EXECUTE_READWRITE, &old_protection))
    return false;

  memcpy (original_bytes, dst, sizeof(void*) == 4 ? 5 : 14);
#elif defined(__linux__)
  uintptr_t page_size = sysconf (_SC_PAGE_SIZE);
  if (0 != mprotect (func - ((uintptr_t) func % page_size), page_size, PROT_EXEC | PROT_READ | PROT_WRITE))
    return false;
#endif

#if defined(_M_X64) || defined(__amd64__) // x86_64
  func[0] = 0xFF; // absolute jmp
  func[1] = 0x25; // absolute jmp
  *(uint32_t*)(func+2) = 0;
  *(uint64_t*)(func+6) = (uint64_t)dst;
#else
  *func = 0xE9; //relative jmp near instruction
  *(uint32_t *) (func + 1) = dst - func - 5;
#endif

#if defined(_WIN32)
  if (!VirtualProtect (func, 1024, old_protection, &old_protection))
    {
      memcpy (func, original_bytes, sizeof(void*) == 4 ? 5 : 14);
      return false;
    }
#endif
  return true;
}

#if defined(__linux__)
NOINLINE int
example_function (char *text)
{
  std::cout << text << std::endl;
  return 0;
}
#endif

int
#if defined(_WIN32)
__stdcall
hooked_function (
  HWND    hWnd,
  LPCSTR  lpText,
  LPCSTR  lpCaption,
  UINT    uType)
{
  std::cout << "Call to MessageBox redirected, parameters passed: " << hWnd
    << ", " << lpText << ", " << lpCaption << ", " << uType << std::endl;

#elif defined __linux__
hooked_function (char *text)
{
  std::cout << "hooked! original text: " << text << std::endl;
#endif
  return 0;
}

int
main (int argc, char **argv)
{
#ifdef __linux__
  example_function ("test");
  jmp_hook ((unsigned char *) example_function, (unsigned char *) hooked_function);
  example_function ("hello world");
#elif defined _WIN32
  auto user32 = GetModuleHandleA ("user32.dll");

  if (nullptr == user32)
    {
      std::cout << "couldn't find user32.dll" << std::endl;
      return EXIT_FAILURE;
    }

  auto messagebox_address = (unsigned char *) GetProcAddress (user32, "MessageBoxA");
  jmp_hook (messagebox_address, (unsigned char *) hooked_function);
  MessageBoxA (0, "hello world", "test call", MB_OK);
#endif
  return EXIT_SUCCESS;
}
