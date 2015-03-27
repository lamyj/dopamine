
import subprocess
import unittest

from code.testbase import TestBase

class TestEchoSCP(TestBase):
    
    #################################################
    # Nominal test case: Execute Echo (true)
    #################################################
    def test_run_echoscu(self):
        try:
            # Send ECHO request
            subprocess.check_output(["echoscu", "localhost", self._dopamine_port])
        except subprocess.CalledProcessError as error:
            self.assertEqual(error.returncode, 0)
            
    #################################################
    # Nominal test case: Execute Echo (false)
    #################################################
    # Warning: echoscu from DCMTK doesn't take care of Status return
    #          use another SCU if it's possible
    def test_run_echoscu_false(self):
        # Remove authorization from database
        thread_remove = subprocess.Popen(self._remove_authorization, shell=True)
        thread_remove.wait()
        
        try:
            # Send ECHO request
            subprocess.check_output(["echoscu", "localhost", self._dopamine_port])
        except subprocess.CalledProcessError as error:
            self.assertEqual(error.returncode, 0)
            
if __name__ == '__main__':
    unittest.main()
