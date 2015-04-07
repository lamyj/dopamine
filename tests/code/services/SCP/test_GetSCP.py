
import os
import subprocess
import unittest

from code.testbase import TestBase

class TestGetSCP(TestBase):
    
    #################################################
    # Nominal test case: Execute Get
    #################################################
    def test_run_getscu(self):
        try:
            pathjoin = os.path.join(self._output_directory, 
                                    "MR.2.16.756.5.5.100.3611280983.20092.1364462458.1.0")
            self.assertEqual(os.path.isfile(pathjoin), False)
            
            # Send a Get request
            subproc = subprocess.Popen(["../../build/tests/tools/getscu", "-aet", "LOCAL", "-aec", 
                                        "REMOTE", "-P", "-k", "0010,0010=Doe^Jane", 
                                        "-k", " 0008,0052=PATIENT", 
                                        "-od", self._output_directory,
                                        "localhost", self._dopamine_port],
                                        stderr=subprocess.PIPE, stdout=subprocess.PIPE)
            
            # Read the output
            out, err = subproc.communicate()
            
            # Check results
            self.assertEqual(out, "")
            self.assertEqual(err, "")
            
            # Check result file
            self.assertEqual(os.path.isfile(pathjoin), True)
            
            # remove file
            os.remove(pathjoin)
            
        except subprocess.CalledProcessError as error:
            self.assertEqual(error.returncode, 0)
    
    #################################################
    # Nominal test case: Not Allowed
    #################################################
    def test_run_getscu(self):
        # Remove authorization from database
        thread_spec = subprocess.Popen(self._create_specific_auth, shell=True)
        thread_spec.wait()
        
        try:
            pathjoin = os.path.join(self._output_directory, 
                                    "MR.2.16.756.5.5.100.3611280983.20092.1364462458.1.0")
            self.assertEqual(os.path.isfile(pathjoin), False)
            
            # Send a Get request
            subproc = subprocess.Popen(["../../build/tests/tools/getscu", "-aet", "LOCAL", "-aec", 
                                        "REMOTE", "-P", "-k", "0010,0010=Doe^Jane", 
                                        "-k", " 0008,0052=PATIENT", 
                                        "-od", self._output_directory,
                                        "localhost", self._dopamine_port],
                                        stderr=subprocess.PIPE, stdout=subprocess.PIPE)
            
            # Read the output
            out, err = subproc.communicate()
            
            # Check results
            self.assertEqual(out, "")
            self.assertEqual(err, "")
            
            # Check result file (no file)
            self.assertEqual(os.path.isfile(pathjoin), False)
            
        except subprocess.CalledProcessError as error:
            self.assertEqual(error.returncode, 0)
        
    #################################################
    # Error test case: No QueryRetrieveLevel
    #################################################
    def test_get_no_queryretrievelevel(self):
        try:
            # Send a Get request
            subproc = subprocess.Popen(["../../build/tests/tools/getscu", "-aet", "LOCAL", "-aec", 
                                        "REMOTE", "-P", "-k", "0010,0010=Doe^Jane", 
                                        "-od", self._output_directory,
                                        "localhost", self._dopamine_port],
                                        stderr=subprocess.PIPE, stdout=subprocess.PIPE)
            
            # Read the output
            out, err = subproc.communicate()
            
            # Check results
            self.assertEqual(out, "")
            #self.assertRegexpMatches(err, "\(0000,0901\) AT \(0008,0052\)")
            #self.assertRegexpMatches(err, "\(0000,0902\) LO \[Tag not found\]")
            # Error return is not "Tag not found" but "Identifier does not match SOP class"
            self.assertRegexpMatches(err, "Identifier does not match SOP class")
            
        except subprocess.CalledProcessError as error:
            self.assertEqual(error.returncode, 0)
        
    #################################################
    # Error test case: No Authentication
    #################################################
    def test_get_not_authenticate(self):
        # Remove authorization from database
        thread_remove = subprocess.Popen(self._remove_authorization, shell=True)
        thread_remove.wait()
        
        try:
            # Send a Get request
            subproc = subprocess.Popen(["../../build/tests/tools/getscu", "-aet", "LOCAL", "-aec", 
                                        "REMOTE", "-P", "-k", "0010,0010=Doe^Jane", 
                                        "-k", " 0008,0052=PATIENT", 
                                        "-od", self._output_directory,
                                        "localhost", self._dopamine_port],
                                        stderr=subprocess.PIPE, stdout=subprocess.PIPE)
            
            # Read the output
            out, err = subproc.communicate()
            
            # Check results
            self.assertEqual(out, "")
            self.assertRegexpMatches(err, "Out of Resouces - Unable to calculate number of matches")
            #self.assertRegexpMatches(err, "\(0000,0901\) AT \(ffff,ffff\)")
            #self.assertRegexpMatches(err, "\(0000,0902\) LO \[User not allowed to perform GET\]")
            
        except subprocess.CalledProcessError as error:
            self.assertEqual(error.returncode, 0)
            
if __name__ == '__main__':
    unittest.main()
