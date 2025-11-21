import os
import sys
import ctypes
import subprocess


def resolve_env_path(args):
    """Return the env file path, allowing optional name without .env."""
    if not args:
        return ".env"

    candidate = args[0]

    if os.path.isdir(candidate):
        candidate = os.path.join(candidate, ".env")

    if not os.path.exists(candidate) and not candidate.endswith(".env"):
        alt_path = f"{candidate}.env"
        if os.path.exists(alt_path):
            candidate = alt_path

    return candidate


def running_as_admin():
    """Return True if the current process has admin privileges."""
    try:
        return bool(ctypes.windll.shell32.IsUserAnAdmin())
    except Exception:
        return False


ENV_PATH = resolve_env_path(sys.argv[1:])

if not os.path.exists(ENV_PATH):
    print(f"Archivo de variables no encontrado: {ENV_PATH}")
    exit(1)

has_admin_rights = running_as_admin()

with open(ENV_PATH, "r", encoding="utf-8") as f:
    for line in f:
        line = line.strip()

        # Ignorar comentarios y líneas vacías
        if not line or line.startswith("#"):
            continue

        # Parse KEY=VALUE
        if "=" not in line:
            continue

        key, value = line.split("=", 1)
        key = key.strip()
        value = value.strip()

        # Crear/actualizar variable del SISTEMA
        # "MACHINE" = variables del sistema
        # "USER" = variables del usuario
        ctypes.windll.kernel32.SetEnvironmentVariableW(key, value)

        command = ["setx", key, value]
        scope = "SYSTEM" if has_admin_rights else "USER"
        if has_admin_rights:
            command.append("/M")

        result = subprocess.run(command, capture_output=True, text=True, shell=False)

        if result.returncode != 0:
            print(f"No se pudo persistir {key} en {scope}. Detalle: {result.stderr.strip()}")
        else:
            print(f"Variable cargada ({scope}): {key}={value}")

# Avisar a Windows que las variables cambiaron
HWND_BROADCAST = 0xFFFF
WM_SETTINGCHANGE = 0x1A

ctypes.windll.user32.SendMessageTimeoutW(
    HWND_BROADCAST,
    WM_SETTINGCHANGE,
    0,
    "Environment",
    0x0002,
    5000,
    None
)

print("Cambios aplicados al sistema.")
