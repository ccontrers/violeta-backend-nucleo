# ClassArregloTransacciones

## 1. Resumen
Estructura de colección estática (tamaño máximo fijo `MAX_TRANSACCIONES=30000`) que almacena transacciones tanto de Cuentas por Pagar (CXPAG) como de Cuentas por Cobrar (CXCOB). Cada transacción se modela mediante `ElementoTransacciones`, que concentra datos operativos (fechas, folios, importes, saldos) y de comisiones (tolerancias, porcentajes, montos calculados), además de campos auxiliares para control comercial (cliente, vendedor/cobrador, ubicaciones, condiciones). Provee dos operaciones principales: limpiar el arreglo (`BlanqueaArreglo`) y recalcular totales (`CalculaTCargosAbonosYSaldo`).

## 2. Objetivos / Responsabilidad Principal
- Servir como contenedor en memoria para un conjunto masivo de transacciones precargadas para operaciones de consulta, cálculo de saldos y posible despliegue en UI.
- Facilitar el cálculo rápido de totales (cargos, abonos, saldo) sin recalcular individualmente cada vez.
- Encapsular (aunque de forma plana y pública) la información relevante de créditos/pagos y datos para cálculo de comisiones.

## 3. Alcance y No Alcance
Alcance:
- Gestión simple de un buffer indexado de transacciones.
- Cálculo acumulado de cargos (valores >= 0) y abonos (valores < 0).
- Reset completo de estado.

No Alcance (implícito):
- Inserción segura con validaciones de límites (no hay método Add).
- Persistencia u operaciones de E/S a base de datos.
- Cálculo de comisiones a partir de reglas; solo almacena datos resultantes.
- Filtrado, ordenamiento o búsqueda avanzada.

## 4. Dependencias
- `ClassFuncionesGenericas` (instancia `mFg`) para comparación de flotantes (`CompararFlotantes`).
- Tipos VCL/Builder (`AnsiString`).

## 5. Estructuras y Atributos Clave
### 5.1 `ElementoTransacciones`
Campos agrupados por temática:
- Identificación básica: `FechaTran`, `Tipo`, `TraCredito`, `Referencia`, `FolioProv`, `uuid`.
- Pago / documento: `FormaPago`, `FechaCheque`, `StatusCheque`, `TipoCheque`, `RefPago`, `Terminos`, `Ubicacion`, `TipoNotaCred`, `notasSinUuid`.
- Valores económicos: `Valor`, `Saldo`, `SaldoEnCheques`.
- Vencimiento y control: `FechaVenc`, `HoraTran`, `Sucursal`.
- Notas y referencias: `NotaProv`, `NotaCProv`.
- Actores: `Comprador`, `Cobrador`.
- Datos de comisiones y venta origen: `FolioFisico`, `ToleranciaComision`, `VentaOrigen`, `TraPagoOrigen`, `CveCliente`, `NomCliente`, `FechaRecuperacion`, `FechaLimite`, `ValorVenta`, `TotComisionVenta`, `DiasTardoPago`, `AplicaComision`, `PorcentajeComision`, `ValorComision`, `SaldoReal`, `ChequesNoCobrados`, `TotalNCredito`, `TotalNCargo`, `FechaUCheque`, `CveUsuarioAlta`, `NomUsuAlta`, `NumChequeYBanco`, `Termino`.

### 5.2 `ArregloTransacciones`
- `Elemento[MAX_TRANSACCIONES]`: almacenamiento principal.
- `TotalCargos`, `TotalAbonos`, `Saldo`: agregados calculados.
- `UsoDelArreglo`: 0 = CXP (pagar), 1 = CXC (cobrar).
- `NumTransacciones`: contador lógico (último índice cargado). Se asume índices 1..Num (dado el cálculo de totales inicia en 1).

## 6. Métodos
### 6.1 `BlanqueaArreglo()`
Recorre desde índice 0 hasta `NumTransacciones` limpiando cada campo (cadenas a "", numéricos a 0, booleanos a false). Luego pone `NumTransacciones=0`, totales a 0. No valida límites. Complejidad: O(n) respecto a número de transacciones previas.

Observaciones:
- Incluye índice 0 aun cuando el cálculo de totales omite 0 (patrón sugiere que 0 podría estar vacío o reservado).
- Si previamente se habían cargado más elementos que el nuevo `NumTransacciones` final (0), quedan limpiados.

### 6.2 `CalculaTCargosAbonosYSaldo()`
Itera de 1 a `NumTransacciones`:
- Usa `mFg.CompararFlotantes(Elemento[i].Valor,0)` para decidir signo.
- Suma valores >=0 a `TotalCargos`, los negativos (en valor absoluto) a `TotalAbonos`.
- Luego `Saldo = TotalCargos - TotalAbonos`.

Limitaciones actuales:
- No inicializa `TotalCargos` ni `TotalAbonos` dentro del método (si fueron previamente usados, riesgo de acumulación indeseada a menos que se hayan reiniciado antes manualmente). En el flujo correcto se debería llamar `BlanqueaArreglo()` o manualmente poner a 0 antes de recalcular.

## 7. Flujo de Uso Típico
1. Inicializar estructura (constructor implícito zero-inicializado si es global/estático, sino se requiere set manual).
2. Cargar datos externos en `Elemento[i]` incrementando `NumTransacciones` (lógica externa no incluida aquí).
3. Llamar `TotalCargos=TotalAbonos=Saldo=0` (recomendado) o `BlanqueaArreglo()` si es un reinicio completo.
4. Invocar `CalculaTCargosAbonosYSaldo()` para obtener agregados.
5. Consultar `TotalCargos`, `TotalAbonos`, `Saldo` y recorrer elementos para UI/reportes.
6. Ante necesidad de limpiar y recargar, usar `BlanqueaArreglo()`.

## 8. Reglas y Convenciones Implícitas
- Indexación lógica inicia en 1 para cálculos, dejando 0 como potencial centinela.
- Valores negativos representan abonos (pagos, devoluciones, notas de crédito) y positivos cargos (deudas, ventas, facturas emitidas) en el contexto de CXC/CXP.
- `UsoDelArreglo` no se utiliza internamente para ramas de lógica; su semántica depende de capas externas.

## 9. Riesgos y Debilidades
| Riesgo | Descripción | Impacto | Mitigación Propuesta |
|--------|-------------|---------|----------------------|
| Falta de encapsulación | Todos los campos son públicos, se puede romper invariantes fácilmente | Alto (datos inconsistentes) | Proveer métodos Add / Clear / Recalculate y setters validados |
| Recalculo acumulativo | `CalculaTCargosAbonosYSaldo` no reinicia acumuladores | Resultados inflados si se llama varias veces | Reiniciar internamente o documentar precondición explícita |
| Límite Fijo 30000 | Riesgo de overflow silencioso si se supera | Pérdida de datos / corrupción | Validar antes de insertar; usar contenedor dinámico (std::vector) |
| Indexación mixta 0/1 | Limpia desde 0 pero calcula desde 1 | Confusión y potencial omisión del primer elemento válido | Uniformar (usar 0..Num-1) y ajustar loops |
| Uso de `AnsiString` | Dependencia de VCL y encoding ambiguo | Migración compleja y errores de internacionalización | Plan de refactor a `std::string` / UTF-8 |
| Campos heterogéneos | Mezcla datos de comisión, documento, control operativo | Clase inflada y poco cohesionada | Extraer estructuras temáticas (Finanzas, Comisiones, Metadata) |
| Sin control de concurrencia | Escrituras concurrentes no protegidas | Corrupción de memoria en escenarios multi-hilo | Añadir mutex o limitar a thread UI |
| Falta de constructor | Estados no inicializados garantizados solo si está en segmento BSS | Errores sutiles en instancias locales | Implementar constructor que ponga totales y contador en 0 |

## 10. Mejoras Propuestas
1. Refactor a contenedor dinámico (`std::vector<ElementoTransaccion>`), añadiendo método `Agregar(const ElementoTransaccion&)` que valide límites.
2. Separar: a) Datos documento; b) Métricas de comisión; c) Estado de cobranza. Aplicar composición.
3. Implementar método `RecalcularTotales()` que siempre ponga a 0 los acumuladores antes de iterar.
4. Cambiar semántica de índices: usar 0..size-1 coherente con STL.
5. Añadir función `Clasificar()` (cargos vs abonos) y quizá mantener listas separadas para consultas rápidas.
6. Introducir tipo fuerte para montos (`Money`, decimal fijo) vs `double` para evitar problemas de redondeo.
7. Convertir `UsoDelArreglo` en enum `enum class TipoUso { CXP, CXC };`.
8. Proveer serialización (JSON/CSV) para diagnóstico y pruebas automatizadas.
9. Añadir pruebas unitarias: casos solo cargos, solo abonos, mixto, llamada doble a recalculo, valor límite MAX.
10. Documentar invariantes: `NumTransacciones <= MAX_TRANSACCIONES`, acumuladores coherentes con suma de elementos.

## 11. Contrato y Pre/Post Condiciones (propuesto)
- Pre: Antes de `CalculaTCargosAbonosYSaldo`, `NumTransacciones` refleja el número real de elementos cargados (>=0), y acumuladores están en 0.
- Post: `TotalCargos` = suma(Elemento[i].Valor >=0); `TotalAbonos` = suma(|Elemento[i].Valor| de Valor<0); `Saldo = TotalCargos - TotalAbonos`.
- Invariante: Para cualquier i válido, campos numéricos no deben contener NaN; `NumTransacciones` nunca excede `MAX_TRANSACCIONES`.

## 12. Ejemplo de Uso Simplificado (Futuro Refactor)
```cpp
ArregloTransacciones arr;
arr.BlanqueaArreglo();
// Supongamos agregamos 2 transacciones externamente
arr.NumTransacciones = 2;
arr.Elemento[1].Valor = 1500.00; // Cargo
arr.Elemento[2].Valor = -300.00; // Abono
arr.TotalCargos = arr.TotalAbonos = arr.Saldo = 0; // Asegurar reinicio
arr.CalculaTCargosAbonosYSaldo();
// Resultado esperado: TotalCargos=1500, TotalAbonos=300, Saldo=1200
```

## 13. Impacto en Otros Módulos
- Usado presumiblemente por reportes de cartera (CXC) y módulo de pagos/proveedores (CXP) para presentar listas y totales.
- Campos de comisión podrían ser consumidos por un motor externo de cálculo (no presente aquí) o por reportes de desempeño de cobranza.
- La falta de encapsulación obliga a que cualquier módulo externo mantenga disciplina manual, aumentando probabilidad de errores cruzados.

## 14. Observaciones de Calidad
- Diseño refleja enfoque procedimental previo a adopción de contenedores STL modernos.
- Alto acoplamiento implícito a orden de llamadas y disciplina externa.
- Documentación inline insuficiente en la implementación (métodos sin comentarios detallados). Este archivo compensa.

## 15. Resumen Ejecutivo
`ArregloTransacciones` es un buffer estático masivo para transacciones financieras de CXP/CXC con funciones mínimas (limpieza y cálculo de totales). Su diseño actual sacrifica encapsulación y robustez a cambio de simplicidad y acceso directo. Las mejoras sugeridas se centran en: uso de contenedores dinámicos, separación de responsabilidades, fortalecimiento de invariantes y precisión en cálculos monetarios.

---
Última revisión: 2025-09-22
