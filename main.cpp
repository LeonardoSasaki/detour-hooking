/*
 * Simple detour hooking made for x86
 * I don't plan in making a x64 version anytime 
 * soon, this should be enough to get the idea
 */

#include <iostream>
#include <cstdint>

#if defined _WIN32
#include <Windows.h>
#elif defined __linux__
#include <sys/mman.h>
#endif

bool jmp_hook(unsigned char* func, unsigned char* dst)
{
#ifdef _WIN32
    char original_bytes[5];
    DWORD old_protection;
    if (VirtualProtect(func, 5, PAGE_EXECUTE_READWRITE, &old_protection) == 0)
        return false;

    memcpy(original_bytes, func, 5);
#elif defined __linux__
    if (mprotect(func, 5, PROT_EXEC | PROT_READ | PROT_WRITE) != 0)
            return false;
#endif
    
    *func = 0xE9; //relative jmp near instruction
    *reinterpret_cast<uintptr_t*>(func + 1) = dst - func - 5;
    
#ifdef _WIN32
    if (!VirtualProtect(func, 5, old_protection, &old_protection)) {
        memcpy(func, original_bytes, 5);
        return false;
    }
#endif
    return true;
}

#ifdef __linux__
int example_function(const char* text) {
    std::cout << text << std::endl;
    return 0;
}
#endif

int
#ifdef _WIN32
__stdcall hooked_function(
        HWND    hWnd,
        LPCSTR lpText,
        LPCSTR lpCaption,
        UINT    uType) {
    std::cout << "Call to MessageBox redirected, parameters passed: "
        << hWnd << ", " << lpText << ", " << lpCaption << ", " << uType << std::endl;
#elif defined __linux__
hooked_function(
        const char* text){
    std::cout << "hooked! original text: " << text << std::endl;
#endif
    return 0;
}

int main(int argc, const char* argv[]) {
#ifdef __linux__
    example_function("test");
    jmp_hook(reinterpret_cast<unsigned char*>(example_function), reinterpret_cast<unsigned char*>(hooked_function));
    example_function("hello world");
#elif defined _WIN32
    auto messagebox_address = reinterpret_cast<unsigned char*>(GetProcAddress(
        GetModuleHandleA("user32.dll"), "MessageBoxA"));
    jmp_hook(messagebox_address, reinterpret_cast<unsigned char*>(hooked_function));
    MessageBoxA(0, "hello world", "test call", MB_OK);
#endif
    return 0;
}
