; used files: tb2/header.h tb2/main.c 

Jmp-Dword-Clasic main
_s1: Assume-Byte 80,97,117,108,0
_s2: Assume-Byte 73,110,103,0
s1_ptr: Assume-Dword 0
s2_ptr: Assume-Dword 0
main:
  Lea-Dword s1_ptr
  Out-Dword _s1
  Lea-Dword s2_ptr
  Out-Dword _s2


