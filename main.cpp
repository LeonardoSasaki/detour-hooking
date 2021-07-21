/*
 * Simple detour hooking made for x86
 * I don't plan to make a x64 version anytime 
 * soon, this should be enough to get the idea
 */

#include <iostream>
#include <cstdint>

#ifdef __GNUC__
#define NOINLINE __attribute__((noinline))
#elif defined _MSC_VER
#define NOINLINE __declspec(noinline)
#endif

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>

#elif defined __linux__
#include <sys/mman.h>
#include <unistd.h>
#endif

bool 
jmp_hook (unsigned char *func, unsigned char *dst)
{
#ifdef defined(_WIN32) || defined(_WIN64)
  char original_bytes[5];
  DWORD old_protection;
    
  if (0 == VirtualProtect (func, 5, PAGE_EXECUTE_READWRITE, &old_protection))
    return false;

  memcpy (original_bytes, func, 5);
  
#elif defined __linux__
  uintptr_t page_size = sysconf (_SC_PAGE_SIZE);
  if(0 != mprotect (func - ((uintptr_t) func % page_size), page_size, PROT_EXEC | PROT_READ | PROT_WRITE))
    return false;
#endif
    
  *func = 0xE9; //relative jmp near instruction
  *(uint32_t *) (func + 1) = dst - func - 5;
    
#ifdef defined(_WIN32) || defined(_WIN64)
  if (!VirtualProtect (func, 5, old_protection, &old_protection))
    {
      memcpy (func, original_bytes, 5);
      return false;
    }
#endif
    return true;
}

#ifdef __linux__
NOINLINE int
example_function (char *text)
{
  std::cout << text << std::endl;
  return 0;
}
#endif

int
#ifdef defined(_WIN32) || defined(_WIN64)
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

#elif defined(_WIN32) || defined(_WIN64)
  auto messagebox_address = (unsigned char *) GetProcAddress (GetModuleHandleA ("user32.dll"), "MessageBoxA"));
  jmp_hook (messagebox_address, (unsigned char *) hooked_function);
  MessageBoxA (0, "hello world", "test call", MB_OK);
#endif
  return 0;
}
