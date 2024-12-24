"""
TcMenu Automated Code Generator for PlatformIO
==============================================

This script automates TcMenu code generation in a PlatformIO project. It is automatically run before each build,
checking if the `.emf` file has changed and only regenerating code when necessary.

Available Options (platformio.ini)
----------------------------------
- **tcmenu_disable_generator**: (boolean/string, optional)
  If set to `true` (or `1`, `yes`), the script is disabled entirely. No generation occurs.
  Example:
      tcmenu_disable_generator = true

- **tcmenu_force_generation**: (boolean/string, optional)
  If set to `true` (or `1`, `yes`), the script always regenerates the code regardless of the file’s hash.
  Example:
      tcmenu_force_generation = true

- **tcmenu_generator_path**: (string, optional)
  Path to the TcMenu Designer generator executable. Example:
      tcmenu_generator_path = "C:/MyTools/TcMenuDesigner/tcmenu.exe"

- **tcmenu_project_file**: (string, optional)
  Custom path to the `.emf` (or project) file. Example:
      tcmenu_project_file = "/home/user/customMenus/myMenu.emf"

"""

import os
import platform
import pathlib
import subprocess
import hashlib

from platformio import fs
from SCons.Script import Import

Import("env")

def find_tcmenu_generator():
    """
    Determine the path to the TcMenu Designer generator executable based on:
      1) platformio.ini override (tcmenu_generator_path)
      2) host operating system defaults
    Return the executable path or None if not found.
    """
    custom_generator_path = env.GetProjectOption("tcmenu_generator_path", default=None)
    if custom_generator_path and os.path.isfile(custom_generator_path):
        return custom_generator_path

    system_name = platform.system().lower()
    if system_name.startswith("win"):
        default_path = "C:\\Program Files (x86)\\TcMenuDesigner\\tcmenu.exe"
    elif system_name.startswith("darwin"):
        # macOS
        default_path = "/Applications/tcMenuDesigner.app/Contents/MacOS/tcMenuDesigner/tcmenu"
    else:
        # Linux
        default_path = "/opt/tcmenudesigner/bin/tcMenuDesigner/tcmenu"

    return default_path if os.path.isfile(default_path) else None


def find_project_file():
    """
    Locate the .emf (or project) file in the project root or via user-specified path in platformio.ini:
      tcmenu_project_file=<path>
    """
    custom_emf = env.GetProjectOption("tcmenu_project_file", default=None)
    if custom_emf and os.path.isfile(custom_emf):
        return custom_emf

    project_dir = env.subst("$PROJECT_DIR")
    emf_candidates = fs.match_src_files(project_dir, "+<*.emf>")
    if emf_candidates:
        return os.path.join(project_dir, emf_candidates[0])
    return None


def compute_file_sha256(file_path):
    """
    Compute the SHA-256 hash of the given file.
    """
    with open(file_path, "rb") as f:
        data = f.read()
    return hashlib.sha256(data).hexdigest()


def generate_code(tcmenu_generator, project_file):
    """
    Run the TcMenu Designer command, generating code into .pio/build/<env>/tcmenu.
    """
    build_dir = env.subst("$BUILD_DIR")
    tcmenu_output_dir = os.path.join(build_dir, "tcmenu")

    os.makedirs(tcmenu_output_dir, exist_ok=True)

    old_cwd = os.getcwd()
    try:
        # Change directory to the output directory
        os.chdir(tcmenu_output_dir)

        cmd = [
            tcmenu_generator,
            "generate",
            "--emf-file",
            project_file
        ]
        print(f"[TcMenu] Generating code with command: {' '.join(cmd)}")

        result = subprocess.run(cmd, check=True, capture_output=True)
        stdout_str = result.stdout.decode("utf-8")
        if stdout_str.strip():
            print("[TcMenu] Output:\n", stdout_str)

    except subprocess.CalledProcessError as e:
        print(f"[TcMenu] Warning: TcMenu generation failed: {e}")
        print("[TcMenu] Continuing build anyway...")

    finally:
        # Always restore the original working directory
        os.chdir(old_cwd)

def remove_duplicates(tcmenu_output_dir):
    """
    Remove or skip duplicates if user code is in 'src/'.
    The user code always takes precedence over generated code.
    """
    project_src = os.path.join(env.subst("$PROJECT_DIR"), "src")
    if not os.path.isdir(tcmenu_output_dir) or not os.path.isdir(project_src):
        return

    for root, _, files in os.walk(tcmenu_output_dir):
        for f in files:
            generated_file = os.path.join(root, f)
            user_file = os.path.join(project_src, f)
            if os.path.isfile(user_file):
                print(f"[TcMenu] Skipping generated file because user code takes precedence: {generated_file}")
                # Optionally remove or rename the generated file here:
                # os.remove(generated_file)


def main():
    # Check if script is disabled
    disable_generator_str = env.GetProjectOption("tcmenu_disable_generator", default="false").lower()
    if disable_generator_str in ["true", "1", "yes"]:
        print("[TcMenu] Script is disabled via 'tcmenu_disable_generator'. Skipping code generation.")
        return

    print("[TcMenu] Starting code generation script (SHA-256 check).")

    # Locate the TcMenu generator executable
    tcmenu_generator = find_tcmenu_generator()
    if not tcmenu_generator:
        print("[TcMenu] WARNING: TcMenu generator not found. Code generation will be skipped.")
        return

    # Locate the project file (i.e., .emf)
    project_file = find_project_file()
    if not project_file:
        print("[TcMenu] WARNING: No project (.emf) file found. Code generation will be skipped.")
        return

    # Determine if we should force generation
    force_generation_str = env.GetProjectOption("tcmenu_force_generation", default="false").lower()
    force_generation = force_generation_str in ["true", "1", "yes"]

    # Compute SHA-256 of the project file
    project_sha = compute_file_sha256(project_file)

    build_dir = env.subst("$BUILD_DIR")
    tcmenu_output_dir = os.path.join(build_dir, "tcmenu")
    os.makedirs(tcmenu_output_dir, exist_ok=True)

    # Store the last known SHA-256 in a file
    sha_file_path = os.path.join(tcmenu_output_dir, "tcmenu.project.sha256")

    # Determine if we need to regenerate
    need_generate = True
    if not force_generation:
        try:
            last_sha = pathlib.Path(sha_file_path).read_text().strip()
            if last_sha == project_sha:
                need_generate = False
                print("[TcMenu] Skipping code generation: Project file unchanged.")
        except FileNotFoundError:
            pass

    if need_generate:
        generate_code(tcmenu_generator, project_file)
        # Write the new SHA-256
        pathlib.Path(sha_file_path).write_text(project_sha)
        # Remove duplicates (skip or remove existing user code)
        remove_duplicates(tcmenu_output_dir)
    else:
        # If skipping generation, still remove duplicates
        remove_duplicates(tcmenu_output_dir)

    print("[TcMenu] Finished code generation script.")


# Run the generator script
main()