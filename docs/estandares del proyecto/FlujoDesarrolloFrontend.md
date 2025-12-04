# Flujo de desarrollo con IA para migración del FRONTEND de VioletaCliente

## 1.- Copiar archivos legados del proyecto de C++
Agregar a la ruta del proyecto los archivos que consideremos que servirán para usarse como contexto. Debemos considerar agregar lo que sea conveniente que la IA considere que sea útil para la conversación o para refinar la especificación.
Copiar los forms y clases legadas a la carpeta `.cpp` del proyecto de 

Copiar documentos de especificación de backend, frontend, tipos de módulos, seguridad, etc. que sean necesarios.

Carpetas:
>docs

>github
 
>db



## 2.- Revisar los controles del módulo a migrar
**Revisar manualmente** o pedirle a GitHub Copilot que compare los controles del módulo legado con los controles disponibles en la galería de componentes generada por Lovable. En caso de que falte algún control, agregarlo a la galería de componentes personalizada utilizando lovable. Por ejemplo:

Desde el copilot situado en el proyecto de `Núcleo Frontend`:
**Prompt para Copilot:**
> Analiza el formulario legado `cpp/FormCatalogoUsuarios.*` y compáralo con los controles de la plantilla disponibles en la galería de componentes (src/pages/gallery) que es parte de este mismo proyecto. Cada componente del formulario legado debe tener un control similar en funcionamiento en la plantilla. Dame una lista de todos los controles o componentes que no has podido relacionar en la galería.

 ### Si se encuentra que falta algún control: 
 Pedirle a Lovable que genere el control faltante y lo agregue a la galería de componentes personalizada, generando diferentes variantes y ejemplos de uso.** Por ejemplo:

**1. Prompt para Lovable:**
> Genera un componente de tipo `DataGrid` que soporte las siguientes funcionalidades: filtrado por columnas, paginación, selección múltiple de filas y edición en línea. Crea diferentes variantes del componente y ejemplos de uso para cada una.

**2. Integrar los componentes de Lovable**:
Hacer pull usando SourceTree o Git Bash para traer los nuevos controles generados por Lovable a nuestro proyecto local. Verificar que los nuevos controles estén disponibles en la galería de componentes personalizada dentro del proyecto de `Núcleo Frontend`.

## 3.- Crear un prompt de especificaciones frontend
Crear un prompt para generar una especificación detallada de la función legada que se va a migrar. Se puede basar en otro prompt con funcionalidad similar.

Este prompt debería incluir:
- Instrucciones específicas para explorar el código fuente de la interfaz legada y la función backend correspondiente.
- Contexto adicional relevante (documentación del código legado, definiciones de tablas en SQL).
- Un esquema claro del contenido esperado en el documento de especificación.

Ejemplo: Se edita el documento y se cambia lo que sea diferente para el módulo que pensamos migrar, por ejemplo `.github/prompts/busquedas/proveedores/genera_specs_legacy_busqueda_proveedores.md`

## 4.- Revisión asistida del prompt de especificaciones frontend
Indicarle a Copilot que revise un prompt que genera un archivo de especificación de la parte legada.

En github copilot con el modelo GPT-5 (codex) en modo agente:

> Revisa el siguiente documento y corrígelo interactuando conmigo sobre los hallazgos para hacer el documento lo más preciso y completo posible, también evalúa que no haya archivos faltantes o ambigüedad: `.github/prompts/busquedas/proveedores/genera_specs_legacy_busqueda_proveedores.md`

## 5.- Ejecutar el prompt de especificación frontend
Indicarle a Copilot que genere un archivo de especificación tipo markdown con base a la especificación que mandemos, indicando que debe ser una especificación técnica que tiene como objetivo un programado que va a implementar la aplicación.

En github copilot con el modelo GPT-5 (codex) en modo agente:

> Ejecuta las tareas indicadas en el documento siguiente: `.github/prompts/busquedas/proveedores/genera_specs_legacy_busqueda_proveedores.md`

## 6.- Revisión del archivo de especificación frontend
Revisar el archivo generado de especificación y pulir lo que sea necesario.

Ejemplo revisar `spec-legacy-busqueda-proveedores.md`

## 7. Crear un prompt de proceso frontend
Crear un prompt para describir los casos de uso deducibles del módulo que se va a migrar, basándose en know how entendible en el código legado y en la especificación técnica generada en el paso anterior.

Ejemplo, se puede copiar y editar un prompt similar a `.github\prompts\7-generar_proceso-plantilla.md`

## 8. Revisión del prompt de proceso frontend
Indicarle a Copilot que revise el prompt que genera un archivo de proceso de la parte legada.

En github copilot con el modelo GPT-5 (codex) en modo agente:

> Revisa el siguiente documento y corrígelo interactuando conmigo sobre los hallazgos para hacer el documento lo más preciso y completo posible, también evalúa que no haya archivos faltantes o ambigüedad: `docs/{tipo_módulo}/{entidad_módulo}/proceso_legacy_{tipo_módulo}_{entidad_módulo}.md`

## 9.- Ejecutar el prompt de proceso frontend
Indicarle a Copilot que genere un archivo de proceso tipo markdown con base a la especificación que mandemos, indicando que debe ser un proceso detallado para migrar la funcionalidad del módulo legado al nuevo frontend.

En github copilot con el modelo GPT-5 (codex) en modo agente:
> Ejecuta las tareas indicadas en el documento siguiente: `docs/{tipo_módulo}/{entidad_módulo}/proceso_legacy_{tipo_módulo}_{entidad_módulo}.md`

## 10.- Crear prompt de migración frontend
Crear un prompt para detallas los comandos de migración de la función legada que se va a migrar. Se puede basar en una plantilla de un módulo con funcionalidad similar.

El documento debe contener:
- Una referencia a la estructura actualizada de peticiones y respuestas de la API de núcleo backend migrado **mencionando los endpoints (o el prefijo de la URL) que se usará¨**. Para ello es necesario actualizar el archivo `docs/api-docs.json` exportando desde Swagger la definición actualizada de la API.
- Una referencia a los nuevos componentes que se usarán en la migración, mencionando la galería de componentes (**solamente en caso de que se espere que no se deduzca adecuadamente por la IA**, probablemente no sea necesario si ya se hizo el paso 2 correctamente).
Por ejemplo se puede usar `.github/prompts/migrar_busqueda_proveedores.md`

En github copilot con el modelo GPT-5 (codex) en modo agente:
> Crea un documento de migración frontend basado en la especificación técnica `docs/busquedas/proveedores/` y el proceso de migración generados previamente. El documento debe detallar los pasos necesarios para migrar la funcionalidad del módulo legado al nuevo frontend, incluyendo referencias a la estructura actualizada de peticiones y respuestas de la API de núcleo backend migrado, así como a los nuevos componentes que se usarán en la migración. Usa el siguiente formato de documento: `.github/prompts/migrar_busqueda_proveedores.md`

## 11.- Indicarle a Copilot que revise el prompt para la migración.

En github copilot con el modelo GPT-5 (codex) en modo agente:

> Revisa el siguiente documento y corrígelo interactuando conmigo sobre los hallazgos para hacer el documento lo más preciso y completo posible, también evalúa que no haya archivos faltantes o ambigüedad: `.github/prompts/migrar_busqueda_proveedores.md`
## 11.- Implementar la migracion frontend
Decirle a Copilot que implemente la migración indicada en el documento correspondiente:

En github copilot con el modelo CLAUDE SONNET 4.5 en modo agente:
> Ejecuta las tareas indicadas en el documento siguiente: `.github/prompts/migrar_busqueda_proveedores.md`

## 12.- Seguimiento
Darle seguimiento a lo que va haciendo, pararlo si es necesario cuando este haciendo cosas incorrectas, aclararle y continuar.

## 13.- Pruebas e iteraciones
Probar la funcionalidad y revisar el código y lo que va generando, e iterar en modificar lo necesario hasta completar la funcionalidad tal y como la queremos.

------------------------------------------------------------------------
# Tips para desarrollar con Github Copilot
- Darle keep en cada logro y comitear. Revisar lo que se comitea, para
  detectar que la IA no haya modificado cosas fuera del alcance de su
  tarea.

- Usar los checkpoints cuando el prompt o lo que hizo copilot estuvo
  incorrecto.

- Agregar en un archivo .github/copilot-rules.md las indicaciones generales para el proyecto, tener ya reglas predeterminadas por ejemplo para scripts en python, indicaciones de nuestra plataforma de desarrollo, tipo de base de datos, etc.

- Interactuar con el agente, detenerlo, darle tips, preguntarle, precisar. etc.

- Hacer funcionalidades con alcance limitado, y hacer pruebas de uso en cada paso, para corregir lo que sea necesario y comitear cada vez que se logra el alcance.

- Cuando un motor en particular no logra completar una tarea, revisar el prompt, intentar simplificarla o usar otro motor. Chatgpt 5 y sonnet 4 han sido los que mejores resultados han dado.

- Abrir nuevos chats para cada nueva tarea diferente, con el objetivo de reducir el contexto con el que tiene que trabajar el LLM.
