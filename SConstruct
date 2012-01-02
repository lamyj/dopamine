env = Environment()

env.SharedLibrary("research_pacs", ["database.cpp"],
    CPPPATH=["/usr/include/gdcm-2.0"],
    LIBS=["mongoclient", "boost_thread", "boost_filesystem", 
          "gdcmCommon", "gdcmIOD", "gdcmMSFF"])

env.Program("test_database", ["tests/code/test_database.cpp"],
    CPPPATH=[".", "/usr/include/gdcm-2.0"],
    LIBPATH=["."],
    LIBS=["research_pacs", "boost_unit_test_framework"])
