 #!/usr/bin/env python
import os

from SCons.Script import (  # pylint: disable=no-name-in-module
    Variables,
    SConscript,
    Environment,
    ARGUMENTS,
    PathVariable,
    BoolVariable,
    Glob,
    Default,
    Help,
    Copy,
)

from SCons.Errors import UserError


def normalize_path(val, _env):
    return val if os.path.isabs(val) else os.path.join(_env.Dir("#").abspath, val)


def validate_parent_dir(key, val, _env):
    if not os.path.isdir(normalize_path(os.path.dirname(val), _env)):
        dir_name = os.path.dirname(val)
        raise UserError(f"'{key}' is not a directory: {dir_name}")


libname = "libmaszyna"
projectdir = "demo"

localEnv = Environment(tools=["default"], PLATFORM="")

customs = ["custom.py"]
customs = [os.path.abspath(path) for path in customs]

opts = Variables(customs, ARGUMENTS)
opts.Add("maszyna_debug", help="Enable internal debug mode", default="yes")
opts.Add(
    BoolVariable(
        key="compiledb",
        help="Generate compilation DB (`compile_commands.json`) for external tools",
        default=localEnv.get("compiledb", False),
    )
)
opts.Add(
    PathVariable(
        key="compiledb_file",
        help="Path to a custom `compile_commands.json` file",
        default=localEnv.get("compiledb_file", "compile_commands.json"),
        validator=validate_parent_dir,
    )
)
opts.Update(localEnv)

Help(opts.GenerateHelpText(localEnv))

env = localEnv.Clone()
env["compiledb"] = False

env.Tool("compilation_db")
compilation_db = env.CompilationDatabase(
    normalize_path(localEnv["compiledb_file"], localEnv)
)
env.Alias("compiledb", compilation_db)

env = SConscript("godot-cpp/SConstruct", {"env": env, "customs": customs})

env.Append(CPPPATH=["src/"])
sources = (
    Glob("src/*.cpp")
    + Glob("src/maszyna/*.cpp")
    + Glob("src/maszyna/McZapkie/*.cpp")
    + Glob("src/switches/*.cpp")
    + Glob("src/core/*.cpp")
    + Glob("src/doors/*.cpp")
    + Glob("src/engines/*.cpp")
    + Glob("src/systems/*.cpp")
    + Glob("src/brakes/*.cpp")
    + Glob("src/types/*.cpp")
    + Glob("src/parsers/*.cpp")
    + Glob("src/lighting/*.cpp")
    + Glob("src/load/*.cpp")
    + Glob("src/resources/engines/*.cpp")
    + Glob("src/resources/lighting/*.cpp")
    + Glob("src/resources/load/*.cpp")
)

if env["target"] in ["editor", "template_debug"]:
    try:
        doc_data = env.GodotCPPDocData(
            "src/gen/doc_data.gen.cpp", source=Glob("doc_classes/*.xml")
        )
        sources.append(doc_data)
    except AttributeError:
        print("Not including class reference as we're targeting a pre-4.3 baseline.")


if env["maszyna_debug"].lower() == "yes":
    env.Append(CPPDEFINES=["LIBMASZYNA_DEBUG_ENABLED"])

if env["target"] in ("debug", "template_debug"):
    env.Append(CPPDEFINES=["DEBUG_MODE"])
else:
    env.Append(CPPDEFINES=["RELEASE_MODE"])

# Paths

suffix = ""
if env["maszyna_debug"].lower() == "yes":
    env.Append(CPPDEFINES=["LIBMASZYNA_DEBUG_ENABLED"])

if env["target"] in ("debug", "template_debug"):
    suffix += ".debug"
    env.Append(CPPDEFINES=["DEBUG_MODE"])
else:
    env.Append(CPPDEFINES=["RELEASE_MODE"])

if env["arch"] in ("x86_64"):
    suffix += ".64"
elif env["arch"] not in ("x86_32"):
    suffix += "." + env["arch"]
# Paths


def get_subdirs(abs_path_dir):
    subdirs = [
        name
        for name in os.listdir(abs_path_dir)
        if os.path.isdir(os.path.join(abs_path_dir, name)) and name[0] != "."
    ]
    subdirs.sort()
    return subdirs


shlib_suffix = env["SHLIBSUFFIX"]
file = f"{libname}{suffix}{shlib_suffix}"
platform = env["platform"]
target_bin_path = os.path.join("bin", platform, file)
addons_src_path = os.path.join("addons", libname)
addons_dst_path = os.path.join("demo", "addons", libname)
# Build

commands = []

if os.path.exists("src") and sources:
    library = env.SharedLibrary(target_bin_path, source=sources)

    copy_bin_to_project = env.InstallAs(
        os.path.join(projectdir, "bin", libname, platform, file), source=library
    )

    copy_gut_framework_to_project = env.Command(
        os.path.join(projectdir, "addons", "gut"),
        os.path.join("vendor", "gut", "addons", "gut"),
        Copy("$TARGET", "$SOURCE"),
    )

    commands += [
        library,
        copy_bin_to_project,
        copy_gut_framework_to_project,
    ]

    if not os.path.exists(addons_dst_path):
        os.makedirs(addons_dst_path, exist_ok=True)

    if os.path.islink(addons_dst_path):
        print(f"[!] Symlink detected at {addons_dst_path}. Skipping Install().")
    else:
        addon_files = Glob(os.path.join(addons_src_path, "*"))

        # FIXME: I'm not sure about cleaning the addons_dst_path (shutil.rmtree)
        #
        # If enabled, it could break the running Godot Editor
        # during (re)build of the scons project.
        #
        # If commented out, the env.Install() will not remove files deleted
        # in addons_src_path. The addons_dst_path may differ.
        #
        # Maybe there is a better approach: to find removed files in addons/ and
        # remove them one by one from addons_dst_path? To complicated for me.
        #
        # shutil.rmtree(addons_dst_path)

        commands.append(env.Install(addons_dst_path, source=addon_files))

        for addon_subdir in get_subdirs(addons_src_path):
            subdir_dst_path = os.path.join(addons_dst_path, addon_subdir)
            commands.append(
                env.Install(
                    source=Glob(os.path.join(addons_src_path, addon_subdir, "*")),
                )
            )

        print(f"✅ Files will be installed from {addons_src_path} to {addons_dst_path}")


if localEnv.get("compiledb", False):
    commands.append(compilation_db)

Default(*commands)
