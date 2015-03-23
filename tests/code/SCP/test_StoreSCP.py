
import os
import subprocess
import unittest

from code.testbase import TestBase

class TestStoreSCP(TestBase):
    
    #################################################
    # Nominal test case: Execute Store
    #################################################
    def test_run_getscu(self):
        try:
            johndoe = os.environ["DOPAMINE_TEST_JOHNDOE"]
            
            # Send a Get request
            subproc = subprocess.Popen(["storescu", "-aet", "LOCAL", "-aec", 
                                        "REMOTE", "localhost", self._dopamine_port,
                                        johndoe],
                                        stderr=subprocess.PIPE, stdout=subprocess.PIPE)
            
            # Read the output
            out, err = subproc.communicate()
            
            # Check results
            self.assertEqual(out, "")
            self.assertEqual(err, "")
            
        except subprocess.CalledProcessError as error:
            self.assertEqual(error.returncode, 0)
            
if __name__ == '__main__':
    unittest.main()

