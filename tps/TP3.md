env_return
==========

- al terminar un proceso su función umain() ¿dónde retoma la ejecución el kernel? Describir la secuencia de llamadas desde que termina umain() hasta que el kernel dispone del proceso.

La librería estandar llama a la función `exit()`, que a su vez llama a la syscall `sys_env_destroy()`, que llama a `env_destroy()`, que libera la memoria asociada a ese environment y llama al scheduler con `sched_yield()`.

- ¿en qué cambia la función env_destroy() en este TP, respecto al TP anterior?
Se agrega esa llamada a `sched_yield()`.

sys_yield
=========

```
Booting from Hard Disk..6828 decimal is 15254 octal!
Physical memory: 131072K available, base = 640K, extended = 130432K
boot_alloc: initialized in f027e000
boot_alloc: allocating 4096 bytes (1 pages), starting in f027e000
boot_alloc: allocation ends in f027f000
mem_init: kern_pgdir allocated in f027e000
boot_alloc: allocating 262144 bytes (64 pages), starting in f027f000
boot_alloc: allocation ends in f02bf000
mem_init: pages allocated in f027f000
boot_alloc: allocating 126976 bytes (31 pages), starting in f02bf000
boot_alloc: allocation ends in f02de000
boot_alloc: allocating 0 bytes (0 pages), starting in f02de000
boot_alloc: allocating 0 bytes (0 pages), starting in f02de000
check_page_free_list() succeeded!
check_page_alloc() succeeded!
check_page() succeeded!
mem_init: mapping pages (va[ef000000, ef040000] -> pa[0027f000, 002bf000])
mem_init: mapping pages (va[eec00000, eec1f000] -> pa[002bf000, 002de000])
mem_init: mapping kernel stack (va[efff8000, f0000000] -> pa[00118000, 00120000])
mem_init: mapping physical memory (va[f0000000, 00000000] -> pa[00000000, 10000000])
check_kern_pgdir() succeeded!
boot_alloc: allocating 0 bytes (0 pages), starting in f02de000
check_page_free_list() succeeded!
check_page_installed_pgdir() succeeded!
SMP: CPU 0 found 1 CPU(s)
enabled interrupts: 1 2
[00000000] new env 00001000
[00000000] new env 00001001
[00000000] new env 00001002
Found env 00001000 to run
Hello, I am environment 00001000.
Found env 00001001 to run
Hello, I am environment 00001001.
Found env 00001002 to run
Hello, I am environment 00001002.
Found env 00001000 to run
Back in environment 00001000, iteration 0.
Found env 00001001 to run
Back in environment 00001001, iteration 0.
Found env 00001002 to run
Back in environment 00001002, iteration 0.
Found env 00001000 to run
Back in environment 00001000, iteration 1.
Found env 00001001 to run
Back in environment 00001001, iteration 1.
Found env 00001002 to run
Back in environment 00001002, iteration 1.
Found env 00001000 to run
Back in environment 00001000, iteration 2.
Found env 00001001 to run
Back in environment 00001001, iteration 2.
Found env 00001002 to run
Back in environment 00001002, iteration 2.
Found env 00001000 to run
Back in environment 00001000, iteration 3.
Found env 00001001 to run
Back in environment 00001001, iteration 3.
Found env 00001002 to run
Back in environment 00001002, iteration 3.
Found env 00001000 to run
Back in environment 00001000, iteration 4.
All done in environment 00001000.
[00001000] exiting gracefully
[00001000] free env 00001000
Found env 00001001 to run
Back in environment 00001001, iteration 4.
All done in environment 00001001.
[00001001] exiting gracefully
[00001001] free env 00001001
Found env 00001002 to run
Back in environment 00001002, iteration 4.
All done in environment 00001002.
[00001002] exiting gracefully
[00001002] free env 00001002
No runnable environments in the system!
Welcome to the JOS kernel monitor!
Type 'help' for a list of commands.
K>
```

El programa hace cinco iteraciones llamando a la syscall `sys_yield`, por lo que devuelve el control al kernel. Habiendo lanzado tres instancias del programa, el kernel pasa al siguiente cada vez, hasta que van terminando.

Tarea: envid2env
================

En JOS, si un proceso llama a `sys_env_destroy(0)`, se destruye a sí mismo. El `kill(0, 9)` de Linux hace lo mismo.

En cambio, si se llama a `sys_env_destroy(-1)` la llamada a va fallar con -E_BAD_ENV, mientras que el `kill(-1, 9)` de Linux va a enviar esa señal a todos los procesos a los que tenga permiso para hacerlo.

Tarea: dumbfork
================

* Si, antes de llamar a dumbfork(), el proceso se reserva a sí mismo una página con sys_page_alloc() ¿se propagará una copia al proceso hijo? ¿Por qué?

Dumbfork copia todo el adress space y el stack del padre al hijo, si esa página fue reservada en esas dos regiones entonces se propaga la copia al proceso hijo.


* ¿Se preserva el estado de solo-lectura en las páginas copiadas? Mostrar, con código en espacio de usuario, cómo saber si una dirección de memoria es modificable por el proceso, o no. (Ayuda: usar las variables globales uvpd y/o uvpt.)

El duppage que se llama en dumbfork es un bastante simple y para todas las páginas copiadas se pasan los mismos permisos: `PTE_P|PTE_U|PTE_W`, deja de ser de solo lectura con el terce flag que indica permisos de escritura.

```
if ((uvpd[PDX(addr)] & PTE_P) && (uvpt[PGNUM(addr)] & PTE_P)) { // Check if page exists, then if page is present
	if (uvpt[PGNUM(addr)] & PTE_W) // Page is writable
		...
}	

```

* Describir el funcionamiento de la función duppage().

Duppage copia el contenido de una página del proceso padre al hijo. 
	1. Aloca una página en addr en el proceso hijo.
	2. Mapea la dirección addr del proceso hijo al proceso padre en la dirección UTEMP.
	3. Se copia el contenido de la página en addr en UTEMP.
	4. Se desmapea la dirección UTEMP.

* Supongamos que se añade a duppage() un argumento booleano que indica si la página debe quedar como solo-lectura en el proceso hijo:

	* indicar qué llamada adicional se 	debería hacer si el booleano es true

	Con el nuevo argumento la página de solo lectura debe mapearse solo con los permisos `PTE_P|PTE_U` en sys_page_map.

	* describir un algoritmo alternativo que no aumente el número de llamadas al sistema, que debe quedar en 3 (1 × alloc, 1 × map, 1 × unmap).

	Como describimos en clase, una alternativa podría ser: Alocar la página en el proceso padre en UTEMP, copiar el contenido de addr en UTEMP, mapear UTEMP en el proceso hijo en addr y desmapear UTEMP del padre.

* ¿Por qué se usa ROUNDDOWN(&addr) para copiar el stack? ¿Qué es addr y por qué, si el stack crece hacia abajo, se usa ROUNDDOWN y no ROUNDUP?

Se usa ROUNDOWN porque queremos el principio de la página.

Tarea: multicore_init
================

1. ¿Qué código copia, y a dónde, la siguiente línea de la función boot_aps()?

`memmove(code, mpentry_start, mpentry_end - mpentry_start);`

Copia el código de kern/mpentry.S (que es el entry code para los APs, que empezarán a ejecutar cuando arranquen) a la dirección MPENTRY_PADDR, que es accesible en real mode.

2. ¿Para qué se usa la variable global mpentry_kstack? ¿Qué ocurriría si el espacio para este stack se reservara en el archivo kern/mpentry.S, de manera similar a bootstack en el archivo kern/entry.S?

Se utiliza para setear el stack de cada CPU. Se necesita uno distinto para cada uno, por eso no se puede reservar en kern/mpentry.S.

3. Cuando QEMU corre con múltiples CPUs, éstas se muestran en GDB como hilos de ejecución separados. Mostrar una sesión de GDB en la que se muestre cómo va cambiando el valor de la variable global mpentry_kstack:

```
$ make qemu-nox-gdb CPUS=4

Booting from Hard Disk..6828 decimal is 15254 octal!
Physical memory: 131072K available, base = 640K, extended = 130432K
boot_alloc: initialized in f0283000
boot_alloc: allocating 4096 bytes (1 pages), starting in f0283000
boot_alloc: allocation ends in f0284000
mem_init: kern_pgdir allocated in f0283000
boot_alloc: allocating 262144 bytes (64 pages), starting in f0284000
boot_alloc: allocation ends in f02c4000
mem_init: pages allocated in f0284000
boot_alloc: allocating 126976 bytes (31 pages), starting in f02c4000
boot_alloc: allocation ends in f02e3000
boot_alloc: allocating 0 bytes (0 pages), starting in f02e3000
boot_alloc: allocating 0 bytes (0 pages), starting in f02e3000
check_page_free_list() succeeded!
check_page_alloc() succeeded!
check_page() succeeded!
mem_init: mapping pages (va[ef000000, ef040000] -> pa[00284000, 002c4000])
mem_init: mapping pages (va[eec00000, eec1f000] -> pa[002c4000, 002e3000])
mem_init: mapping kernel stack (va[efff8000, f0000000] -> pa[00119000, 00121000])
mem_init: mapping physical memory (va[f0000000, 00000000] -> pa[00000000, 10000000])
check_kern_pgdir() succeeded!
boot_alloc: allocating 0 bytes (0 pages), starting in f02e3000
check_page_free_list() succeeded!
check_page_installed_pgdir() succeeded!
SMP: CPU 0 found 4 CPU(s)
enabled interrupts: 1 2
SMP: CPU 1 starting
SMP: CPU 2 starting
...

// En otra terminal:
$ make gdb
gdb -q -s obj/kern/kernel -ex 'target remote 127.0.0.1:27000' -n -x .gdbinit
Reading symbols from obj/kern/kernel...
Remote debugging using 127.0.0.1:27000
warning: No executable has been specified and target does not support
determining executable automatically.  Try using the "file" command.
0x0000fff0 in ?? ()
(gdb) watch mpentry_kstack
Hardware watchpoint 1: mpentry_kstack
(gdb) continue
Continuing.
The target architecture is assumed to be i386
=> 0xf0100174 <boot_aps+92>:	mov    %esi,%ecx

Thread 1 hit Hardware watchpoint 1: mpentry_kstack

Old value = (void *) 0x0
New value = (void *) 0xf0252000 <percpu_kstacks+65536>
boot_aps () at kern/init.c:105
105			lapic_startap(c->cpu_id, PADDR(code));
(gdb) bt
#0  boot_aps () at kern/init.c:105
#1  0xf010022a in i386_init () at kern/init.c:55
#2  0xf0106646 in ?? ()
#3  0xf0100047 in entry () at kern/entry.S:86
(gdb) info threads
  Id   Target Id                    Frame
* 1    Thread 1.1 (CPU#0 [running]) boot_aps () at kern/init.c:105
  2    Thread 1.2 (CPU#1 [halted ]) 0x000fd0ae in ?? ()
  3    Thread 1.3 (CPU#2 [halted ]) 0x000fd0ae in ?? ()
  4    Thread 1.4 (CPU#3 [halted ]) 0x000fd0ae in ?? ()
(gdb) continue
Continuing.
=> 0xf0100174 <boot_aps+92>:	mov    %esi,%ecx

Thread 1 hit Hardware watchpoint 1: mpentry_kstack

Old value = (void *) 0xf0252000 <percpu_kstacks+65536>
New value = (void *) 0xf025a000 <percpu_kstacks+98304>
boot_aps () at kern/init.c:105
105			lapic_startap(c->cpu_id, PADDR(code));
(gdb) info threads
  Id   Target Id                    Frame
* 1    Thread 1.1 (CPU#0 [running]) boot_aps () at kern/init.c:105
  2    Thread 1.2 (CPU#1 [running]) 0xf01061cc in holding (lock=0x1) at kern/spinlock.c:42
  3    Thread 1.3 (CPU#2 [halted ]) 0x000fd0ae in ?? ()
  4    Thread 1.4 (CPU#3 [halted ]) 0x000fd0ae in ?? ()
(gdb) thread 2
[Switching to thread 2 (Thread 1.2)]
#0  0xf01061cc in holding (lock=0x1) at kern/spinlock.c:42
42		return lock->locked && lock->cpu == thiscpu;
(gdb) bt
#0  0xf01061cc in holding (lock=0x1) at kern/spinlock.c:42
#1  0xfee00000 in ?? ()
#2  0xf0106216 in spin_lock (lk=0x0) at kern/spinlock.c:64
#3  0x00000000 in ?? ()
(gdb) p cpunum()
$1 = 1
(gdb) thread 1
[Switching to thread 1 (Thread 1.1)]
#0  boot_aps () at kern/init.c:107
107			while(c->cpu_status != CPU_STARTED)
(gdb) p cpunum()
$2 = 0
(gdb) continue
Continuing.
=> 0xf0100174 <boot_aps+92>:	mov    %esi,%ecx

Thread 1 hit Hardware watchpoint 1: mpentry_kstack

Old value = (void *) 0xf025a000 <percpu_kstacks+98304>
New value = (void *) 0xf0262000 <percpu_kstacks+131072>
boot_aps () at kern/init.c:105
105			lapic_startap(c->cpu_id, PADDR(code));
(gdb)
```

4. En el archivo kern/mpentry.S se puede leer:

```
# We cannot use kern_pgdir yet because we are still
# running at a low EIP.
movl $(RELOC(entry_pgdir)), %eax
```
	* ¿Qué valor tendrá el registro %eip cuando se ejecute esa línea?
       Responder con redondeo a 12 bits, justificando desde qué región de memoria se está ejecutando este código.

       El archivo kern/mpentry.S fue mapeado a MPENTRY_PADDR, es decir 0x7000 (así fue definido en int/memlayout.h). Entonces, el valor de %eip redondeado a 12 bits también es 0x7000.

	* ¿Se detiene en algún momento la ejecución si se pone un breakpoint en mpentry_start? ¿Por qué?

        No se detiene, ya que el cpu todavía está en real mode y no tiene virtualización de memoria.

5. Con GDB, mostrar el valor exacto de %eip y mpentry_kstack cuando se ejecuta la instrucción anterior en el último AP. Se recomienda usar, por ejemplo:

```
(gdb) b *0x7000
Breakpoint 1 at 0x7000
(gdb) continue
Continuing.

Thread 2 received signal SIGTRAP, Trace/breakpoint trap.
[Switching to Thread 1.2]
The target architecture is assumed to be i8086
[ 700:   0]    0x7000:	cli
0x00000000 in ?? ()
(gdb) si 10
The target architecture is assumed to be i386
=> 0x7020:	mov    $0x10,%ax
0x00007020 in ?? ()
(gdb) continue
Continuing.

Thread 3 received signal SIGTRAP, Trace/breakpoint trap.
[Switching to Thread 1.3]
The target architecture is assumed to be i8086
[ 700:   0]    0x7000:	cli
0x00000000 in ?? ()
(gdb) si 10
The target architecture is assumed to be i386
=> 0x7020:	mov    $0x10,%ax
0x00007020 in ?? ()
(gdb) continue
Continuing.

Thread 4 received signal SIGTRAP, Trace/breakpoint trap.
[Switching to Thread 1.4]
The target architecture is assumed to be i8086
[ 700:   0]    0x7000:	cli
0x00000000 in ?? ()
(gdb) disable 1
(gdb) x/15i $eip
=> 0x0:	push   %ebx
   0x1:	incl   (%eax)
   0x3:	lock push %ebx
   0x5:	incl   (%eax)
   0x7:	lock ret
   0x9:	loop   0xb
   0xb:	lock push %ebx
   0xd:	incl   (%eax)
   0xf:	lock push %ebx
   0x11:	incl   (%eax)
   0x13:	lock push %esp
   0x15:	incl   (%eax)
   0x17:	lock push %ebx
   0x19:	incl   (%eax)
   0x1b:	lock push %ebx
(gdb) si 10
The target architecture is assumed to be i386
=> 0x7020:	mov    $0x10,%ax
0x00007020 in ?? ()
(gdb) x/15i $eip
=> 0x7020:	mov    $0x10,%ax
   0x7024:	mov    %eax,%ds
   0x7026:	mov    %eax,%es
   0x7028:	mov    %eax,%ss
   0x702a:	mov    $0x0,%ax
   0x702e:	mov    %eax,%fs
   0x7030:	mov    %eax,%gs
   0x7032:	mov    %cr4,%eax
   0x7035:	or     $0x10,%eax
   0x7038:	mov    %eax,%cr4
   0x703b:	mov    $0x121000,%eax
   0x7040:	mov    %eax,%cr3
   0x7043:	mov    %cr0,%eax
   0x7046:	or     $0x80010001,%eax
   0x704b:	mov    %eax,%cr0
(gdb) watch $eax == 0x121000
Watchpoint 2: $eax == 0x121000
(gdb) continue
Continuing.
=> 0x7040:	mov    %eax,%cr3

Thread 4 hit Watchpoint 2: $eax == 0x121000

Old value = 0
New value = 1
0x00007040 in ?? ()
(gdb) p $eip
$1 = (void (*)()) 0x7040
(gdb) p mpentry_kstack
$2 = (void *) 0x0
(gdb) si 6
=> 0x7046:	or     $0x80010001,%eax

Thread 4 hit Watchpoint 2: $eax == 0x121000

Old value = 1
New value = 0
0x00007046 in ?? ()
(gdb) x/5i $eip
=> 0x7046:	or     $0x80010001,%eax
   0x704b:	mov    %eax,%cr0
   0x704e:	mov    0xf0240e84,%esp
   0x7054:	mov    $0x0,%ebp
   0x7059:	mov    $0xf010025c,%eax
(gdb) p mpentry_kstack
$3 = (void *) 0x0
```

Como puede verse, el valor de $eip es 0x7040 y el de mpentry_kstack parece ser 0x0, pero más abajo se puede ver (en la instrucción 0x704e) que es 0xf0240e84.

Tarea: ipc_recv
================

Un proceso podría intentar enviar el valor númerico -E_INVAL vía ipc_send(). ¿Cómo es posible distinguir si es un error, o no? En estos casos:

```
// Versión A
envid_t src = -1;
int r = ipc_recv(&src, 0, NULL);

if (r < 0)
  if (!src)
    puts("Hubo error.");
  else
    puts("Valor negativo correcto.")
```
En esta versión ipc_recv tiene como parámtro a &src, al ser distinto de NULL la función le asignará 0 en caso de que haya un error al recibir la página.
```
// Versión B
int r = ipc_recv(NULL, 0, NULL);

if (r < 0)
  if (!thisenv->env_tf.tf_regs.reg_eax)
    puts("Hubo error.");
  else
    puts("Valor negativo correcto.")
```
En esta versión, en cambio, 'from_env_store' es NULL. Entonces podemos verificar el estado del registro eax, si hay un error es distinto de 0.

Tarea: sys_ipc_try_send
================

¿Cómo se podría hacer bloqueante esta llamada? Esto es: qué estrategia de implementación se podría usar para que, si un proceso A intenta a enviar a B, pero B no está esperando un mensaje, el proceso A sea puesto en estado ENV_NOT_RUNNABLE, y sea despertado una vez B llame a ipc_recv().

De la misma forma que existe el booleano para indicar que el proceso no está recibiendo en el momento, podría agregarse otro para esta situación, guardarse el id del proceso intentando mandar un mensaje y luego sea puesto es estado ENV_NOT_RUNNABLE. Cuando se llame a recv el proceso se fija si hay algún proceso tratando de comunicarse y se lo despierta. 


