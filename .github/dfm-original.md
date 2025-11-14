# DFM original — formularios legacy

Los archivos `.dfm` contienen la definición de formularios y controles en C++ Builder. Son la referencia para construir los componentes React.

## Qué extraer de los .dfm
- Lista de campos y tipos (input, select, checkbox).
- Layout y agrupación (pestañas, paneles).
- Validaciones/máscaras (Lada, Teléfono, CP, RFC).
- Comportamientos (visible/readonly/enable) según estado.

## Reglas de interpretación
- Crear un mapa `nombre_legacy → nombre_nuevo` por formulario.
- Documentar defaults y comportamiento esperado de cada campo.
- No replicar acoplamientos UI-negocio; mover lógica a Services/DTOs.

## Referencias
- Ver: `.github/frontend-general.md` para patrones UI y estado.