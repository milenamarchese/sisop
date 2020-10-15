
TP2: Procesos de usuario
========================

env_alloc
---------

1. ¿Qué identificadores se asignan a los primeros 5 procesos creados? (Usar base hexadecimal.)

Como son los primero procesos creados, `e->env_id` es cero, por lo que los identificadores quedan de la siguiente manera:
```
>>> format(((1 << 12) & ~((1 << 10) - 1)) | 0, '08x')
'00001000'
>>> format(((1 << 12) & ~((1 << 10) - 1)) | 1, '08x')
'00001001'
>>> format(((1 << 12) & ~((1 << 10) - 1)) | 2, '08x')
'00001002'
>>> format(((1 << 12) & ~((1 << 10) - 1)) | 3, '08x')
'00001003'
>>> format(((1 << 12) & ~((1 << 10) - 1)) | 4, '08x')
'00001004'
```

2. Supongamos que al arrancar el kernel se lanzan NENV procesos a ejecución. A continuación se destruye el proceso asociado a envs[630] y se lanza un proceso que cada segundo muere y se vuelve a lanzar. ¿Qué identificadores tendrá este proceso en sus sus primeras cinco ejecuciones?

El identificador que corresponde al primer proceso asociado a `envs[630]` es:
```
>>> format(((1 << 12) & ~((1 << 10) - 1)) | 630, '08x')
'00001276'
```
Como cuando se destruye el proceso es el único espacio libre, el siguiente proceso se va a ubicar ahí, y va a tener el siguiente identificador:
```
>>> format(((0x1276 + (1 << 12)) & ~((1 << 10) - 1)) | 630, '08x')
'00002276'
>>>
```
De la misma forma, el siguiente será:
```
>>> format(((0x2276 + (1 << 12)) & ~((1 << 10) - 1)) | 630, '08x')
'00003276'
```
Y los restantes: `00004276`, `00005276`, `00006276`, etc...

env_init_percpu
---------------

La función env_init() hace una llamada a env_init_percpu() para configurar sus segmentos. Antes de ello, se invoca a la instrucción lgdt. Responder:

* ¿Cuántos bytes escribe la función lgdt, y dónde?

	Escribe en el registo GDTR y escribe 6 bytes, los 2 bytes menos significativos del operando de 6 bytes y una dirección de 4 bytes, siendo estos 4 los más significatiovs del mismo operando.

* ¿Qué representan esos bytes?

    La función escribe en el registro GDTR 16 bits que representan un límite (el tamaño de la tabla) y 32 bits que representan la dirección base de la tabla y 32 bits que representan la dirección base de la tabla.

env_pop_tf
----------

1. Dada la secuencia de instrucciones assembly en la función, describir qué contiene durante su ejecución:

	* El tope de la pila justo antes popal.

		La pila apunta al inicio de Trapframe, `uint32_t reg_edi`.

	* El tope de la pila justo antes iret.

		El tope es el `uintptr_t tf_eip`.

	* El tercer elemento de la pila justo antes de iret.

		El tercer elementos es `uint32_t tf_eflags`.

2. ¿Cómo determina la CPU (en x86) si hay un cambio de ring (nivel de privilegio)? Ayuda: Responder antes en qué lugar exacto guarda x86 el nivel de privilegio actual. ¿Cuántos bits almacenan ese privilegio?

	La CPU determina el cambio de ring (0, 1, 2, 3) con 2 bits. Compara el campo CPL (bits 0 y 1) del tf_cs, code segment register, con el campo CPL del registro cs acutal.

gdb_hello
---------


1. Poner un breakpoint en env_pop_tf() y continuar la ejecución hasta allí.

```
0x0000fff0 in ?? ()
(gdb) b env_pop_tf
Breakpoint 1 at 0xf0102fbe: file kern/env.c, line 466.
(gdb) c
Continuing.
The target architecture is assumed to be i386
=> 0xf0102fbe <env_pop_tf>:	push   %ebp

Breakpoint 1, env_pop_tf (tf=0xf01c0000) at kern/env.c:466
466	{
```

2. En QEMU, entrar en modo monitor (Ctrl-a c), y mostrar las cinco primeras líneas del comando info registers.

```
EAX=00044000 EBX=00010094 ECX=f0044000 EDX=00000202
ESI=00010094 EDI=00000000 EBP=f0118fd8 ESP=f0118fbc
EIP=f0102fbe EFL=00000092 [--S-A--] CPL=0 II=0 A20=1 SMM=0 HLT=0
ES =0010 00000000 ffffffff 00cf9300 DPL=0 DS   [-WA]
CS =0008 00000000 ffffffff 00cf9a00 DPL=0 CS32 [-R-]
```

3. De vuelta a GDB, imprimir el valor del argumento tf:

```
$1 = (struct Trapframe *) 0xf01c0000
```

4. Imprimir, con x/Nx tf tantos enteros como haya en el struct Trapframe donde N = sizeof(Trapframe) / sizeof(int).

```
(gdb) print sizeof(struct Trapframe) / sizeof(int)
$2 = 17
(gdb) x/17x 0xf01c0000
0xf01c0000:	0x00000000	0x00000000	0x00000000	0x00000000
0xf01c0010:	0x00000000	0x00000000	0x00000000	0x00000000
0xf01c0020:	0x00000023	0x00000023	0x00000000	0x00000000
0xf01c0030:	0x00800020	0x0000001b	0x00000000	0xeebfe000
0xf01c0040:	0x00000023
```

5. Avanzar hasta justo después del movl ...,%esp, usando si M para ejecutar tantas instrucciones como sea necesario en un solo paso:

```
(gdb) si 4
=> 0xf0102fc7 <env_pop_tf+9>:	popa   
0xf0102fc7	467		asm volatile("\tmovl %0,%%esp\n"
```

6. Comprobar, con x/Nx $sp que los contenidos son los mismos que tf (donde N es el tamaño de tf).

```
(gdb) x/17x $sp
0xf01c0000:	0x00000000	0x00000000	0x00000000	0x00000000
0xf01c0010:	0x00000000	0x00000000	0x00000000	0x00000000
0xf01c0020:	0x00000023	0x00000023	0x00000000	0x00000000
0xf01c0030:	0x00800020	0x0000001b	0x00000000	0xeebfe000
0xf01c0040:	0x00000023
```

7. Describir cada uno de los valores. Para los valores no nulos, se debe indicar dónde se configuró inicialmente el valor, y qué representa.

8. Continuar hasta la instrucción iret, sin llegar a ejecutarla. Mostrar en este punto, de nuevo, las cinco primeras líneas de info registers en el monitor de QEMU. Explicar los cambios producidos.

```
EAX=00044000 EBX=00010094 ECX=f0044000 EDX=00000202
ESI=00010094 EDI=00000000 EBP=f0118fd8 ESP=f0118fbc
EIP=f0102fbe EFL=00000092 [--S-A--] CPL=0 II=0 A20=1 SMM=0 HLT=0
ES =0010 00000000 ffffffff 00cf9300 DPL=0 DS   [-WA]
CS =0008 00000000 ffffffff 00cf9a00 DPL=0 CS32 [-R-]
```

9. Ejecutar la instrucción iret. En ese momento se ha realizado el cambio de contexto y los símbolos del kernel ya no son válidos.

* Imprimir el valor del contador de programa con p $pc o p $eip

```
(gdb) p $pc
$3 = (void (*)()) 0x800020
```

* Cargar los símbolos de hello con el comando add-symbol-file, así:

* volver a imprimir el valor del contador de programa

```
(gdb) p $pc
$4 = (void (*)()) 0x800020 <_start>
```

* Mostrar una última vez la salida de info registers en QEMU, y explicar los cambios producidos.

```
EAX=00000000 EBX=00000000 ECX=00000000 EDX=00000000
ESI=00000000 EDI=00000000 EBP=00000000 ESP=eebfe000
EIP=00800020 EFL=00000002 [-------] CPL=3 II=0 A20=1 SMM=0 HLT=0
ES =0023 00000000 ffffffff 00cff300 DPL=3 DS   [-WA]
CS =001b 00000000 ffffffff 00cffa00 DPL=3 CS32 [-R-]
```

10. Poner un breakpoint temporal (tbreak, se aplica una sola vez) en la función syscall() y explicar qué ocurre justo tras ejecutar la instrucción int $0x30. Usar, de ser necesario, el monitor de QEMU.

kern_idt
--------

1. ¿Cómo decidir si usar `TRAPHANDLER` o `TRAPHANDLER_NOEC`? ¿Qué pasaría si se usara solamente la primera?

Se debe usar `TRAPHANDLER_NOEC` en los casos en los que el CPU no pushea un código de error al stack del kernel cuando recibe la interrupción.
Lo que hace es pushear un cero para que se mantenga la misma estructura, si se usara siempre `TRAPHANDLER` habría que manejar los dos casos por separado más adelante.

2. ¿Qué cambia, en la invocación de handlers, el segundo parámetro (istrap) de la macro `SETGATE`? ¿Por qué se elegiría un comportamiento u otro durante un syscall?

Define si se trata de un trap o un interrupt. Es decir, en el primer caso va a permitir que ocurran otras interrupciones mientras se está ejecutando el handler, mientras que en el otro no.

3. Leer user/softint.c y ejecutarlo con make run-softint-nox. ¿Qué excepción se genera? Si es diferente a la que invoca el programa… ¿cuál es el mecanismo por el que ocurrió esto, y por qué motivos?

El programa utiliza la instrucción `int $14` desde user space, lo que genera un interrupt 13 (general protection), ya que sólo puede ser usada por el kernel.

user_evilhello
---------

La versión anterior de evilhello.c imprime un byte de la dirección de memoria del kernel, en este caso imprimió:

```
edi  0x00000000
esi  0x00000000
ebp  0xeebfdfd0
oesp 0xefffffdc
ebx  0x00000000
edx  0x00000000
ecx  0x00000000
eax  0x00000000
es   0x----0023
ds   0x----0023
trap 0x0000000e Page Fault
cr2  0xf010000c
err  0x00000005 [user, read, protection]
eip  0x00800039
cs   0x----001b
flag 0x00000082
esp  0xeebfdfb0
ss   0x----0023
```

Esa dirección de memoria es del kernel y el usuario no debería tener permisos para accederla.
