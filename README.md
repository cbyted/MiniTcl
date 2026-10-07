# Compilador de Tcl en C

Este proyecto es un compilador para el lenguaje Tcl, desarrollado en C. Actualmente incluye un lexer para el lenguaje y un lexer independiente para los argumentos del comando `expr`. Estas dos etapas del compilador parecen funcionar para el lenguaje definido, aunque también considero que hay optimizaciones que se pueden hacer.

## Estructura del proyecto

- `include/`: archivos de cabecera.
- `src/`: código fuente del compilador.
- `test/`: pruebas escritas en Tcl.
- `Makefile`: reglas para compilar, limpiar y generar una compilación de depuración.
- `README.md`: este archivo.

## Compilar

Desde la raíz del proyecto, ejecuta:

```sh
make
```

El comando crea el ejecutable `minitcl` en la raíz del proyecto. Los demás archivos generados durante la compilación se guardan en la carpeta `bin/`, que se crea automáticamente si no existe.

Para compilar con opciones útiles para depuración y uso de GDB:

```sh
make debug
```

## Limpiar los archivos generados

Para eliminar los archivos generados por la compilación, ejecuta:

```sh
make clean
```

## Ejecutar pruebas

Las pruebas de código fuente de Tcl se encuentran en `test/`. Para ejecutar una, usa el ejecutable desde la raíz del proyecto y proporciona la ruta del archivo de prueba:

```sh
./minitcl test/lexer/file.tcl
```

Sustituye `file.tcl` por el nombre de la prueba a ejecutar. 

Puedes agregar esta sección al README para aclarar cómo se prueban actualmente las expresiones de `expr`:

## Pruebas del lexer de `expr`

A diferencia de las pruebas generales de Tcl, las pruebas del lexer y parser de `expr` todavía no están en archivos separados. Por ahora, se ejecutan desde el propio programa mediante un arreglo de cadenas llamado `exprTests`, que contiene expresiones de prueba, por ejemplo:

```c
const char *exprTests[] =
{
    "$VAR + 1.23 * 2 ** 3",
    "a >= b && a != 0",
    "a <= b || a > b && a < 100",
    "a = min(12, 1.23) + max(2, 4)",
    "!true || false && true",
    "8 / 2 + 7 % 3 - 1",
    "sqrt(9) + abs(-2) * min(1, 2)",
    "$x + max(1, 2) * 3 >= 4 && !false || true",
    "($VAR ** 2 + 1.23) / 2 >= min(12, 4) && true"
};
```

Estas expresiones se añadieron como pruebas rápidas durante el desarrollo. Más adelante se pueden trasladar a archivos de prueba independientes para organizarlas mejor.
