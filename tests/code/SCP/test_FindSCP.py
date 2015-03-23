
import subprocess
import unittest

from code.testbase import TestBase

class TestFindSCP(TestBase):
    
    #################################################
    # Nominal test case: Execute Find
    #################################################
    def test_run_findscu(self):
        try:
            # Send a Find request
            subproc = subprocess.Popen(["findscu", "-aet", "LOCAL", "-aec", 
                                        "REMOTE", "-P", "-k", "0010,0010=Doe^Jane", 
                                        "-k", " 0008,0052=PATIENT", "localhost", 
                                        self._dopamine_port],
                                        stderr=subprocess.PIPE, stdout=subprocess.PIPE)
            
            # Read the output
            out, err = subproc.communicate()
            
            # Check results
            self.assertEqual(out, "")
            self.assertRegexpMatches(err, "Find Response: 1")
            self.assertRegexpMatches(err, "\(0010,0010\) PN \[Doe\^Jane\]")
            self.assertRegexpMatches(err, "\(0010,0020\) LO \[dopamine_test_01\]")
            
        except subprocess.CalledProcessError as error:
            self.assertEqual(error.returncode, 0)
        
    #################################################
    # Error test case: No QueryRetrieveLevel
    #################################################
    def test_find_no_queryretrievelevel(self):
        try:
            # Send a Find request
            subproc = subprocess.Popen(["findscu", "-aet", "LOCAL", "-aec", 
                                        "REMOTE", "-P", "-k", "0010,0010=Doe^Jane", 
                                        "localhost", self._dopamine_port],
                                        stderr=subprocess.PIPE, stdout=subprocess.PIPE)
            
            # Read the output
            out, err = subproc.communicate()
            
            # Check results
            self.assertEqual(out, "")
            self.assertRegexpMatches(err, "Find Response: Failed: IdentifierDoesNotMatchSOPClass")
            self.assertRegexpMatches(err, "\(0000,0901\) AT \(0008,0052\)")
            self.assertRegexpMatches(err, "\(0000,0902\) LO \[Tag not found \]")
            
        except subprocess.CalledProcessError as error:
            self.assertEqual(error.returncode, 0)
            
if __name__ == '__main__':
    unittest.main()
