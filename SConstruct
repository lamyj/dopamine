import os

env = Environment()
env.AppendUnique(CPPFLAGS=os.environ.get("CPPFLAGS", ""))
env.AppendUnique(CPPPATH=os.environ.get("CPPPATH", "").split(os.pathsep))
env.AppendUnique(LIBPATH=os.environ.get("LIBPATH", "").split(os.pathsep))

env.SharedLibrary("research_pacs", ["dicom_to_cpp.cpp", "database.cpp"],
    CPPFLAGS=env["CPPFLAGS"],
    CPPPATH=env["CPPPATH"],
    LIBS=["mongoclient", "boost_thread", "boost_filesystem", 
          "gdcmCommon", "gdcmIOD", "gdcmMSFF"])

env.Program("test_database", ["tests/code/test_database.cpp"],
    CPPFLAGS=env["CPPFLAGS"],
    CPPPATH=env["CPPPATH"]+["."],
    LIBPATH=env["LIBPATH"]+["."],
    LIBS=["research_pacs", "boost_unit_test_framework"])
