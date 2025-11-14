# Clase `FuncionesGenericas`

## Resumen general
La clase `FuncionesGenericas` actúa como un contenedor de utilidades transversales usadas en múltiples módulos del sistema. Centraliza operaciones comunes relacionadas con:
- Manipulación y empaquetado de cadenas en buffers (serie de campos separados por terminadores nulos).
- Conversión y validación de fechas, horas, números, RFC, UUID, email, teléfonos y otros formatos de datos fiscales y comerciales.
- Formateo de cantidades numéricas (presentación, eliminación de ceros no significativos, construcción de folios, enunciados en letras, máscaras telefónicas, factores de artículos, UUID, etc.).
- Limpieza, normalización y filtrado de cadenas para cumplir reglas de negocio (CFDI 3.3/4.0, SQL, caracteres válidos, remoción de espacios redundantes, sanitización de comillas, sustitución por subrayados, etc.).
- Soporte para interfaz gráfica (word wrap manual, gradientes, foco forzado en controles ubicados en pestañas no activas, omitir columnas invisibles en grids, interacción con `VTListBox`).
- Conversión y extracción estructurada de datos JSON (navegación por rutas, obtención de valores tipados y construcción de arreglos JSON desde listas).
- Utilidades numéricas (comparación tolerante de flotantes, chequeos de igualdad, parseo flexible, validación de decimales).

Su función estratégica es reducir duplicación de lógica repetitiva, unificar criterios de validación y ofrecer primitivas seguras (por ejemplo `StrCpy`, filtros de caracteres y generación controlada de identificadores) facilitando mantenimiento y consistencia.

> Nota: La clase no mantiene estado (no hay atributos miembro); funciona como una colección de métodos utilitarios invocables sobre instancias efímeras.

---
## Constantes relevantes (definidas en el header)
Estas macros definen alfabetos permitidos o patrones de filtrado según distintas necesidades del dominio (captura de CFDI, validaciones fiscales y sanitización para SQL):
- `ALFANUMERICO_PUNTUACION`
- `ALFANUMERICO`
- `ALFANUMERICO_CON_PUNTO`
- `ALFANUMERICO_CON_PUNTO_CON_ESPACIO`
- `ALFANUMERICO_CFDI33`
- `ALFANUMERICO_CFDI40`

Usadas por métodos de validación / filtrado como: `ValidaCaracteresCadena`, `FiltraCadenaMysql`, `FiltraCadenaCfdi33`, `FiltraCadenaCfdi40`, `ValidaPassword`.

---
## Atributos / Variables miembro
La clase no declara miembros privados ni públicos; todos los comportamientos son funciones puras o con efectos sobre parámetros.

---
## Métodos
A continuación se documentan todos los métodos públicos encontrados en `ClassFuncionesGenericas.h` y su comportamiento según la implementación en `ClassFuncionesGenericas.cpp`.

### 1. Buffer & Serialización de Cadenas
| Método | Descripción | Parámetros | Retorno | Notas / Uso |
|--------|-------------|------------|---------|-------------|
| `char * AgregaPCharABuffer(char *cadena, char *buffer, char terminador=0)` | Copia una cadena C al buffer y añade un terminador indicado (por defecto `\0`). | `cadena`: origen; `buffer`: destino donde se escribe; `terminador`: separador. | Puntero a la siguiente posición disponible (buffer + longitud + 1). | Para construir paquetes contiguos de campos terminados en nulo. No valida longitud previa. |
| `char * AgregaPCharABufferSinCerosNoSignificativos(char *cadena, char *buffer, int max_decimales=7)` | Copia un número decimal removiendo ceros no significativos a la derecha, respetando un máximo de decimales. | `cadena`: número como C-string; `buffer`: destino; `max_decimales`: límite de evaluación. | Puntero al final (+1) de la cadena escrita. | Elimina parte decimal redundante; si todos los decimales son 0 remueve el punto. |
| `char * AgregaPCharABufferMySqlDateToStrDate(char *cadena, char *buffer)` | Convierte fecha MySQL `YYYY-MM-DD` a formato `DD/MM/YYYY` con correcciones mínimas (día/mes/ año cero ? mínimos válidos). | `cadena`: fecha MySQL; `buffer`: destino. | Puntero tras el fin (buffer + 11). | No valida longitud; asume formato correcto. |
| `char * AgregaPCharABufferMySqlTimeStampToStrDate(char *cadena, char *buffer)` | Convierte timestamp `YYYY-MM-DD HH:MM:SS` a `DD/MM/YYYY HH:MM:SS` corrigiendo cero en día/mes/año. | `cadena`: timestamp MySQL; `buffer`: destino. | Puntero tras el fin (buffer + 20). | Mantiene el separador espacio y la hora intacta. |
| `char * AgregaStringABuffer(AnsiString cadena, char *buffer, char terminador=0)` | Igual que `AgregaPCharABuffer` pero desde `AnsiString`. | `cadena`, `buffer`, `terminador`. | Puntero a siguiente posición libre. | Usa `memcpy` y añade terminador. |
| `char * ExtraePCharDeBuffer(char **puntero)` | Devuelve la cadena C actual y avanza el doble puntero tras el terminador. | `puntero`: referencia al cursor del buffer. | Puntero al inicio de la cadena extraída. | Útil para iterar secuencias empaquetadas. |
| `AnsiString ExtraeStringDeBuffer(char **puntero)` | Igual que anterior pero regresa `AnsiString`. | `puntero`. | Cadena extraída. | Internamente usa longitud y avanza. |

### 2. Fechas y Tiempos
| Método | Descripción | Parámetros | Retorno | Notas |
|--------|-------------|------------|---------|-------|
| `TDate MySqlDateToTDate(char *date)` | Convierte `YYYY-MM-DD` a `TDate` con mínimos: año<1900?1900, mes/día<1?1. | `date`: cadena. | `TDate`. | No valida estructura fuera de reemplazos. |
| `AnsiString StrToMySqlDate(AnsiString date)` | De `DD-MM-YYYY` a `YYYY-MM-DD`. | `date`. | Fecha formateada. | Usa `SubString` posiciones fijas. |
| `AnsiString DateToMySqlDate(TDate Date)` | Formatea `TDate` a `yyyy-mm-dd`. | `Date`. | Cadena. | Usa `DateTimeToString`. |
| `AnsiString TimeToMySqlTime(TTime Time)` | Formatea `TTime` a `hh:nn:ss`. | `Time`. | Cadena. | -- |
| `AnsiString ConvertirFecha(AnsiString fecha)` | Convierte de `YYYY-MM-DD` a `DD/MM/YYYY` mediante parseo `stringstream`. | `fecha`. | Nueva fecha. | Lanza excepción si formato inválido. |

### 3. Manipulación de Secciones / Listas
| Método | Descripción |
|--------|-------------|
| `AnsiString ExtraeSeccion(AnsiString cadena, int seccion, char separador='|')` Obtiene la sección N (0-based) separada por `separador`; si no existe devuelve vacío. |
| `int ExtraeNumSecciones(AnsiString cadena, char separador='|')` Cuenta secciones resultantes. |
| `AnsiString ObtieneClaveListBox(VTListBox *lb)` Devuelve la parte 0 de la cadena seleccionada (usa `ExtraeSeccion`). |
| `void AsignaClaveListBox(VTListBox *lb, AnsiString Clave)` Selecciona el item cuya clave (sección 0) coincide; si no lo encuentra, deselecciona. |
| `std::vector<AnsiString> split(AnsiString str, char delimiter)` Divide manualmente sin considerar delimitadores consecutivos como tokens vacíos al inicio; preserva el último token. |
| `void concatenarNoVacio(AnsiString & original, AnsiString nuevoDato)` Si `nuevoDato` no está vacío: concatena con coma y espacio. |
| `AnsiString commaStringToSqlStrings(AnsiString commaString)` Convierte lista separada por comas a lista SQL `'a', 'b', 'c'`. |

### 4. Teléfonos y Máscaras
| Método | Descripción | Parámetros | Retorno |
|--------|-------------|------------|---------|
| `AnsiString CreaMaskaraTelefono(int AnchoLada, int AnchoNumero, bool Forzar=false)` Genera máscara de entrada con paréntesis y guiones; usa '0' para posiciones forzadas y '9' opcional. | Lada, Número, Forzar. | Máscara. |
| `bool ValidaTelefono(AnsiString Telefono, int AnchoLada, int AnchoNumero, bool PermitirVacio=true)` Valida que un teléfono concuerde con máscara forzada y respeta configuración de vacío. | Teléfono, anchos, bandera. | `true/false`. |

### 5. Validaciones Generales
| Método | Propósito | Criterio esencial |
|--------|----------|-------------------|
| `bool ValidaUuid(AnsiString Uuid)` | Verifica formato estándar UUID v4-like (hex y guiones). | Expresión regular. |
| `bool ValidaEmail(AnsiString Email, bool PermitirVacio=true)` | E-mail básico evitando secuencias `@-`, `-.`, etc. | Regex + lógica adicional. |
| `bool ValidaNumero(AnsiString Campo)` | Verifica si es un solo dígito (regex `^[0-9]$`). | Limitado a 1 dígito. |
| `bool ValidaCadena(AnsiString Campo)` | Patrón alfanumérico combinado con espacios controlados y letras. | Regex compleja. |
| `bool ValidaPassword(AnsiString Password, bool PermitirVacio=false)` | Longitud entre 8 y 13, al menos 1 mayúscula, 1 minúscula, 1 número y sólo caracteres de `ALFANUMERICO`. | Conteo por regex carácter a carácter. |
| `int ValidaCaracteresCadena(AnsiString Cadxvalidar, AnsiString Validos)` | Retorna posición (1-based) del primer carácter no permitido o 0 si todos válidos. | Búsqueda secuencial. |
| `bool validaRFC(AnsiString rfc, AnsiString tipo_emp="")` | Validación RFC persona física/moral: longitud, composición, fecha válida, regex final. | Amplia lógica secuencial. |
| `bool SoloDecimales(AnsiString Cadena)` | Intenta `StrToFloat`; si falla, no es decimal. | Try/catch. |
| `bool ValidaDecimales(AnsiString Cadena)` | Usa `CadenaAFlotante` (que limpia formato) y valida conversión. | Try/catch. |
| `bool CadenaABooleano(AnsiString cadena)` | Interpreta `true/1` como true y `false/0` como false. Otros ? false. | Comparación en minúsculas. |
| `bool ObtenerBoolJsonValue(...)` | Recupera bool desde JSON (cadena) usando `StrToBool`; vacío ? false. | Depende de `jsonValueAt`. |

### 6. Numérico y Formateo
| Método | Descripción | Notas |
|--------|-------------|-------|
| `AnsiString FiltraCadenaNumeroDecimal(AnsiString Numero)` Quita espacios, comas y punto final sin decimales. Vacío?"0". |
| `double CadenaAFlotante(AnsiString cadena)` / `Extended CadenaAFlotanteExt(AnsiString cadena)` Convierte tras filtrar. | Acepta cadenas con comas. |
| `int CompararFlotantes(double Var1, double Var2, double Epsilon=1e-7)` Devuelve 0 si |Var1-Var2|<=Epsilon; 1 si Var1>Var2+Epsilon; -1 si Var1<Var2-Epsilon. | Comparación tolerante. |
| `bool EsCero(double Valor, double Epsilon=1e-7)` / overload con `AnsiString` | Determina si valor?0. | Usa `fabs`. |
| `bool SonIguales(double Var1, double Var2, double Epsilon=1e-7)` | Igualdad tolerante. | -- |
| `AnsiString GeneraEnunciadoDeCantidad(double Cantidad, int NumDecimales=2)` | Convierte número a texto en español y agrega parte centavos `XX/100 M.N.` | Soporta hasta billones (estructura de 15 dígitos). |
| `AnsiString FormateaCantidad(double/AnsiString Cantidad, int NumDecimales=2, bool SepMiles=true)` | Formatea con separador de miles opcional y N decimales. | Overloads para `Extended` y desde `AnsiString`. |
| `AnsiString FormateaCantidadExt(...)` | Igual pero usando `Extended`. | -- |
| `AnsiString EliminaCerosNoSignificativos(AnsiString numero)` | Remueve ceros finales en parte decimal y posible punto sobrante. | Conserva enteros si no hay punto. |
| `AnsiString FormateaArticuloFactor(AnsiString/double factor)` | Formatea a 3 decimales y elimina ceros no significativos. | Reutiliza `FormateaCantidad` + `EliminaCerosNoSignificativos`. |
| `AnsiString FormateaFolio(AnsiString Prefijo, AnsiString Sufijo, int Ancho)` | Inserta ceros intermedios hasta completar ancho; recorta si excede. | Prefijo + ceros + sufijo. |
| `AnsiString ProtegeCadenaAsteriscos(AnsiString Cadena, int AnchoFinal, int DerIzq=1)` | Rellena con `*` según modo: 0 derecha, 1 izquierda, 2 ambos. | Asegura longitud objetivo. |

### 7. Cadenas y Limpieza / Seguridad
| Método | Descripción |
|--------|-------------|
| `void RTrim(char *Cadena)` Elimina espacios a la derecha in-place (y terminadores redundantes). |
| `void StrCpy(char *CadenaDestino, char *CadenaOrigen, int LongitudMaxDest)` Copia segura (trunca si necesario) asegurando terminación nula. |
| `AnsiString LimpiaEspaciosSobrantes(AnsiString cadena)` Normaliza espacios múltiples a uno. |
| `AnsiString FiltraCadenaMysql(AnsiString cadena)` Sustituye caracteres no permitidos por `_` usando alfabeto ampliado. |
| `AnsiString FiltraCadenaCfdi33(AnsiString cadena)` Filtra manteniendo sólo caracteres válidos estándar CFDI 3.3. |
| `AnsiString FiltraCadenaCfdi40(AnsiString cadena)` Similar para CFDI 4.0. |
| `AnsiString limpiarComillas(AnsiString mensaje)` Elimina comillas simples y dobles. |
| `AnsiString respetarApostrofesSql(AnsiString original)` Escapa comillas simples duplicándolas (estándar SQL). |

### 8. Interfaz de Usuario / Visual
| Método | Descripción |
|--------|-------------|
| `void DibujaGradiente(int r1,g1,b1,r2,g2,b2, TRect &Rectangulo, TCanvas *CanvasDestino, bool Vertical)` Dibuja gradiente manual píxel a píxel (vertical u horizontal). |
| `int AppMessageBox(AnsiString msg, AnsiString error, int tipo=MB_OK)` Wrapper a `Application->MessageBox`. |
| `void WordWrap(AnsiString Cadena, AnsiString ArregloLineas[], int AnchoLinea, int MaxNumLineas)` Ajuste manual basado en separadores básicos (espacio, coma, punto). |
| `void OmitirColumnasVaciasDeGrid(TStringGrid *StringGridReporte, int &mColStringGridReporte, int ACol, bool &CanSelect)` Evita selección de columnas ocultas saltando a la visible más cercana. Requiere variable externa de tracking. |
| `void ForceFocus(TWinControl *control)` Cambia pestañas necesarias (PageControl/TabSheet) para poder hacer `SetFocus` sin excepción. |

### 9. Identificadores y UUID
| Método | Descripción |
|--------|-------------|
| `AnsiString GeneraUUID()` Genera GUID usando `CreateGUID`, ajusta segmentos y devuelve en formato estándar (subcadena parcial del primer bloque). Lanza excepción si falla. |
| `bool ValidaUuid(AnsiString Uuid)` (también listado en Validaciones). |

### 10. JSON Utilities
| Método | Descripción |
|--------|-------------|
| `TJSONArray * StringListAJSONArray(TStringList * lista)` Convierte cada línea no vacía en elemento JSON (string). |
| `TJSONArray * StringListAJSONArrayInteger(TStringList * lista)` Igual pero convierte a entero. |
| `TJSONValue * jsonValueAt(TJSONValue * jsonObject, AnsiString jsonPath, char delimiter=';', bool throwsOnNull=false)` Navega jerárquicamente un objeto JSON siguiendo claves separadas; retorna NULL o lanza excepción. |
| `AnsiString ObtenerAnsiStringJsonValue(...)` Obtiene valor string; NULL?NULL. |
| `int ObtenerIntJsonValue(...)` Convierte a entero (vacío?0). |
| `float ObtenerFloatJsonValue(...)` Convierte a flotante (vacío?0.0). |
| `bool ObtenerBoolJsonValue(...)` Convierte a booleano (vacío?false). |

### 11. Conversión Simple / Helpers
| Método | Descripción |
|--------|-------------|
| `AnsiString IntToAnsiString(int entero)` Alias de `IntToStr` con cast. |
| `AnsiString DateToAnsiString(TDateTime DateTime)` Usa `DateToStr`. |
| `AnsiString TimeToAnsiString(TDateTime DateTime)` Reusa `TimeToMySqlTime`. |
| `char * AnsiStringToCstr(AnsiString cadena)` Devuelve `c_str()` directo. |
| `AnsiString ObtenLetra(int Indice)` Devuelve código de columna Excel-like desde arreglo precalculado. |
| `AnsiString GeneraEnunciadoDeCantidad(double Cantidad, int NumDecimales=2)` (Listada en Numérico) |
| `AnsiString ConvertirFecha(AnsiString fecha)` (Listada en Fechas) |
| `AnsiString verificarValorNulo(AnsiString cadena)` Cadena vacía ? espacio simple. |

---
## Ejemplos de uso (deducidos)
- Serialización de parámetros: repetir `ptr = AgregaStringABuffer(valor, ptr);` para construir un paquete y luego consumirlo con `ExtraeStringDeBuffer(&cursor);`.
- Formateo de importe para UI: `FormateaCantidad(total, 2, true)` ? "12,345.67".
- Validación RFC antes de timbrado CFDI: `if(!validaRFC(rfc,"0")) ...`.
- Generar enunciado de cheque: `GeneraEnunciadoDeCantidad(1234.56)` ? "MIL DOSCIENTOS TREINTA Y CUATRO PESOS 56/100 M.N.".
- Limpieza para SQL seguro (mínimo): `FiltraCadenaMysql(cadenaUsuario)` o para conservar apostrofes en inserciones `respetarApostrofesSql(cadena)`.
- Acceso JSON profundo: `ObtenerIntJsonValue(obj,"cliente;direccion;cp")`.
- Creación de folio: `FormateaFolio("LP","22",8)` ? `LP000022`.

---
## Consideraciones y buenas prácticas
1. Muchos métodos asumen formato correcto (no se hacen validaciones exhaustivas de longitud); validar antes de llamar si la fuente es externa.
2. Métodos que retornan punteros dentro de buffers (`Agrega*`) requieren cálculo previo de capacidad total para evitar sobrescrituras.
3. `StrCpy` debe preferirse sobre `strcpy` estándar al copiar hacia buffers limitados.
4. Validaciones con regex pueden ser costosas si se llaman en bucles grandes; considerar caching si se detecta cuello de botella.
5. `GeneraUUID` modifica el primer bloque (toma sólo 5 caracteres del segmento inicial); verificar si esto es un requerimiento de negocio o si se requiere el GUID completo.
6. `ValidaNumero` actualmente sólo acepta un dígito; si se requiere validar números con más dígitos debería ajustarse el regex a `^[0-9]+$`.
7. `verificarValorNulo` devuelve espacio en blanco en vez de cadena vacía: documentar este comportamiento para evitar sorpresas en comparaciones.
8. Para listas JSON, la función ignora líneas vacías; si se requieren valores vacíos explícitos, habría que extender la lógica.

---
## Rol dentro del proyecto
`FuncionesGenericas` sirve como base reutilizable para capas de:
- Captura de datos (limpieza y validaciones antes de persistir o timbrar CFDI).
- Presentación (formateo consistente de cantidades, fechas, textos en UI y reportes).
- Generación de documentos fiscales (RFC, UUID, filtros CFDI 3.3/4.0, enunciado de importes, normalización de cadenas).
- Interacción con componentes visuales VCL (`TStringGrid`, `VTListBox`, `TPageControl`).
- Interoperabilidad (buffers C y estructuras JSON).

Su centralización reduce dispersión de lógica y facilita aplicar cambios globales (por ejemplo, nuevos criterios en validación de correos o formateo monetario).

---
## Posibles mejoras futuras (sugerencias)
- Separar en módulos temáticos (por ejemplo: `FgStrings`, `FgNumericos`, `FgFiscal`, `FgUI`, `FgJson`).
- Añadir pruebas unitarias especialmente para validaciones fiscales (`validaRFC`, `GeneraEnunciadoDeCantidad`, filtros CFDI) y comparaciones flotantes.
- Revisar consistencia de longitud permitida de `ValidaPassword` con la documentación en comentarios (comentario dice ">=6" pero código exige 8?13).
- Ampliar `ValidaNumero` para múltiples dígitos si el uso real lo requiere.
- Parametrizar alfabetos CFDI desde configuración externa si cambian reglas oficiales.

---
## Referencias cruzadas
- Funciones numéricas y de formateo se utilizan ampliamente en formularios como `FormVentas.cpp` y reportes (por ejemplo llamadas a `FormateaCantidad`).
- Filtros CFDI se relacionan con generación de comprobantes en clases como `ClassComprobanteFiscalDigital`.
- Manejo de listas y extracción se observa en lógicas de interfaz (list boxes y grids) dispersas en otros formularios.

---
Fin de la documentación de `FuncionesGenericas`.
