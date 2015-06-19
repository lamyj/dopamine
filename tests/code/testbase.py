
import os
import subprocess
import time
import unittest

class TestBase(unittest.TestCase) :
    
    def __init__(self, *args, **kwargs) :
        self._dopamine_build_dir = os.environ["DOPAMINE_BUILD_DIR"]
        self._dopamine_port = os.environ["DOPAMINE_TEST_LISTENINGPORT"]
        self._scu_port = os.environ["DOPAMINE_TEST_WRITINGPORT"]
        self._output_directory = os.environ["DOPAMINE_TEST_OUTPUTDIR"]
        self._create_authorization = os.environ["DOPAMINE_TEST_ADD_AUTH"]
        self._create_specific_auth = os.environ["DOPAMINE_TEST_SPE_AUTH"]
        self._remove_authorization = os.environ["DOPAMINE_TEST_DEL_AUTH"]
        self._add_doe_jane = os.environ["DOPAMINE_TEST_ADD_JANE"]
        self._del_doe_jane = os.environ["DOPAMINE_TEST_DEL_JANE"]
        unittest.TestCase.__init__(self, *args, **kwargs)
    
    def setUp(self) :
        # Add authorization in database
        subprocess.Popen(str(self._create_authorization), shell=True)
        # Add data
        subprocess.Popen(str(self._add_doe_jane), shell=True)
        # launch dopamine in a subprocess
        subprocess.Popen(os.path.join(str(self._dopamine_build_dir), "/src/appli/dopamine"))
        # wait for dopamine initialization
        time.sleep(1)
    
    def tearDown(self) :
        # Remove authorization from database
        subprocess.Popen(self._remove_authorization, shell=True)
        # Remove data
        subprocess.Popen(str(self._del_doe_jane), shell=True)
        # stop dopamine
        subprocess.Popen(["termscu localhost " + self._dopamine_port], shell=True)
        # wait for dopamine shutdown
        time.sleep(1)
    
if __name__ == '__main__':
    unittest.main()
