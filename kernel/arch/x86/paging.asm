global loadpdbr
global getpdbr
global flushtlb

getpdbr:
    mov eax, cr3
    ret

loadpdbr:
    mov eax, [esp+4]
    mov cr3, eax
    ret

flushtlb:
    cli
    mov eax, [esp+4]
    invlpg [eax]
    sti
    ret
