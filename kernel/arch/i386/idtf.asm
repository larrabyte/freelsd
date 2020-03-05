global idtflush

idtflush:
    mov eax, [esp+4]
    lidt [eax]
    ret