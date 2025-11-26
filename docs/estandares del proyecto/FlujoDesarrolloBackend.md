# Flujo de desarrollo con IA para migración del backend de VioletaServidor

 

## Criterio actual
Un agente de IA enfocado al desarrollo es un
programador muy ágil, con mucho conocimiento técnico en especial sobre
los dominios más populares, pero le falta visión, empatía, tiene límite
de contexto y en dominios técnicos poco populares no es tan eficiente,
por lo que tendremos que complementar dichas áreas, darle reglas donde
veamos que falla más y darle tareas de horizonte corto.

### 1.- Preparación previa de archivos
Agregar a la ruta del proyecto los archivos que consideremos que
servirán para usarse como contexto. Debemos considerar agregar lo que
sea conveniente que la IA considere que sea útil para la conversación o
para refinar la especificación.

#### 1.1 Copiar los forms y clases legadas faltantes a la carpeta `cpp`

#### 1.2 Copiar documentos de especificación  de backend, frontend, tipos de módulos, seguridad, etc. que sean necesarios.

 Carpetas:

>docs
github
db

### 2.- Abrir el proyecto en VS code con la extensión Github Copilot activa.

> Ejemplo: `c:/github/violetaserver`

### 3.- Generar especificaciones (specs)
 Crear un prompt para generar una especificación detallada de la
función legada que se va a migrar. Se puede basar en otro prompt con
funcionalidad similar.

>Ejemplo: Se edita el documento y se cambia lo que sea diferente para el
módulo que pensamos migrar, por ejemplo `genera_specs_legacy_busqueda_vendedores.md`

### 4.- Revisión de generación de especificaciones
Indicarle a Copilot que revise un prompt que genera un archivo de especificación de la parte legada.

 En github copilot con el modelo GPT-5 (codex) en modo agente:

PROMPT: 
> Revisa el siguiente documento y corrígelo interactuando conmigo sobre los hallazgos para hacer el documento lo más preciso y completo posible, también evalúa que no haya archivos faltantes o ambigüedad: `genera_specs_legacy_busqueda_vendedores.md`

### 5.- Generar las especificaciones
Indicarle a Copilot que genere un archivo de especificación tipo markdown con base a la especificación que mandemos, indicando que debe ser una especificación técnica que tiene como objetivo un programado que va a implementar la aplicación.

En github copilot con el modelo GPT-5 (codex) en modo agente:

>PROMPT: Ejecuta las tareas indicadas en el documento siguiente:  
`prompts/genera_specs_legacy_busqueda_vendedores.md`

### 6.- Revisar el archivo de especificaciones generado

Ejemplo revisar
`spec-legacy-busqueda-vendedores.md`

### 7.- Revisión de prompt de migración
Crear un prompt para detallar los comandos de migración de la función legada que se va a migrar. Se puede basar en una plantilla de un módulo con funcionalidad similar.

 Se edita el documento y se cambia lo que es diferente para el módulo
que pensamos migrar:  
`prompts/migrar_busqueda_vendedores.md`

### 8.- Revisión del prompt de migración con Copilot

 En github copilot con el modelo GPT-5 (codex) en modo agente:

>PROMPT: Revisa el siguiente documento y corrígelo interactuando conmigo sobre los hallazgos para hacer el documento lo más preciso y completo posible, también evalúa que no haya archivos faltantes o ambigüedad: `prompts/migrar_busqueda_vendedores.md`

### 9.- Ejecutar la migración
Decirle a Copilot que implemente la migración indicada en el documento correspondiente:

En github copilot con el modelo CLAUDE SONNET 4.5 en modo agente:

>PROMPT: Ejecuta las tareas indicadas en el documento siguiente:  
`prompts/migrar_busqueda_vendedores.md`

- Darle seguimiento a lo que va haciendo, pararlo si es necesario
cuando este haciendo cosas incorrectas, aclararle y continuar.

### 10.- Pruebas de la funcionalidad
Probar la funcionalidad y revisar el código y lo que va generando, e iterar en modificar lo necesario hasta completar la funcionalidad tal y como la queremos.----------------------------------------------------------------------

## Tips para desarrollar con Github Copilot
- Darle keep en cada logro y comitear. Revisar lo que se comitea, para
  detectar que la IA no haya modificado cosas fuera del alcance de su
  tarea.

- Usar los checkpoints cuando el prompt o lo que hizo copilot estuvo
  incorrecto.

- Agregar en un archivo `/copilot-rules.md` las indicaciones generales para el proyecto, tener ya reglas predeterminadas por ejemplo para scripts en python, indicaciones de nuestra plataforma de desarrollo, tipo de base de datos, etc.
- Interactuar con el agente, detenerlo, darle tips, preguntarle, precisar. etc.
- Hacer funcionalidades con alcance limitado, y hacer pruebas de uso en cada paso, para corregir lo que sea necesario y comitear cada vez que se logra el alcance.
- Cuando un motor en particular no logra completar una tarea, revisar el prompt, intentar simplificarla o usar otro motor. Chatgpt 5 y sonnet 4 han sido los que mejores resultados han dado
- Abrir nuevos chats para cada nueva tarea diferente, con el objetivo de reducir el contexto con el que tiene que trabajar el LLM.