# detour hooking

### What is hooking?
Hooking is the concept of redirecting existing code flow. This makes possible to modify, block or change the behavior of a function or code at given address for many purposes, including defensive/offensive reasons like monitoring calls to a function, blocking dangerous actions, modifying/reading parameters passed or hiding information/code by faking results (or changing return values) to bypass antivirus checks for example

![output](https://github.com/LeonardoSasaki/detour-hooking/blob/main/img/output.png?raw=true)

### What is detour hooking?
Detour hooking is just one among plenty other hooking methods, like IAT, VMT, VEH, etc. Detours usually works by inserting a jmp instruction at a given address to redirect code execution. Even though a call instruction instead of a jmp is possible, it may not always fit the purpose due to the code returning to the original function, also it will overwrite your target function prologue if you choose to overwrite the beggining of it, which is another problem you would need to take care of.

In this code, we hooked two functions: MessageBoxA from WinAPI (if compiled on windows, obviously) and a function i wrote myself (in case you compile for linux). Take a look at MessageBoxA disassembly on memory before hooking:
![original](https://github.com/LeonardoSasaki/detour-hooking/blob/main/img/orig.png?raw=true)

And after hooking:
![hooked](https://github.com/LeonardoSasaki/detour-hooking/blob/main/img/hooked.png?raw=true)
As you can see, the beggining of the code changed to a jump to our function. Here is what i did: first, i got a pointer to MessageBoxA function address querying it using GetProcAddress, passing the first parameter as the handle to the module where this function is located (user32.dll) and the second parameter was the name of the function itself. Other methods you could use to get the target address you want is for example pattern scanning, this allows for finding not just a function, but a specific piece of code anywhere in the process. After i got the target function address, i changed page protection to be able to write to it (generally functions are compiled to .text segment for default, which doesnt has write permission) and wrote this piece of code: 
```
E9 xx xx xx xx
```
E9, which is the hexadecimal value for relative jump near instruction, followed by "xx xx xx xx", which is the offset to jump, being E9 (1 byte length) +  the offset (4 bytes length) 5 bytes long, here is how you calculate the offset: ``` destination - target function - 5 ```
By doing that, you will be overwriting 5 bytes with a jump to your code. Make sure your hooked function has the same prototype as the target function (return type, calling convention, parameters) or else things might go wrong. For example in my code, i replicated MessageBoxA:
```
int __stdcall hooked_function(
        HWND    hWnd,
        LPCSTR  lpText,
        LPCSTR  lpCaption,
        UINT    uType)
```
From original Microsoft documentation, we have:
```
int MessageBox(
  HWND    hWnd,
  LPCTSTR lpText,
  LPCTSTR lpCaption,
  UINT    uType)
```
##### *source: https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-messagebox*
You might have noticed that there are 2 differences: __stdcall and the function name. In fact, function name does not change anything at all. After compiled, every function is just code in memory, they don't exactly have names (except for exported functions, of course, but still they do not have their name on its body). You will get a compile error if you use the same name, since you're re-defining a existing function. And about the __stdcall, its just the calling convention. Every function in WinAPI uses stdcall for default,  you could even change it to "WINAPI" macro, with extends to __stdcall. I myself had the project settings to compile functions with __cdecl calling convention by default, but you  might be asking: "what is a calling convention?" From wikipedia,
> A calling convention is an implementation-level (low-level) scheme for how subroutines receive parameters from their caller and how they return a result. Differences in various implementations include where parameters, return values, return addresses and scope links are placed (registers, stack or memory etc.), and how the tasks of preparing for a function call and restoring the environment afterwards are divided between the caller and the callee.
##### *source: https://en.wikipedia.org/wiki/Calling_convention*

After executing my code, you will see that every call to MessageBoxA will be redirected to my own function, which will output "Call to MessageBox redirected, parameters passed: " + the parameters, as you already have seen in the first image on the top of this readme

### Detection vectors
Since this is a method oftenly used by malwares and cheats, i will be also talking about detection vectors. As aforementioned, this method involves modifying memory page protection and writing in a section which is supposed to not change. Here we have 2 major vectors: the call to VirtualProtect, which can be monitored for modifications of .text memory space and bytepatching of code in functions. Bytepatch can be detected by integrity checks, and if what is in memory differs from what is in disk then a red flag is raised. Some things you could do to circumvent the detection is:
* Modify the memory protection from a external application using VirtualProtectEx (in windows), so hooks for VirtualProtect inside the target application won't catch it
* Hook the integrity check routines and manipulate the results
* Make a jump hook in mid-function so a simple check for initial function bytes won't find a detour, making it slight more difficult to detect
* If a relative short jump is present inside the function code, you can simply overwrite the offset instead of creating a new jump instruction

After all, this is a simple method that has difficult to hide vectors, if you're dealing with antivirus/anticheats then you should know its inner workings
