; Entry point (DllMain candidate) RVA=0x4ed880 VA=0x1804ed880
  0x00000001804ed880  48895c2408           mov qword ptr [rsp + 8], rbx
  0x00000001804ed885  4889742410           mov qword ptr [rsp + 0x10], rsi
  0x00000001804ed88a  57                   push rdi
  0x00000001804ed88b  4883ec20             sub rsp, 0x20
  0x00000001804ed88f  498bf8               mov rdi, r8
  0x00000001804ed892  8bda                 mov ebx, edx
  0x00000001804ed894  488bf1               mov rsi, rcx
  0x00000001804ed897  83fa01               cmp edx, 1
  0x00000001804ed89a  7505                 jne 0x1804ed8a1
  0x00000001804ed89c  e8af030000           call 0x1804edc50
  0x00000001804ed8a1  4c8bc7               mov r8, rdi
  0x00000001804ed8a4  8bd3                 mov edx, ebx
  0x00000001804ed8a6  488bce               mov rcx, rsi
  0x00000001804ed8a9  488b5c2430           mov rbx, qword ptr [rsp + 0x30]
  0x00000001804ed8ae  488b742438           mov rsi, qword ptr [rsp + 0x38]
  0x00000001804ed8b3  4883c420             add rsp, 0x20
  0x00000001804ed8b7  5f                   pop rdi
  0x00000001804ed8b8  e983feffff           jmp 0x1804ed740
  0x00000001804ed8bd  cc                   int3 
  0x00000001804ed8be  cc                   int3 
  0x00000001804ed8bf  cc                   int3 
  0x00000001804ed8c0  4883ec28             sub rsp, 0x28
  0x00000001804ed8c4  4d8b4138             mov r8, qword ptr [r9 + 0x38]
  0x00000001804ed8c8  488bca               mov rcx, rdx
  0x00000001804ed8cb  498bd1               mov rdx, r9
  0x00000001804ed8ce  e80d000000           call 0x1804ed8e0
  0x00000001804ed8d3  b801000000           mov eax, 1
  0x00000001804ed8d8  4883c428             add rsp, 0x28
  0x00000001804ed8dc  c3                   ret 
  0x00000001804ed8dd  cc                   int3 
  0x00000001804ed8de  cc                   int3 
  0x00000001804ed8df  cc                   int3 
  0x00000001804ed8e0  48895c2410           mov qword ptr [rsp + 0x10], rbx
  0x00000001804ed8e5  48896c2418           mov qword ptr [rsp + 0x18], rbp
  0x00000001804ed8ea  56                   push rsi
  0x00000001804ed8eb  4883ec20             sub rsp, 0x20
  0x00000001804ed8ef  496328               movsxd rbp, dword ptr [r8]
  0x00000001804ed8f2  488bf1               mov rsi, rcx
  0x00000001804ed8f5  40f6c504             test bpl, 4
  0x00000001804ed8f9  7415                 je 0x1804ed910
  0x00000001804ed8fb  418b4008             mov eax, dword ptr [r8 + 8]
  0x00000001804ed8ff  49635804             movsxd rbx, dword ptr [r8 + 4]
  0x00000001804ed903  f7d8                 neg eax
  0x00000001804ed905  4c63c0               movsxd r8, eax
  0x00000001804ed908  4803d9               add rbx, rcx
  0x00000001804ed90b  4923d8               and rbx, r8
  0x00000001804ed90e  eb03                 jmp 0x1804ed913
  0x00000001804ed910  488bde               mov rbx, rsi
  0x00000001804ed913  488b4210             mov rax, qword ptr [rdx + 0x10]
  0x00000001804ed917  488b5208             mov rdx, qword ptr [rdx + 8]
  0x00000001804ed91b  48897c2430           mov qword ptr [rsp + 0x30], rdi
  0x00000001804ed920  8b4808               mov ecx, dword ptr [rax + 8]
  0x00000001804ed923  4803d1               add rdx, rcx
  0x00000001804ed926  440fb602             movzx r8d, byte ptr [rdx]
  0x00000001804ed92a  0fb64a03             movzx ecx, byte ptr [rdx + 3]
  0x00000001804ed92e  410fb6c0             movzx eax, r8b
  0x00000001804ed932  2407                 and al, 7
  0x00000001804ed934  3c03                 cmp al, 3
  0x00000001804ed936  730d                 jae 0x1804ed945
  0x00000001804ed938  f6c10f               test cl, 0xf
  0x00000001804ed93b  7412                 je 0x1804ed94f
  0x00000001804ed93d  448bc9               mov r9d, ecx
  0x00000001804ed940  e90c010000           jmp 0x1804eda51
  0x00000001804ed945  8bf9                 mov edi, ecx
  0x00000001804ed947  448bd9               mov r11d, ecx
  0x00000001804ed94a  83e71f               and edi, 0x1f
  0x00000001804ed94d  7508                 jne 0x1804ed957
  0x00000001804ed94f  4533c9               xor r9d, r9d
  0x00000001804ed952  e9fe000000           jmp 0x1804eda55
  0x00000001804ed957  4883c204             add rdx, 4
  0x00000001804ed95b  8bc7                 mov eax, edi
  0x00000001804ed95d  41f6c040             test r8b, 0x40
  0x00000001804ed961  7409                 je 0x1804ed96c
  0x00000001804ed963  488d147a             lea rdx, [rdx + rdi*2]
  0x00000001804ed967  48ffc2               inc rdx
  0x00000001804ed96a  eb03                 jmp 0x1804ed96f
  0x00000001804ed96c  4803d0               add rdx, rax
  0x00000001804ed96f  4533c9               xor r9d, r9d
  0x00000001804ed972  41c1eb05             shr r11d, 5
  0x00000001804ed976  418bc9               mov ecx, r9d
  0x00000001804ed979  4585db               test r11d, r11d
  0x00000001804ed97c  7435                 je 0x1804ed9b3
  0x00000001804ed97e  6690                 nop 
  0x00000001804ed980  440fb612             movzx r10d, byte ptr [rdx]
  0x00000001804ed984  4883c203             add rdx, 3
  0x00000001804ed988  418bc2               mov eax, r10d
  0x00000001804ed98b  c1e803               shr eax, 3
  0x00000001804ed98e  85c0                 test eax, eax
  0x00000001804ed990  741a                 je 0x1804ed9ac
  0x00000001804ed992  448bc0               mov r8d, eax
  0x00000001804ed995  41f6c202             test r10b, 2
  0x00000001804ed999  740a                 je 0x1804ed9a5
  0x00000001804ed99b  488d1442             lea rdx, [rdx + rax*2]
  0x00000001804ed99f  4883c204             add rdx, 4
  0x00000001804ed9a3  eb07                 jmp 0x1804ed9ac
  0x00000001804ed9a5  4883c203             add rdx, 3
  0x00000001804ed9a9  4903d0               add rdx, r8
  0x00000001804ed9ac  ffc1                 inc ecx
  0x00000001804ed9ae  413bcb               cmp ecx, r11d
  0x00000001804ed9b1  72cd                 jb 0x1804ed980
  0x00000001804ed9b3  458bd1               mov r10d, r9d
  0x00000001804ed9b6  85ff                 test edi, edi
  0x00000001804ed9b8  0f8497000000         je 0x1804eda55
  0x00000001804ed9be  4c8d1d3b26b1ff       lea r11, [rip - 0x4ed9c5]
  0x00000001804ed9c5  0fb60a               movzx ecx, byte ptr [rdx]
  0x00000001804ed9c8  80f903               cmp cl, 3
  0x00000001804ed9cb  7632                 jbe 0x1804ed9ff
  0x00000001804ed9cd  440fb6c1             movzx r8d, cl
  0x00000001804ed9d1  4180e00f             and r8b, 0xf
  0x00000001804ed9d5  418d40f8             lea eax, [r8 - 8]
  0x00000001804ed9d9  3c02                 cmp al, 2
  0x00000001804ed9db  761e                 jbe 0x1804ed9fb
  0x00000001804ed9dd  440fb6c1             movzx r8d, cl
  0x00000001804ed9e1  4180e007             and r8b, 7
  0x00000001804ed9e5  418d40fc             lea eax, [r8 - 4]
  0x00000001804ed9e9  3c03                 cmp al, 3
  0x00000001804ed9eb  760e                 jbe 0x1804ed9fb
  0x00000001804ed9ed  80e13f               and cl, 0x3f
  0x00000001804ed9f0  80f920               cmp cl, 0x20
  0x00000001804ed9f3  0f8584000000         jne 0x1804eda7d
  0x00000001804ed9f9  eb0d                 jmp 0x1804eda08
  0x00000001804ed9fb  410fb6c8             movzx ecx, r8b
  0x00000001804ed9ff  84c9                 test cl, cl
  0x00000001804eda01  7449                 je 0x1804eda4c
  0x00000001804eda03  80f920               cmp cl, 0x20
  0x00000001804eda06  7775                 ja 0x1804eda7d
  0x00000001804eda08  0fb6c1               movzx eax, cl
  0x00000001804eda0b  410fb6840398da4e00   movzx eax, byte ptr [r11 + rax + 0x4eda98]
  0x00000001804eda14  418b8c8384da4e00     mov ecx, dword ptr [r11 + rax*4 + 0x4eda84]
  0x00000001804eda1c  4903cb               add rcx, r11
  0x00000001804eda1f  ffe1                 jmp rcx
  0x00000001804eda21  b801000000           mov eax, 1
  0x00000001804eda26  eb13                 jmp 0x1804eda3b
  0x00000001804eda28  b802000000           mov eax, 2
  0x00000001804eda2d  eb0c                 jmp 0x1804eda3b
  0x00000001804eda2f  b803000000           mov eax, 3
  0x00000001804eda34  eb05                 jmp 0x1804eda3b
  0x00000001804eda36  b805000000           mov eax, 5
  0x00000001804eda3b  4803d0               add rdx, rax
  0x00000001804eda3e  41ffc2               inc r10d
  0x00000001804eda41  443bd7               cmp r10d, edi
  0x00000001804eda44  0f827bffffff         jb 0x1804ed9c5
  0x00000001804eda4a  eb09                 jmp 0x1804eda55
  0x00000001804eda4c  440fb64a01           movzx r9d, byte ptr [rdx + 1]
  0x00000001804eda51  4983e1f0             and r9, 0xfffffffffffffff0
  0x00000001804eda55  498d0c31             lea rcx, [r9 + rsi]
  0x00000001804eda59  488bc5               mov rax, rbp
  0x00000001804eda5c  4883e0f8             and rax, 0xfffffffffffffff8
  0x00000001804eda60  48330c18             xor rcx, qword ptr [rax + rbx]
  0x00000001804eda64  488b7c2430           mov rdi, qword ptr [rsp + 0x30]
  0x00000001804eda69  488b5c2438           mov rbx, qword ptr [rsp + 0x38]
  0x00000001804eda6e  488b6c2440           mov rbp, qword ptr [rsp + 0x40]
  0x00000001804eda73  4883c420             add rsp, 0x20
  0x00000001804eda77  5e                   pop rsi
  0x00000001804eda78  e903f0ffff           jmp 0x1804eca80
  0x00000001804eda7d  e85ef6ffff           call 0x1804ed0e0
  0x00000001804eda82  cc                   int3 
  0x00000001804eda83  90                   nop 
  0x00000001804eda84  28da                 sub dl, bl
  0x00000001804eda86  4e0036               add byte ptr [rsi], r14b
  0x00000001804eda89  da4e00               fimul dword ptr [rsi]
