# Lab: shell

### Búsqueda en $PATH

* ¿Cuáles son las diferencias entre la _syscall_ `execve(2)` y la familia de _wrappers_ proporcionados por la librería estándar de C (libc) `exec(3)`?

`execve(2)` 

`int execve(const char *pathname, char *const argv[], char *const envp[])`

_pathname_: Ejecuta el programa referido en _pathname_. Hace que el programa actual sea reemplazado por el nuevo programa. Inicializa otro heap, stack y otros segmentos. 

`exec(3)`

La familia de wrappers reemplaza la imagen del programa en ejecución por la imagen de otro proceso dejando muchos atributos intactos, particularme el PID. 

En conclusión: `execve(2)` deja las condiciones para que el proceso existente ejecute un nuevo programa, mientras que, `exec(3)` modifica el proceso en ejecución para la nueva funcionalidad.


---

### Procesos en segundo plano

Sólo se pide la implementación de un proceso en segundo plano. No es necesario que se notifique de la terminación del mismo por medio de mensajes en la shell.

* Detallar cúal es el mecanismo utilizado.

Para el proceso en segundo plano se implementó un ciclo donde el programa recibe el flujo de entrada standard. De forma síncrona se esperan procesos hijos que quedaron estado zombie (terminan pero su padre ho hizo wait) con el flag WNOHANG. Luego se reemplazó por el challenge de señales.

---

### Flujo estándar

Haciendo 

```console
$ ls -C /home /noexiste >out.txt 2>&1
```

* Investigar el significado de este tipo de redirección y explicar qué sucede con la salida de `cat out.txt`. Comparar con los resultados obtenidos anteriormente.

`2>` significa: Redirección de stderr.
`&1` significa: Se debe redirigir hacia el primer file descriptor del comando. Se podría extender a `&n` con un cambio del struct exec.

La salida de cat será el error de ls de /noexiste y la salida de ls /home, en los otros ejemplos se redirecciona stdout y stderr a archivos separados.

En esta implementación solo se disponen de 3 posibles file descriptors, como supuesto se asume que siempre se redirige stderr al file descriptor donde se redirigió stdout.

---

### Tuberías simples (pipes)

* Investigar y describir brevemente el mecanismo que proporciona la syscall `pipe(2)`, en particular la sincronización subyacente y los errores relacionados.

`pipe(2)`

Devuelve dos file descriptors que conforman una tubería unidireccional de lectura-escritura. La información escrita por la tuberia está en el kernel hasta que se haga una operación de lectura.

---

### Variables de entorno temporarias

Luego de llamar a `fork(2)` realizar, por cada una de las variables de entorno a agregar, una llamada a `setenv(3)`.

* ¿Por qué es necesario hacerlo luego de la llamada a `fork(2)`?

La llamada a `setenv(3)` se hace después de `fork(2)` para no modificar las variables de entorno del proceso padre.

En algunos de los _wrappers_ de la familia de funciones de `exec(3)` (las que finalizan con la letra e), se les puede pasar un tercer argumento (o una lista de argumentos dependiendo del caso), con nuevas variables de entorno para la ejecución de ese proceso.

Supongamos, entonces, que en vez de utilizar `setenv(3)` por cada una de las variables, se guardan en un array y se lo coloca en el tercer argumento de una de las funciones de `exec(3)`.

* ¿El comportamiento es el mismo que en el primer caso? Explicar qué sucede y por qué.

Las funciones que aceptan como tercer argumento un arreglo de variables de entorno toman esas variables para la imagen del nuevo programa. Por otro lado, con setenv el programa ejecutado por exec(3) (sin el sufijo 'e') heredó todas las variables de entorno del proceso padre y además sumó o modificó las pasadas como parámetro a setenv(3).

* Describir brevemente (sin implementar) una posible implementación para que el comportamiento sea el mismo.

Para que el comportamiento sea el mismo se podría pasar como argumento la variable `extern char **environ;` como tercer parámetro a `exec(3)` con las modificaciones que se quieran para el proceso nuevo.

---

### Pseudo-variables

Investigar al menos otras dos variables mágicas estándar, y describir su propósito.

* `$$` expande el PID del proceso de la instancia de la shell en ejecución.

* `$0` expande el nombre de la shell o el script de la shell.

---

### Comandos built-in

* ¿Entre `cd` y `pwd`, alguno de los dos se podría implementar sin necesidad de ser built-in? ¿por qué? ¿cuál es el motivo, entonces, de hacerlo como built-in? (para esta última pregunta pensar en los built-in como `true` y `false`)

`pwd` puede no ser necesariamente built-in porque solo consulta en donde se encuentra parada la shell. En cambio, `cd` debe ser built-in, ya que modifica el estado de la shell. Implementar cd de forma no-built-in cambiaría el working directory de proceso hijo de la shell. El motivo por el cual `pwd` es built-in es porque hace que sea significativamente más eficiente.

---

### Challenge señales

* Explicar detalladamente cómo se manejó la terminación del mismo.

Para manejar las señales de solamente los procesos que quedan en estado zombie se agrego un handler de señales SIGCHILD con `sigaction(2)` y una stack de señales `sigaltstack(2)`. Para el stack se pide memoria según recomienda man, lo que puede parecer como un memory leak a lo largo de la ejecución del programa, según mi interpretación solamente se crea un stack al comienzo de la shell y se elimina al final. 

Para hacer wait solamente a las señales enviadas por procesos zombies se eligío filtrar por pgid. Según la firma de `waitpid(2)` si el primer argumento es '0' entonces solo va a esperar a las señales que tengan el mismo gpid que el proceso que la invoca. Se decidió cambiar el pgid con `setpgid(2)` a todos los procesos que no ejecuten un comando del tipo PIPE, considerando que es una llamada mucho menos costosa que `fork(2)` y se hace al menos una vez por cada uno. 

Como desafio se encontró que dentro del handler solo se pueden utilizar funciones async-signal-safe, las cuales conforman una [lista] muy acotada. Para imprimir correctamente el PID del proceso esperado entonces se confeccionó una función (bastante artesanal) para evitar el uso de printf y la comodidad de format.

[lista]: https://www.man7.org/linux/man-pages/man7/signal-safety.7.html

¿Por qué es necesario el uso de señales?

Para la solución asíncrona de los procesos en segundo plano se recurre a señales, ya que de forma síncrona solamente se le haría wait a los procesos en estado zombie a medida que ingrese input. Para desligar la entrada de input de la espera de los procesos.