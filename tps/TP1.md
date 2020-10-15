TP1: Memoria virtual en JOS

boot_alloc_pos
--------------

a. Cálculo manual de la primera dirección de memoria que devuelve `boot_alloc()`.

```console

$ readelf -a kernel | grep end
110: f0117950     0 NOTYPE  GLOBAL DEFAULT    6 end
```

o

```console

$ nm kernel | grep end
f0117950 B end
```

End es: f0117950

En decimal: 4027677008

El tamaño de cada pagina es: 4096

4096 - (4027677008 % 4096) = 1712

Redondeo para arriba:

4027677008 + 1712 = 4027678720

La dirección es f0118000.

b. 

```
gdb -q -s obj/kern/kernel -ex 'target remote 127.0.0.1:26000' -n -x .gdbinit
Reading symbols from obj/kern/kernel...done.
Remote debugging using 127.0.0.1:26000
warning: No executable has been specified and target does not support
determining executable automatically.  Try using the "file" command.
0x0000fff0 in ?? ()
(gdb) b boot_alloc
Breakpoint 1 at 0xf0100b95: file kern/pmap.c, line 89.
(gdb) continue
Continuing.
The target architecture is assumed to be i386
=> 0xf0100b95 <boot_alloc>:	push   %ebp

Breakpoint 1, boot_alloc (n=65684) at kern/pmap.c:89
89	{
(gdb) l
84	// If we're out of memory, boot_alloc should panic.
85	// This function may ONLY be used during initialization,
86	// before the page_free_list list has been set up.
87	static void *
88	boot_alloc(uint32_t n)
89	{
90		static char *nextfree;  // virtual address of next byte of free memory
91		char *result;
92	
93		// Initialize nextfree if this is the first time.
(gdb) p nextfree
$1 = 0x0
(gdb) p &end
$2 = (<data variable, no debug info> *) 0xf0117950
(gdb) fin
Run till exit from #0  boot_alloc (n=65684) at kern/pmap.c:89
Could not fetch register "orig_eax"; remote failure reply 'E14'
(gdb) p nextfree
$3 = 0xf0119000 ""
(gdb) p &end
$4 = (<data variable, no debug info> *) 0xf0117950
```

page_alloc
----------

* ¿En qué se diferencia `page2pa()` de `page2kva()`?

`page2pa()` recibe un puntero de struct PageInfo y devuelve la dirección física de la página.

`page2kva()` recibe un puntero de struct PageInfo y devuelve la dirección virtual.

map_region_large
----------

* ¿Cuánta memoria se ahorró de este modo? ¿Es una cantidad fija, o depende de la memoria física de la computadora?

Cuando se implementa large pages en un principio se ahorra la page table incial. Por cada bloque de large pages
ocupado se ahorra una página, 4096 bytes. Si no es necesario el uso de large pages luego del inicio entonces solo se ahorra esa página.
Si todos los bloques de memoria que se piden están alineados a 4 MiB entonces por cada large page necesitamos una entrada en el page directory. Tenemos disponibles un page directory de 1024 entradas que sin large pages cada una tendría 1024 page tables de 1024 entradas
cada una. Se ahorran 1024 bytes * 1024 bytes = 1 MiB.

