import os

env = Environment()
env.AppendUnique(CPPFLAGS=os.environ.get("CPPFLAGS", ""))
env.AppendUnique(CPPPATH=os.environ.get("CPPPATH", "").split(os.pathsep))
env.AppendUnique(LIBPATH=os.environ.get("LIBPATH", "").split(os.pathsep))

Export("env")

SConscript("lib/SConstruct")
SConscript("cli/SConstruct")
SConscript("tests/code/SConstruct")
