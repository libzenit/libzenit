# AGENTS.md — LibZenit

## Documentación de código

### 1. Headers (`*.h`) — Docblocks Doxygen

Toda función pública, typedef, struct o macro exportada en un header **debe** llevar un bloque de documentación Doxygen con el siguiente formato:

```c
/**
 * @brief Una línea que describe qué hace.
 *
 * Párrafo opcional con más detalle, precondiciones, postcondiciones,
 * notas sobre ownership, threadsafety, etc.
 *
 * @param param1 Descripción del parámetro.
 * @param param2 Descripción del parámetro.
 * @return Descripción del valor de retorno.
 */
```

- Usar `@param` (no `\param`).
- Usar `@return` (no `\return` ni `@returns`).
- Para structs/typedefs documentar los campos con `/**< */` inline.

### 2. Implementaciones (`*.c`) — Comentario línea por línea

Cada función implementada **debe** documentar su lógica interna con comentarios `/* ... */` o `//` que expliquen **qué hace cada línea o bloque** y, cuando sea relevante, **por qué** se hace así.

No se trata de repetir el ANSI C en español — se trata de explicar la intención:

```c
/* Calcular el índice de la tabla de transiciones — O(n) */
for (size_t i = 0; i < state->count; i++) {
    /* Agarrar puntero a la i-ésima regla (evita copiar el struct) */
    const zenit_state_transition_t *t = &state->table[i];
```

### 3. Verificación

Antes de dar una tarea por terminada, el agente debe:

1. Verificar que toda función pública nueva tenga docblock en el header.
2. Verificar que toda implementación nueva tenga comentarios línea por línea.
3. Ejecutar `cmake --build build` (o el sistema de build correspondiente) para confirmar que no haya errores de compilación.
4. Ejecutar `ctest --test-dir build` para confirmar que los tests sigan pasando.

---

## Protocolo de cierre (merge workflow)

Cuando el usuario diga **"Acabo de fusionar tu rama"** (o cualquier variante que indique que el merge ya se completó), el agente debe ejecutar la siguiente rutina:

```
git checkout master
git pull
```

Esto garantiza que el agente vuelve a la rama principal con la versión más reciente del código fusionado.

- No hacer fetch ni rebase a menos que se indique explícitamente.
- Si la rama actual ya es `master`, igual hacer `git pull` para actualizar.
- Reportar al usuario el resultado de `git pull` (fast-forward o already up-to-date).
