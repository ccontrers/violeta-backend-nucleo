# Instalación de plataforma de desarrollo. Plataforma Node.js (fmt, node 22). Debezium/Kafka, Mariadb 11.7, ejemplo de replicación. VS Code con github copilot. Lovable.

## Prerrequisitos:

**Plataforma de desarrollo legada:**

VS Code con extensión Github Copilot.

MariaDB 11.2.2 con una BD cargada

Embarcadero Rad Studio 10.2 para compilar el software legado.

Referencias:

> C:\\ PlataformaVioleta2023\\Guia plataforma 2023.txt
>
> C:\\VioletaGithub\\violeta2\\Documentos\\Manuales en
> revision\\Instalacion herremientas desarollo violeta.docx

Otras herramientas útiles:

**React Developer Tools**

<https://react.dev/learn/react-developer-tools>

The Best React Helper Tool - React Dev Tools Tutorial

<https://www.youtube.com/watch?v=QbSXXXEGA70>

------------------------------------------------------------------------

## Instalación de Node.js

Instalar fnm (Fast Node Manager) en powershell como administrador

Nota: Se recomienda desinstalar los node.js instalados fuera de fnm
(opcional)

winget install \--force Schniz.fnm

Reiniciar power Shell y probar con:

> fnm \--version

referencia: <https://github.com/Schniz/fnm>

Verificar la política de ejecución actual de powershell

Get-ExecutionPolicy -List

Si todas esta en undefined entonces permitir scripts locales al usuario
actual (powershell como administrador)

Set-ExecutionPolicy -Scope CurrentUser -ExecutionPolicy RemoteSigned

Editar el profile de powershell

Invoke-Item \$profile

Si lo anterior falla por no existir el profile crearlo con lo siguiente
y luego reintentar el invoke:

New-Item -ItemType File -Path \$profile -Force

Agregar lo siguiente al ginal del profile de ps

fnm env \--use-on-cd \--shell powershell \| Out-String \|
Invoke-Expression

reiniciar el powershell

Instalar la versión requerida de Node.js y ponerla como default.

fnm install 22.21.1

fnm list

fnm use 22

node --version

Referencia fnm y node:

<https://dev.to/astrobotme/mastering-nodejs-version-management-with-fast-node-manager-fnm-17ji>

------------------------------------------------------------------------

## 

## Configuración de un nuevo frontend en Lovable para ejecutarlo localmente.

(Como base para un nuevo proyecto de micro frontend)

1.- Crear un nuevo proyecto en lovable con el prompt:

Crea un proyecto en blanco.

2.- Renombrar el proyecto al nombre que se quiera con base a una
convención.

3.- Conectar el proyecto lovable a github, lo cual crea un proyecto
github con nombre \<nom_proyecto\>\_seeding.

4.- Clonar el proyecto localmente

5.- Abrir power shell y  Cambiarse a la carpeta local del proyecto
clonado

Probado con version: v22.21.1 de node

Verificar la versión de node.

node \--version

Instalar el entorno de node

npm i

Ejecutarlo

npm run dev

Acceder a la app por ejemplo:

<http://localhost:8080/>

Agregar temas (hojas de estilo).

Agregar login.

Agregar ejemplos de plantillas para ciertos módulos.

Menú.

------------------------------------------------------------------------

## Configuración de un nuevo microservicio, como un nuevo proyecto Spring boot

**Pre-requisito**

JDK 21

OpenJDK21U-jdk_x64_windows_hotspot_21.0.8_9.msi

Visual studio code

Visual studio code con las siguientes extensiones:

Extension Pack for java (microsoft)

Spring boot extension pack (vmware)

Gradle for Java (Microsoft)

Github Copilot extensión (github)

Crea un proyecto con el spring boot initializer

Spring Boot 3.5.7, Java 21, empaquetado Jar.

Dependencias:

> Spring Web
>
> Spring Data JPA
>
> JDBC API
>
> Validation
>
> Spring Security
>
> Lombok
>
> Spring Boot DevTools
>
> MariaDB Driver

![Interfaz de usuario gráfica, Texto, Aplicación, Correo electrónico El
contenido generado por IA puede ser
incorrecto.](./media/image1.png){width="7.5in"
height="4.152777777777778in"}

Después de generar el proyecto agrega manualmente en build.gradle:

> implementation \'io.jsonwebtoken:jjwt-api:0.12.6\'

runtimeOnly \'io.jsonwebtoken:jjwt-impl:0.12.6\'

runtimeOnly \'io.jsonwebtoken:jjwt-jackson:0.12.6\'

testImplementation \'com.h2database:h2\'

testRuntimeOnly \'org.junit.platform:junit-platform-launcher\'

Estructura de paquetes a crear para las API Rest:

> config (configuraciones generales)
>
> constant (constantes del dominio)
>
> controller (REST controllers)
>
> dto (request/response DTOs)
>
> entity (entidades JPA)
>
> exception (manejo centralizado de errores)
>
> repository (interfaces JpaRepository/JdbcClient)
>
> security (clases JWT: JwtAuthenticationFilter, JwtTokenProvider,
> configuración de seguridad)
>
> service (interfaces e implementaciones de negocio)
>
> VioletaserverApplication en la raíz

**Recursos adicionales:**

perfiles application-dev.properties, application-test.properties,
application-prod.properties con la configuración de puertos, datasource
(MariaDB en dev/prod, H2 en test), seguridad básica y logging.

activar/desactivar Flyway según el perfil (spring.flyway.enabled=false
en dev).

Configuración de logging (trace para JDBC en dev).

Propiedades de seguridad (intentos login, sesión, etc.).

Con todo lo anterior, el proyecto nuevo quedará listo para replicar el
funcionamiento del Spring Boot actual.

## 

## Guía para configurar Docker + WSL 2 en Windows 10+ pro, indicando desde qué entorno ejecutar cada comando (PowerShell o WSL).

**1. Preparación inicial**

Abre **PowerShell como administrador** (puedes buscar "PowerShell", clic
derecho → "Ejecutar como administrador").

**Habilita las características necesarias:**

Ejecuta una por una:

dism.exe /online /enable-feature
/featurename:Microsoft-Windows-Subsystem-Linux /all /norestart

dism.exe /online /enable-feature /featurename:VirtualMachinePlatform
/all /norestart

dism.exe /online /enable-feature /featurename:Microsoft-Hyper-V-All /all
/norestart

Estas tres activan WSL, la plataforma de virtualización de WSL 2 y
Hyper-V (que usa tanto WSL 2 como Docker).

Reinicia tu equipo cuando termine.

**2. Instalar y configurar WSL 2**

**Después de reiniciar, en PowerShell (normal, no hace falta admin):**

wsl \--install

Esto instala **la última versión de WSL 2** y una distro Linux (por
defecto Ubuntu). Si ya tenías WSL, asegúrate de que use versión 2:

wsl \--set-default-version 2

Y si tienes varias distros, puedes listar y convertirlas:

wsl \--list \--verbose

wsl \--set-version Ubuntu 2

**3. Instalar Docker Desktop**

1.  Descarga desde la versión de para Windows AMD64
    <https://www.docker.com/products/docker-desktop/>

2.  Durante la instalación:

    - Marca **"Use the WSL 2 based engine"**

    - No selecciones el backend de Hyper-V.

3.  Reinicia si te lo pide.

**4. Verificar Docker en WSL**

Abre tu **distro Linux (Ubuntu)** desde el menú inicio o escribiendo wsl
en PowerShell.

Luego, dentro del entorno Linux:

docker version

Deberías ver algo como:

Client: Docker Engine - Community

Server: Docker Engine - Community

Si todo está correcto, puedes probar:

docker run hello-world

------------------------------------------------------------------------

## Debezium/Kafka (en contenedores) replicando con JDBC sync entre bases de de datos Mariadb (fuera de contenedores)

**En wsl 2 instalar jq y curl:**

sudo apt-get install jq curl

**Copiar el contenido de la carpeta "debezium-mariadb" al home del
usuario en WSL2**

En \\\\wsl.localhost\\Ubuntu\\home\\desarrollo\\debezium-mariadb

**Configuración de MariaDB 11.7 para trabajar con debezium.**

**Instalar mariadb 11.7 en windows 11** como **nueva instancia**, es
necesario porque esta versión sí es compatible para ser origen de
debezium 3.3

mariadb-11.7.2-winx64.msi

En my.ini del origen (11.7)

> **Habilitar binary logging (requerido para CDC)**
>
> log-bin=mysql-bin binlog-format=ROW
>
> binlog-row-image=FULL
>
> **Server ID único (cambia por un número único en tu red)**
>
> server-id=184054

\-- Verificar que el binlog esté habilitado

SHOW VARIABLES LIKE \'log_bin\';

SHOW VARIABLES LIKE \'binlog_format\';

------------------------------------------------------------------------

**Usuario para el mariadb de origen 11.7 (conector MariaDB de origen):**

\-- Crear usuario para Debezium

CREATE USER \'debezium\'@\'%\' IDENTIFIED BY \'dbzThor917512\';

\-- Otorgar permisos necesarios

GRANT SELECT, RELOAD, SHOW DATABASES, REPLICATION SLAVE, REPLICATION
CLIENT, BINLOG ADMIN ON *\*.\** TO \'debezium\'@\'%\';

\-- Aplicar cambios

FLUSH PRIVILEGES;

------------------------------------------------------------------------

**Usuario para el mariadb de destino Maridb 11.2 (conector Sink):**

\-- Crear usuario para Debezium

CREATE USER \'debeziumsink\'@\'%\' IDENTIFIED BY \'dbzThor917512\';

\-- Otorgar permisos necesarios

GRANT SELECT, CREATE, ALTER, INSERT, UPDATE, DELETE ON *\*.\** TO
\'debeziumsink\'@\'%\';

\-- Aplicar cambios

FLUSH PRIVILEGES;

------------------------------------------------------------------------

**Bases de datos para mariadb de destino 11.2**

Crear bases de datos en mariadb 11.2 de destino:

prueba_sink_cache

prueba_sink

Cargar una base de datos de prueba para prueba_sink

prueba_sink_cache.sql

Cargar una base de datos de prueba para prueba_sink_cache

prueba_sink_mono_cache.sql

------------------------------------------------------------------------

Abrir el Docker compose en Windows.

WSL 2 (ubuntu)

export DEBEZIUM_VERSION=3.3.1

cd debezium_mariadb

docker compose -f docker-compose-mariadb-externa.yaml up -d

sudo chmod +x register-mariadb-connector.sh

./[register-mariadb-connector.sh](https://register-mariadb-connector.sh)
\"mariadb-external.json\" \"<http://localhost:8083>\"

./[register-mariadb-connector.sh](https://register-mariadb-connector.sh)
\"mariadb-sink-3308-cache.json\" \"<http://localhost:8083>\"

./[register-mariadb-connector.sh](https://register-mariadb-connector.sh)
\"mariadb-sink-3308-transforma.json\" \"<http://localhost:8083>\"

Kafka UI

<http://192.168.56.1:8080>

------------------------------------------------------------------------

**Notas: **

1.- Orden correcto para nuevas tablas esta bien el siguiente: se crea la
tabla en origen y en destino, luego se reinicia primero el conector
mariadb y luego solo el conector jdbc sync.

2.- No replicar tablas que no tienen llave primaria, si es indispensable
que se repliquen, primero agregar una llave primaria.

3.- Considerar las llaves foraneas, que se creen primero las tablas
padres.

4.- Se podría transformar el nombre de las tablas de destino de acuerdo
a su origen, para indicar que son de solo lectura y no deben
modificarse, con el siguiente en la configuracion de JDBC sink:

\"transforms.route.replacement\": \"rrhh\_\$1_cache\"

Por ejemplo en la tabla marcas, trabajaría en el destino con la tabla:
ms1_marcas_cache

\<origen\>\_\<nombre_entidad\>\_cache

mono_marcas_cache

rrhh_asistencias_cache

5.- Usar un conector JDBC sink para cada BD origen a otra BD destino.

------------------------------------------------------------------------

**Referencias:**

La replicación se hizo con base en

<https://github.com/debezium/debezium-examples/tree/main/unwrap-smt#jdbc-sink>

Kafka-ui

<https://github.com/provectus/kafka-ui>

tutorial de debezium

<https://debezium.io/documentation/reference/stable/tutorial.html>

Conector de Mariadb para debezium

<https://debezium.io/documentation/reference/stable/connectors/mariadb.html>
