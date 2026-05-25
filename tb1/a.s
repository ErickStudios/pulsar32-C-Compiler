Jmp-Dword-Clasic _start
x: Assume-Dword 0
y: Assume-Dword 0
two: Assume-Dword 0
_preload:
Lea-Dword x
Out-Dword 0
Lea-Dword y
Out-Dword 21
Lea-Dword two
Out-Dword 2
Jmp-Dword-Clasic Sp
_start:
Jmp-Dword-Call _preload
Jmp-Dword-Clasic main
main:
Jmp-Dword-Call abc
Jmp-Dword-Clasic Sp

abc:
Lea-Dword two
Mov-Dword
Push-Dword Out
Lea-Dword y
Mov-Dword
Push-Dword Out
Mul-Dword-Sp-Sp
Lea-Dword x
Out-Dword Out
Jmp-Dword-Clasic Sp

