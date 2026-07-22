#!/usr/bin/env python3
"""
wyrpkg - Package Manager for Wyrm (.wyr)
"""

import sys
import os
import shutil
import json
from pathlib import Path

WYRPKG_VERSION = "1.2"

# Use global registry at ~/.wyrm/packages (installed mode)
# Fall back to cwd/packages when running from the source tree (dev mode)
_dev_mode = (Path(__file__).parent.parent / "wyrm").is_dir() and (Path(__file__).parent.parent / "setup.py").is_file()
if _dev_mode:
    _registry_root = Path(__file__).parent.parent
else:
    _registry_root = Path.home() / ".wyrm"

PACKAGES_DIR = _registry_root / "packages"
LOCK_FILE = _registry_root / "wyrpkg.lock"

def show_version():
    print(f"wyrpkg version {WYRPKG_VERSION}")

def show_help():
    print(f"""wyrpkg v{WYRPKG_VERSION} - Wyrm Package Manager

Usage:
  wyrpkg <command> [options]

Commands:
  install <package>   Install a package from local registry or directory
  remove <package>    Remove an installed package
  list                List installed packages
  version, --version  Show version
  help, --help        Show help message
""")

def load_lock():
    if LOCK_FILE.exists():
        try:
            return json.loads(LOCK_FILE.read_text(encoding="utf-8"))
        except Exception:
            return {}
    return {}

def save_lock(data):
    LOCK_FILE.write_text(json.dumps(data, indent=2, ensure_ascii=False), encoding="utf-8")

def install(pkg_name):
    PACKAGES_DIR.mkdir(exist_ok=True)
    target_dir = PACKAGES_DIR / pkg_name
    if target_dir.exists():
        print(f"Package '{pkg_name}' is already installed.")
        return
    
    # Check if pkg_name is a local path
    src_path = Path(pkg_name)
    if src_path.exists():
        pkg_real_name = src_path.name
        target_dir = PACKAGES_DIR / pkg_real_name
        if src_path.is_dir():
            shutil.copytree(src_path, target_dir)
        else:
            target_dir.mkdir(exist_ok=True)
            shutil.copy2(src_path, target_dir / src_path.name)
        
        lock = load_lock()
        lock[pkg_real_name] = {"source": str(src_path.resolve()), "status": "installed"}
        save_lock(lock)
        print(f"Installed package '{pkg_real_name}' from {src_path}")
    else:
        # Create virtual placeholder package
        target_dir.mkdir(exist_ok=True)
        init_wyr = target_dir / "mod.wyr"
        init_wyr.write_text(f"// Package {pkg_name}\nfn info() {{\n    print(\"Package {pkg_name}\")\n}}\n", encoding="utf-8")
        
        lock = load_lock()
        lock[pkg_name] = {"source": "virtual", "status": "installed"}
        save_lock(lock)
        print(f"Installed package '{pkg_name}' successfully.")

def remove(pkg_name):
    target_dir = PACKAGES_DIR / pkg_name
    if not target_dir.exists():
        print(f"Package '{pkg_name}' is not installed.")
        return
    shutil.rmtree(target_dir)
    lock = load_lock()
    if pkg_name in lock:
        del lock[pkg_name]
        save_lock(lock)
    print(f"Removed package '{pkg_name}'.")

def list_packages():
    lock = load_lock()
    if not lock:
        print("No packages installed.")
        return
    print("Installed packages:")
    for name, info in lock.items():
        print(f" - {name} ({info.get('source', 'local')})")

def main():
    if len(sys.argv) < 2:
        show_help()
        return

    cmd = sys.argv[1]
    if cmd in ("--version", "-v", "version"):
        show_version()
    elif cmd in ("--help", "-h", "help"):
        show_help()
    elif cmd == "install":
        if len(sys.argv) < 3:
            print("Error: Please specify package name or path to install.")
            sys.exit(1)
        install(sys.argv[2])
    elif cmd in ("remove", "uninstall"):
        if len(sys.argv) < 3:
            print("Error: Please specify package name to remove.")
            sys.exit(1)
        remove(sys.argv[2])
    elif cmd == "list":
        list_packages()
    else:
        print(f"Unknown command '{cmd}'. Run 'wyrpkg --help' for usage.")
        sys.exit(1)

if __name__ == "__main__":
    main()
