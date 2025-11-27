# Flujo de desarrollo con IA para migración del FRONTEND de VioletaCliente

## 1.- Copiar archivos legados del proyecto de C++
Agregar a la ruta del proyecto los archivos que consideremos que servirán para usarse como contexto. Debemos considerar agregar lo que sea conveniente que la IA considere que sea útil para la conversación o para refinar la especificación.
Copiar los forms y clases legadas a la carpeta `.cpp` del proyecto de 

Copiar documentos de especificación de backend, frontend, tipos de módulos, seguridad, etc. que sean necesarios.

Carpetas:
>docs

>github
 
>db



# 2.- Revisar los controles del módulo a migrar
Pedirle a GitHub Copilot que compare los controles del módulo legado con los controles disponibles en la galería de componentes generada por Lovable. En caso de que falte algún control, agregarlo a la galería de componentes personalizada utilizando lovable. Por ejemplo:

Desde el copilot situado en el proyecto de `Núcleo Frontend`:
**Prompt para Copilot:**
> Analiza el formulario legado `FormCatalogoUsuarios` y compáralo con los controles disponibles en la galería de componentes que es parte de este mismo proyecto. Indica si falta algún control necesario para migrar el formulario y en caso de que falte, menciona cuál es.

 ## Si se encuentra que falta algún control: 
 Pedirle a Lovable que genere el control faltante y lo agregue a la galería de componentes personalizada, generando diferentes variantes y ejemplos de uso.** Por ejemplo:

**1. Prompt para Lovable:**
> Genera un componente de tipo `DataGrid` que soporte las siguientes funcionalidades: filtrado por columnas, paginación, selección múltiple de filas y edición en línea. Crea diferentes variantes del componente y ejemplos de uso para cada una.

**2. Integrar los componentes de Lovable**:
Hacer pull usando SourceTree o Git Bash para traer los nuevos controles generados por Lovable a nuestro proyecto local. Verificar que los nuevos controles estén disponibles en la galería de componentes personalizada dentro del proyecto de `Núcleo Frontend`.

# 3.- Crear un prompt de especificaciones frontend
Crear un prompt para generar una especificación detallada de la función legada que se va a migrar. Se puede basar en otro prompt con funcionalidad similar.

Ejemplo: Se edita el documento y se cambia lo que sea diferente para el módulo que pensamos migrar, por ejemplo `.github/prompts/genera_specs_legacy_busqueda_proveedores.md`

# 4.- Revisión asistida del prompt de especificaciones frontend
Indicarle a Copilot que revise un prompt que genera un archivo de especificación de la parte legada.

En github copilot con el modelo GPT-5 (codex) en modo agente:

> Revisa el siguiente documento y corrígelo interactuando conmigo sobre los hallazgos para hacer el documento lo más preciso y completo posible, también evalúa que no haya archivos faltantes o ambigüedad: `genera_specs_legacy_busqueda_vendedores.md`

5.- Indicarle a Copilot que genere un archivo de especificación tipo
markdown con base a la especificación que mandemos, indicando que debe
ser una especificación técnica que tiene como objetivo un programado que
va a implementar la aplicación.

 En github copilot con el modelo GPT-5 (codex) en modo agente:

PROMPT: Ejecuta las tareas indicadas en el documento siguiente:  
.github\\prompts\\genera_specs_legacy_busqueda\_[vendedores.md](https://vendedores.md)

 6.- Revisar el archivo generado especificación y pulir lo que sea
necesario.

Ejemplo revisar
[spec-legacy-busqueda-vendedores.md](https://spec-legacy-busqueda-vendedores.md)

 7.- Crear un prompt para detallas los comandos de migración de la
función legada que se va a migrar. Se puede basar en una plantilla de un
módulo con funcionalidad similar.

 Se edita el documento y se cambia lo que es diferente para el módulo
que pensamos migrar,  
.github\\prompts\\migrar_busqueda\_[vendedores.md](https://vendedores.md)

 8.- Indicarle a Copilot que revise el prompt para la migración.

 En github copilot con el modelo GPT-5 (codex) en modo agente:

PROMPT: Revisa el siguiente documento y corrígelo interactuando conmigo
sobre los hallazgos para hacer el documento lo más preciso y completo
posible, también evalúa que no haya archivos faltantes o ambigüedad:
.github\\prompts\\migrar_busqueda\_[vendedores.md](https://vendedores.md)

 9.- Decirle a Copilot que implemente la migración indicada en el
documento correspondiente:

 En github copilot con el modelo CLAUDE SONNET 4.5 en modo agente:

PROMPT: Ejecuta las tareas indicadas en el documento siguiente:  
.github\\prompts\\migrar_busqueda\_[vendedores.md](https://vendedores.md)

 10.- Darle seguimiento a lo que va haciendo, pararlo si es necesario
cuando este haciendo cosas incorrectas, aclararle y continuar.

 11.- Probar la funcionalidad y revisar el código y lo que va generando,
e iterar en modificar lo necesario hasta completar la funcionalidad tal
y como la queremos.

 12.- Opcionalmente iterar en alguna de las sugerencias de los próximos
pasos solo si se considera necesario.

 

------------------------------------------------------------------------

 

Tips para desarrollar con Github Copilot

 

- Darle keep en cada logro y comitear. Revisar lo que se comitea, para
  detectar que la IA no haya modificado cosas fuera del alcance de su
  tarea.

- Usar los checkpoints cuando el prompt o lo que hizo copilot estuvo
  incorrecto.

- Agregar en un archivo
  .github\\.[copilot-rules.md](https://copilot-rules.md) las
  indicaciones generales para el proyecto, tener ya reglas
  predeterminadas por ejemplo para scripts en python, indicaciones de
  nuestra plataforma de desarrollo, tipo de base de datos, etc.

- Interactuar con el agente, detenerlo, darle tips, preguntarle,
  precisar. etc.

- Hacer funcionalidades con alcance limitado, y hacer pruebas de uso en
  cada paso, para corregir lo que sea necesario y comitear cada vez que
  se logra el alcance.

- Cuando un motor en particular no logra completar una tarea, revisar el
  prompt, intentar simplificarla o usar otro motor. Chatgpt 5 y sonnet 4
  han sido los que mejores resultados han dado

- Abrir nuevos chats para cada nueva tarea diferente, con el objetivo de
  reducir el contexto con el que tiene que trabajar el LLM.
