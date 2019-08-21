ml /c /coff /Cp "main.asm"
link /MAP /debug /debugtype:cv /nologo /driver:WDM /base:0x10000 /align:64 /out:"main.sys" /subsystem:native "main.obj"