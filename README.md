# detour hooking

<details><summary><h2>🌐 EN-US</h2></summary>

### What is hooking?
Hooking is the concept of redirecting existing code flow. This makes possible to modify, block or change the behavior of a function or code at given address for many purposes, including defensive/offensive reasons like monitoring calls to a function, blocking dangerous actions, modifying/reading parameters passed or hiding information/code by faking results (or changing return values) to bypass antivirus checks for example

![output](https://github.com/LeonardoSasaki/detour-hooking/blob/main/img/output.png?raw=true)

### What is detour hooking?
Detour hooking is just one among plenty other hooking methods, like IAT, VMT, VEH, etc. Detours usually works by inserting a jmp instruction at a given address to redirect code execution. Even though a call instruction instead of a jmp is possible, it may not always fit the purpose due to the code returning to the original function, also it will overwrite your target function prologue if you choose to overwrite the beggining of it, which is another problem you would need to take care of. A jmp is the most ideal and the most used.

In this code, we hooked two functions: MessageBoxA from WinAPI (if compiled on windows, obviously) and a function i wrote myself (in case you compile for linux). Take a look at MessageBoxA disassembly on memory before hooking:
![original](https://github.com/LeonardoSasaki/detour-hooking/blob/main/img/orig.png?raw=true)

And after hooking:
![hooked](https://github.com/LeonardoSasaki/detour-hooking/blob/main/img/hooked.png?raw=true)
As you can see, the beggining of the code changed to a jump to our function. Here is what i did: first, i got a pointer to MessageBoxA function address querying it using GetProcAddress, passing the first parameter as the handle to the module where this function is located (user32.dll) and the second parameter was the name of the function itself. There are other methods you can use to get the target address, for example by pattern scanning. It allows for finding not just a function, but a specific piece of code anywhere in the process. After i got the target function address, i changed page protection to be able to write to it (generally functions are compiled to .text segment for default, which doesnt has write permission) and wrote this piece of code: 
```
E9 xx xx xx xx
```
E9 is the hexadecimal value for relative jump near instruction, followed by "xx xx xx xx", that is the 4 byte long offset to jump (which we will write soon). Being E9 (1 byte length) + the offset (4 bytes length) = 5 bytes long, here is how you calculate the offset: ``` destination - target function - 5 ```
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
You might have noticed that there are 2 differences: __stdcall and the function name. In fact, function name does not change anything at all. After compiled, every function is just code in memory, they don't exactly have names (except for exported functions, of course, but still they do not have their name on its body). You will get a compile error if you use the same name, since you're re-defining a existing function. And about the __stdcall, its just the calling convention. Functions from WinAPI uses stdcall as default,  you could even change it to "WINAPI" macro, with extends to __stdcall. I myself had the project settings to compile functions with __cdecl calling convention by default, but you  might be asking: "what is a calling convention?" From wikipedia,
> A calling convention is an implementation-level (low-level) scheme for how subroutines receive parameters from their caller and how they return a result. Differences in various implementations include where parameters, return values, return addresses and scope links are placed (registers, stack or memory etc.), and how the tasks of preparing for a function call and restoring the environment afterwards are divided between the caller and the callee.
##### *source: https://en.wikipedia.org/wiki/Calling_convention*

After executing my code, you will see that every call to MessageBoxA will be redirected to my own function, which will output "Call to MessageBox redirected, parameters passed: " + the parameters, as you already have seen in the first image on the top of this readme

### Detection vectors
Since this is a method oftenly used by malwares and cheats, i will be also talking about detection vectors. As aforementioned, this method involves modifying memory page protection and writing in a section which is supposed to not change. Here we have 2 major vectors: the call to VirtualProtect, which can be monitored for modifications of .text memory space and the bytepatching of code in functions. Bytepatching can be detected by integrity checks, and if what is in memory differs from what is in disk then a red flag is raised. Some things you could do to circumvent the detection is:
* Modify the memory protection from a external application using VirtualProtectEx (in windows) or using other methods, that way, hooks to VirtualProtect inside the target application won't catch it
* Hook the integrity check routines and manipulate the results
* Make a jump hook in mid-function so a simple check for initial function bytes won't find a detour, making it slight more difficult to detect
* If a relative short jump is present inside the function code, you can simply overwrite the offset instead of creating a new jump instruction

After all, this is a simple method that has difficult to hide vectors, if you're dealing with antivirus/anticheats then you should know its inner workings
</summary></details>

<details><summary><h2>🌐 PT-BR</h2></summary>

### O que é hooking?
Hooking é o conceito de redirecionar o fluxo código existente. Isso permite modificar, bloquear ou alterar o comportamento de uma função ou código em um dado endereço para vários propósitos, incluindo razões ofensivas/defensivas como monitorar chamadas para uma função, bloquear ações perigosas, ler/modificar parâmetros passados ou esconder informações/código falsificando o resultado (por exemplo mudando o valor de retorno da função), para bypassar verificações de antivírus por exemplo.

![output](https://github.com/LeonardoSasaki/detour-hooking/blob/main/img/output.png?raw=true)

### O que detour hooking?
Hooking de detour é apenas um entre vários outros métodos, como IAT, VMT, VEH, etc. Detours geralmente funcionam inserindo uma instrução jmp em um dado endereço de memória para redirecionar o fluxo do código. Apesar de uma instrução "call" poder ser usada ao invés de um jmp, ele nem sempre satisfaz o propósito devido ao fato do código retornar para a função original após retornar, além que ele sobrescreverá o prologue da função se você escolher sobrescrever os bytes iniciais da função, que é outro problema que você teria que resolver. Um jmp é o mais ideal e o mais utilizado.

Neste código, nós hookamos duas funções: MessageBoxA da WinAPI (se compilado em Windows, óbviamente) e uma função que eu mesmo escrevi (caso você compile para Linux). Dê uma olhada no disassembly do MessageBoxA antes do hook:
![original](https://github.com/LeonardoSasaki/detour-hooking/blob/main/img/orig.png?raw=true)

E após o hook:
![hooked](https://github.com/LeonardoSasaki/detour-hooking/blob/main/img/hooked.png?raw=true)
Como você pode ver, o início do código alterou para um jmp para nossa função. Aqui está o que eu fiz: primeiro, consegui um ponteiro para o MessageBoxA com a função GetProcAddress, passando o primeiro parâmetro como uma handle para o módulo onde esta função está localizada (user32.dll), enquanto o segundo parâmetro era o própio nome da função. Há outros métodos que você pode usar para obter o endereço alvo, como o pattern scanning por exemplo. Este método permite obter o endereço de qualquer pedaço específico de código, como funções e trechos de código. Depois que eu obtive o endereço da função, eu mudei a proteção da página de memória para poder escrever nela (geralmente, as funções são compiladas na seção .text, que não possui permissão de escritura por padrão) e então escrevi este pedaço de código:
```
E9 xx xx xx xx
```
E9, que é o valor hexadecimal para a instrução relative jmp near (a tradução fica algo como "pulo curto relativo"), seguido de "xx xx xx xx", o qual será escrito a distância de 4 bytes para se pular na memória. Sendo E9 (de tamanho de um byte) + a distância (4 bytes) = 5 bytes, aqui está como é calculado a distância (ou offset, se preferir chamar assim): ``` destino - endereço para hookar - 5 ```
Fazendo isto, você estará sobrescrevendo 5 bytes na memória com um pulo para o seu código. Certifique-se de que o protótipo de sua função é a mesma função a qual você está hookando (mesmo tipo de retorno, convenção de chamada e parâmetros), se não vários erros podem ocorrer. No meu código, eu repliquei o MessageBoxA:
```
int __stdcall hooked_function(
        HWND    hWnd,
        LPCSTR  lpText,
        LPCSTR  lpCaption,
        UINT    uType)
```
Na documentação oficial da microsoft, temos que:
```
int MessageBox(
  HWND    hWnd,
  LPCTSTR lpText,
  LPCTSTR lpCaption,
  UINT    uType)
```
##### *fonte: https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-messagebox*
Você pode ter reparado que há 2 diferenças: o __stdcall e o nome da função. De fato, o nome da função não altera nada. Após compilado, todas funções são apenas códigos na memória, eles não possuem exatamente um "nome" no corpo delas (exceto para as funções exportadas, claro, mas nem mesmo elas possuem um "nome" no corpo delas). Porém, o código falhará na compilação caso você redefina a função utilizando o mesmo nome dela. Na minha função, eu defini a convenção de chamada como __stdcall pois as funções da WinAPI a utilizam como padrão, e em minhas configurações do VS2019 estava setado para a convenção de chamada padrão ser __cdecl, portanto foi necessário especificar (eu poderia até utilizar a macro WINAPI, que expande para __stdcall). Mas agora, você esteve estar se perguntando: "o que é uma convenção de chamada?" Da wikipedia,
> Em ciência da computação, convenção de chamadas de função é um esquema o qual as funções de um programa recebem parâmetros das funções chamadoras e como elas retornam um resultado. Essas convenções diferem de acordo com as linguagens de programação, os sistemas operacionais e CPUs.
##### *fonte: https://pt.wikipedia.org/wiki/Conven%C3%A7%C3%A3o_de_chamada_de_fun%C3%A7%C3%B5es*

Após executar meu código, você verá que todas as chamadas para MessageBoxA vão ser redirecionadas para minha própia função. a qual printará "Call to MessageBox redirected, parameters passed: " + os parâmetros, como você já viu na primeira imagem no topo deste readme

### Vetores de detecção
Já que este é um método muito usado por malwares e cheats, eu também vou falar sobre vetores de detecção. Como mencionado antes, este método envolve modificar a proteção de páginas de memória e escrever em um segmento de código constante. Aqui temos 2 grandes vetores: a chamada para o VirtualProtect, o qual pode ser monitorado contra a modificação da proteção do espaço de memória pertencente ao segmento .text e o bytepatching de código em funções. Bytepatching pode ser detectado por verificações de integridade, por exemplo caso o que está em memória difere com o que está em disco, então uma bandeira vermelha é acionada. Algumas coisas que você pode fazer para evitar a detecção são:

* Modificar a proteção da memória externamente usando VirtualProtectEx (no windows) ou utilizando outros métodos, desta forma, hooks para o VirtualProtect dentro da aplicação alvo não captarão esta alteração
* Hookar as verificações de integridade e manipular os resultados
* Inserir um jmp no meio da função ao invés do início dela, assim uma simples verificação pelos bytes iniciais de uma função não encontrarão um detour, tornando um pouco mais complexo a detecção
* Se um relative short jmp já estiver presente dentro da função alvo, você pode simplesmente sobrescrever este offset ao invés de criar um novo jmp

Apesar de tudo, este é um método simples que possui vetores difíceis de ocultar. Se você está lidando com um antivírus ou anticheat, então é esperado que você conheça o funcionamento do tal dito software
</summary></details>
