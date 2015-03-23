
import subprocess
import unittest

from code.testbase import TestBase

class TestEchoSCP(TestBase):
    
    #################################################
    # Nominal test case: Execute Echo
    #################################################
    def test_run_echoscu(self):
        try:
            # Send ECHO request
            subprocess.check_output(["echoscu", "localhost", self._dopamine_port])
        except subprocess.CalledProcessError as error:
            self.assertEqual(error.returncode, 0)
            
if __name__ == '__main__':
    unittest.main()
