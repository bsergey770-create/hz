
; === DllMain PROCESS_ATTACH handler RVA=0x4edc50 VA=0x1804edc50 ===
  0x0001804edc50  4053                 push rbx
  0x0001804edc52  4883ec30             sub rsp, 0x30
  0x0001804edc56  488b05a3b84200       mov rax, qword ptr [rip + 0x42b8a3]
  0x0001804edc5d  48bb32a2df2d992b0000 movabs rbx, 0x2b992ddfa232
  0x0001804edc67  483bc3               cmp rax, rbx
  0x0001804edc6a  7410                 je 0x1804edc7c
  0x0001804edc6c  48f7d0               not rax
  0x0001804edc6f  488905cab84200       mov qword ptr [rip + 0x42b8ca], rax
  0x0001804edc76  4883c430             add rsp, 0x30
  0x0001804edc7a  5b                   pop rbx
  0x0001804edc7b  c3                   ret 
  0x0001804edc7c  488d4c2440           lea rcx, [rsp + 0x40]
  0x0001804edc81  48c744244000000000   mov qword ptr [rsp + 0x40], 0
  0x0001804edc8a  ff1510d60700         call qword ptr [rip + 0x7d610]
  0x0001804edc90  488b442440           mov rax, qword ptr [rsp + 0x40]
  0x0001804edc95  4889442420           mov qword ptr [rsp + 0x20], rax
  0x0001804edc9a  ff15c8d40700         call qword ptr [rip + 0x7d4c8]
  0x0001804edca0  8bc0                 mov eax, eax
  0x0001804edca2  4831442420           xor qword ptr [rsp + 0x20], rax
  0x0001804edca7  ff1513d70700         call qword ptr [rip + 0x7d713]
  0x0001804edcad  8bc0                 mov eax, eax
  0x0001804edcaf  488d4c2448           lea rcx, [rsp + 0x48]
  0x0001804edcb4  4831442420           xor qword ptr [rsp + 0x20], rax
  0x0001804edcb9  ff1501d50700         call qword ptr [rip + 0x7d501]
  0x0001804edcbf  488b442448           mov rax, qword ptr [rsp + 0x48]
  0x0001804edcc4  8bc8                 mov ecx, eax
  0x0001804edcc6  48c1e120             shl rcx, 0x20
  0x0001804edcca  4833c8               xor rcx, rax
  0x0001804edccd  488d442420           lea rax, [rsp + 0x20]
  0x0001804edcd2  48334c2420           xor rcx, qword ptr [rsp + 0x20]
  0x0001804edcd7  4833c8               xor rcx, rax
  0x0001804edcda  48b8ffffffffffff0000 movabs rax, 0xffffffffffff
  0x0001804edce4  4823c8               and rcx, rax
  0x0001804edce7  48b833a2df2d992b0000 movabs rax, 0x2b992ddfa233
  0x0001804edcf1  483bcb               cmp rcx, rbx
  0x0001804edcf4  480f44c8             cmove rcx, rax
  0x0001804edcf8  48890d01b84200       mov qword ptr [rip + 0x42b801], rcx
  0x0001804edcff  48f7d1               not rcx
  0x0001804edd02  48890d37b84200       mov qword ptr [rip + 0x42b837], rcx
  0x0001804edd09  4883c430             add rsp, 0x30
  0x0001804edd0d  5b                   pop rbx
  0x0001804edd0e  c3                   ret 
  0x0001804edd0f  cc                   int3 
  0x0001804edd10  488d0d59177b00       lea rcx, [rip + 0x7b1759]
  0x0001804edd17  48ff2562d70700       jmp qword ptr [rip + 0x7d762]
  0x0001804edd1e  cc                   int3 
  0x0001804edd1f  cc                   int3 
  0x0001804edd20  488d0d49177b00       lea rcx, [rip + 0x7b1749]
  0x0001804edd27  e9541a0000           jmp 0x1804ef780
  0x0001804edd2c  cc                   int3 
  0x0001804edd2d  cc                   int3 
  0x0001804edd2e  cc                   int3 
  0x0001804edd2f  cc                   int3 
  0x0001804edd30  4883ec28             sub rsp, 0x28
  0x0001804edd34  e80759b2ff           call 0x180013640
  0x0001804edd39  48830824             or qword ptr [rax], 0x24
  0x0001804edd3d  e80e59b2ff           call 0x180013650
  0x0001804edd42  48830802             or qword ptr [rax], 2
  0x0001804edd46  4883c428             add rsp, 0x28
  0x0001804edd4a  c3                   ret 
  0x0001804edd4b  cc                   int3 
  0x0001804edd4c  cc                   int3 
  0x0001804edd4d  cc                   int3 
  0x0001804edd4e  cc                   int3 
  0x0001804edd4f  cc                   int3 
  0x0001804edd50  488d056987c200       lea rax, [rip + 0xc28769]
  0x0001804edd57  c3                   ret 
  0x0001804edd58  cc                   int3 
  0x0001804edd59  cc                   int3 
  0x0001804edd5a  cc                   int3 
  0x0001804edd5b  cc                   int3 
  0x0001804edd5c  cc                   int3 
  0x0001804edd5d  cc                   int3 
  0x0001804edd5e  cc                   int3 
  0x0001804edd5f  cc                   int3 
  0x0001804edd60  48895c2408           mov qword ptr [rsp + 8], rbx
  0x0001804edd65  57                   push rdi
  0x0001804edd66  4883ec20             sub rsp, 0x20

; === CRT dllEntryPoint (dispatch) RVA=0x4ed740 VA=0x1804ed740 ===
  0x0001804ed740  48895c2420           mov qword ptr [rsp + 0x20], rbx
  0x0001804ed745  4c89442418           mov qword ptr [rsp + 0x18], r8
  0x0001804ed74a  89542410             mov dword ptr [rsp + 0x10], edx
  0x0001804ed74e  48894c2408           mov qword ptr [rsp + 8], rcx
  0x0001804ed753  56                   push rsi
  0x0001804ed754  57                   push rdi
  0x0001804ed755  4156                 push r14
  0x0001804ed757  4883ec40             sub rsp, 0x40
  0x0001804ed75b  498bf0               mov rsi, r8
  0x0001804ed75e  8bfa                 mov edi, edx
  0x0001804ed760  4c8bf1               mov r14, rcx
  0x0001804ed763  85d2                 test edx, edx
  0x0001804ed765  7518                 jne 0x1804ed77f
  0x0001804ed767  3915f31c7b00         cmp dword ptr [rip + 0x7b1cf3], edx
  0x0001804ed76d  7f10                 jg 0x1804ed77f
  0x0001804ed76f  33c0                 xor eax, eax
  0x0001804ed771  488b5c2478           mov rbx, qword ptr [rsp + 0x78]
  0x0001804ed776  4883c440             add rsp, 0x40
  0x0001804ed77a  415e                 pop r14
  0x0001804ed77c  5f                   pop rdi
  0x0001804ed77d  5e                   pop rsi
  0x0001804ed77e  c3                   ret 
  0x0001804ed77f  8d42ff               lea eax, [rdx - 1]
  0x0001804ed782  83f801               cmp eax, 1
  0x0001804ed785  7742                 ja 0x1804ed7c9
  0x0001804ed787  488b0502830800       mov rax, qword ptr [rip + 0x88302]
  0x0001804ed78e  4885c0               test rax, rax
  0x0001804ed791  7507                 jne 0x1804ed79a
  0x0001804ed793  bb01000000           mov ebx, 1
  0x0001804ed798  eb08                 jmp 0x1804ed7a2
  0x0001804ed79a  ff15b0e00700         call qword ptr [rip + 0x7e0b0]
  0x0001804ed7a0  8bd8                 mov ebx, eax
  0x0001804ed7a2  895c2430             mov dword ptr [rsp + 0x30], ebx
  0x0001804ed7a6  85db                 test ebx, ebx
  0x0001804ed7a8  0f84b0000000         je 0x1804ed85e
  0x0001804ed7ae  4c8bc6               mov r8, rsi
  0x0001804ed7b1  8bd7                 mov edx, edi
  0x0001804ed7b3  498bce               mov rcx, r14
  0x0001804ed7b6  e875fdffff           call 0x1804ed530
  0x0001804ed7bb  8bd8                 mov ebx, eax
  0x0001804ed7bd  89442430             mov dword ptr [rsp + 0x30], eax
  0x0001804ed7c1  85c0                 test eax, eax
  0x0001804ed7c3  0f8495000000         je 0x1804ed85e
  0x0001804ed7c9  4c8bc6               mov r8, rsi
  0x0001804ed7cc  8bd7                 mov edx, edi
