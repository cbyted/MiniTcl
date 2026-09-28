## 8. Contrato del analizador léxico

El lexer principal debe producir estos tokens:
```
```
| Token | Ejemplo | Valor guardado |
|---|---|---|
| `NAME` | `set`, `total_1` | texto |
| `TEXT` | `-nonewline`, `3.14` | texto |
| `VAR` | `$total` | nombre sin `$` |
| `ARRAY_VAR` | `$datos(3)` | `(nombre, índice)` |
| `BRACED` | `{set x 1}` | contenido sin llaves |
| `STRING` | `"hola $x"` | contenido sin comillas |
| `COMMAND_SUBST` | `[expr {$x+1}]` | contenido sin corchetes |
| `NEWLINE` | uno o más saltos | texto de los saltos |
| `SEMI` | `;` | `;` |
```
```
### Prioridad de reconocimiento

1. espacios horizontales;
2. saltos de línea;
3. `;`;
4. comentarios `#...`;
5. bloques balanceados `{...}`;
6. comandos balanceados `[...]`;
7. cadenas `"..."`;
8. variables `$x` y `$a(i)`;
9. palabras `NAME`/`TEXT`;
10. error.

Para mantener compatibilidad con el lexer inicial, `#` inicia un comentario siempre que aparece fuera de llaves, corchetes o comillas. Esto es una simplificación deliberada respecto de Tcl completo.

lexer que debe realizar el estudiante


Implemente `MiniTclLexer.tokenize(source)` sin utilizar generadores de analizadores léxicos. Puede usar expresiones regulares para nombres y números, pero los delimitadores balanceados deben recorrerse carácter a carácter.

### Requisitos funcionales

1. Registrar línea, columna e índice absoluto de cada token.
2. Ignorar espacio, tabulador y retorno de carro.
3. Agrupar saltos consecutivos en un `NEWLINE`.
4. ignorar comentarios hasta el salto de línea, conservando el `NEWLINE` posterior;
5. admitir anidamiento de `{...}` y `[...]`;
6. no cerrar un grupo cuando el delimitador esté escapado;
7. conservar los escapes como dos caracteres para una fase posterior;
8. reconocer identificadores con `[A-Za-z_][A-Za-z0-9_]*`;
9. informar delimitadores sin cerrar y variables mal formadas;
10. finalizar sin token `EOF`, para ser compatible con el parser entregado.

> **Separación de responsabilidades:** el lexer de MiniTcl no tokeniza los operadores dentro de `{...}`. El comando `expr` entrega ese contenido a un segundo lexer especializado.
