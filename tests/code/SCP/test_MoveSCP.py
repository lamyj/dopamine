
import subprocess
import unittest

from code.testbase import TestBase

class TestMoveSCP(TestBase):
    
    #################################################
    # Nominal test case: Execute Move
    #################################################
    def test_run_movescu(self):
        try:
            # Send a Move request
            subproc = subprocess.Popen(["movescu", "-aet", "LOCAL", "-aec", 
                                        "REMOTE", "-aem", "LOCAL", "+P", self._scu_port, 
                                        "-P", "-k", "0010,0010=Doe^Jane", 
                                        "-k", " 0008,0052=PATIENT", 
                                        "-od", self._output_directory,
                                        "localhost", self._dopamine_port],
                                        stderr=subprocess.PIPE, stdout=subprocess.PIPE)
            
            # Read the output
            out, err = subproc.communicate()
            
            # Check results
            self.assertEqual(out, "")
            self.assertEqual(err, "")
            
            #TODO check result file
            
        except subprocess.CalledProcessError as error:
            self.assertEqual(error.returncode, 0)
        
    #################################################
    # Error test case: No QueryRetrieveLevel
    #################################################
    def test_move_no_queryretrievelevel(self):
        try:
            # Send a Move request
            subproc = subprocess.Popen(["movescu", "-aet", "LOCAL", "-aec", 
                                        "REMOTE", "-aem", "LOCAL", "+P", self._scu_port, 
                                        "-P", "-k", "0010,0010=Doe^Jane", 
                                        "-od", self._output_directory,
                                        "localhost", self._dopamine_port],
                                        stderr=subprocess.PIPE, stdout=subprocess.PIPE)
            
            # Read the output
            out, err = subproc.communicate()
            
            # Check results
            self.assertEqual(out, "")
            self.assertRegexpMatches(err, "\(0000,0901\) AT \(0008,0052\)")
            self.assertRegexpMatches(err, "\(0000,0902\) LO \[Tag not found\]")
            
        except subprocess.CalledProcessError as error:
            self.assertEqual(error.returncode, 0)
            
if __name__ == '__main__':
    unittest.main()
