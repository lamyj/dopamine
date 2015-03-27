
import os
import subprocess
import unittest

from code.testbase import TestBase

class TestStoreSCP(TestBase):
    
    #################################################
    # Nominal test case: Execute Store
    #################################################
    def test_run_storescu(self):
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
    
    #################################################
    # Error test case: Not Allowed
    #################################################
    def test_store_not_allowed(self):
        # Remove authorization from database
        thread_spec = subprocess.Popen(self._create_specific_auth, shell=True)
        thread_spec.wait()
        
        try:
            johndoe = os.environ["DOPAMINE_TEST_JOHNDOE2"]
            
            # Send a Get request
            subproc = subprocess.Popen(["storescu", "-aet", "LOCAL", "-aec", 
                                        "REMOTE", "localhost", self._dopamine_port,
                                        johndoe],
                                        stderr=subprocess.PIPE, stdout=subprocess.PIPE)
            
            # Read the output
            out, err = subproc.communicate()
            
            # Check results
            self.assertEqual(out, "")
            #self.assertRegexpMatches(err, "Refused: OutOfResources")
            self.assertRegexpMatches(err, "\(0000,0901\) AT \(ffff,ffff\)")
            self.assertRegexpMatches(err, "\(0000,0902\) LO \[User not allowed to perform STORE\]")
            
        except subprocess.CalledProcessError as error:
            self.assertEqual(error.returncode, 0)
        
    #################################################
    # Error test case: No Authentication
    #################################################
    def test_store_not_authenticate(self):
        # Remove authorization from database
        thread_remove = subprocess.Popen(self._remove_authorization, shell=True)
        thread_remove.wait()
        
        try:
            johndoe = os.environ["DOPAMINE_TEST_JOHNDOE2"]
            
            # Send a Get request
            subproc = subprocess.Popen(["storescu", "-aet", "LOCAL", "-aec", 
                                        "REMOTE", "localhost", self._dopamine_port,
                                        johndoe],
                                        stderr=subprocess.PIPE, stdout=subprocess.PIPE)
            
            # Read the output
            out, err = subproc.communicate()
            
            # Check results
            self.assertEqual(out, "")
            #self.assertRegexpMatches(err, "Refused: OutOfResources")
            self.assertRegexpMatches(err, "\(0000,0901\) AT \(ffff,ffff\)")
            self.assertRegexpMatches(err, "\(0000,0902\) LO \[User not allowed to perform STORE\]")
            
        except subprocess.CalledProcessError as error:
            self.assertEqual(error.returncode, 0)
            
if __name__ == '__main__':
    unittest.main()

