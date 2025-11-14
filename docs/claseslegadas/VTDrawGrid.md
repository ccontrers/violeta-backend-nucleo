# VTDrawGrid

## Resumen general
`VTDrawGrid` es una derivación de `TDrawGrid` que incorpora los estados comunes del framework (bloqueo lógico y modo de solo lectura) y gestiona respaldo de posición de celda seleccionada para restauración. Permite personalizar la interacción de teclado y foco con un formato visual coherente.

## Objetivo de la clase
Proporcionar un grid dibujado manualmente con control de edición centralizado, capaz de recordar la celda activa al entrar y salir de modos de bloqueo o de restaurar la selección tras obtener foco.

## Atributos privados
- `TColor color_original`: Color base antes de efectos de foco.
- `bool FBloqueado`: Estado lógico que evita selección o edición interna.
- `bool mRespaldoEnabled`: Guarda `Enabled` real para reversión.
- `bool FReadOnly`: Modo consulta donde la navegación puede permitirse pero no la edición (si existiera).
- `int mRespaldoRow`: Fila previamente seleccionada.
- `int mRespaldoCol`: Columna previamente seleccionada.

## Métodos protegidos
- `DoEnter()/DoExit()`: Gestionan realce visual y restablecimiento de selección.
- `KeyDown(Word &Key, TShiftState Shift)`: Control de navegación (flechas, PgUp, PgDn, Enter) respetando bloqueo.
- `GetEnabled()/SetEnabled(bool)`: Ajuste de habilitación en presencia de bloqueo.
- `GetBloqueado()/SetBloqueado(bool)`: Control directo del estado de bloqueo.

## Métodos públicos
- `VTDrawGrid(TComponent* Owner)`: Constructor inicializa estados y color.
- `Click()`: Maneja selección/cambio de celda; puede validar bloqueo antes de continuar con la lógica base.

## Propiedades publicadas
- `Bloqueado (bool)`: Previene interacción activa.
- `ReadOnly (bool)`: Indica modo sin modificación.

## Notas de implementación inferidas
El respaldo de fila/columna permite volver a la celda original tras perder foco o al salir de un modo temporal (p.ej., validaciones). El color original se usa para destacar el foco.

## Ejemplo hipotético
```cpp
VTDrawGrid *grid = new VTDrawGrid(this);
grid->ColCount = 5;
grid->RowCount = 10;
grid->Bloqueado = false;
```

## Integración
Se alinea con el conjunto de componentes para manejar de forma uniforme la edición en formularios complejos donde varios controles cambian de modo simultáneamente.

---
