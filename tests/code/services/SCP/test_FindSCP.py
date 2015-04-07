
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
    # Nominal test case: Not Allowed
    #################################################
    def test_find_not_allowed(self):
        # Remove authorization from database
        thread_spec = subprocess.Popen(self._create_specific_auth, shell=True)
        thread_spec.wait()
        
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
            self.assertEqual(err, "")
            
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
            self.assertRegexpMatches(err, "\(0000,0901\) AT \(ffff,ffff\)")
            self.assertRegexpMatches(err, "\(0000,0902\) LO \[An error occured while processing Find operation\]")
            
        except subprocess.CalledProcessError as error:
            self.assertEqual(error.returncode, 0)
        
    #################################################
    # Error test case: No Authentication
    #################################################
    def test_find_not_authenticate(self):
        # Remove authorization from database
        thread_remove = subprocess.Popen(self._remove_authorization, shell=True)
        thread_remove.wait()
        
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
            self.assertRegexpMatches(err, "Refused: OutOfResources")
            self.assertRegexpMatches(err, "\(0000,0901\) AT \(ffff,ffff\)")
            self.assertRegexpMatches(err, "\(0000,0902\) LO \[An error occured while processing Find operation\]")
            
        except subprocess.CalledProcessError as error:
            self.assertEqual(error.returncode, 0)
            
if __name__ == '__main__':
    unittest.main()
