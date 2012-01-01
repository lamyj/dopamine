import os
import shutil
import tarfile
import tempfile
import unittest

import medipy.io.dicom
from pymongo import Connection

from database import Database

class TestDatabase(unittest.TestCase) :
    
    def __init__(self, *args, **kwargs) :
        unittest.TestCase.__init__(self, *args, **kwargs)
        
        self.data_directory = os.path.abspath(os.path.join(
            os.path.dirname(__file__), "..", "data",))
        
        self._dicom_directory = tempfile.mkdtemp()
        archive = tarfile.open(
            os.path.join(self.data_directory, "brainix.tgz"), "r")
        archive.extractall(self._dicom_directory)
    
    def __del__(self) :
        shutil.rmtree(self._dicom_directory)
        
        # unittest.TestCase has not __del__ member
    
    def setUp(self) :
        self._temporary_directory = tempfile.mkdtemp()
        self._db_name = os.path.split(self._temporary_directory)[-1]
        self._database = Database(self._db_name, self._temporary_directory)
    
    def tearDown(self) :
        connection = Connection()
        connection.drop_database(self._db_name)
        shutil.rmtree(self._temporary_directory)
    
    def test_user(self) :
        radiologist = { "id" : "radiologist", "name" : "Ronald Radiologist" }
        self._database.insert_user(radiologist)
        
        users = list(self._database.query({}, collection="users"))
        self.assertEqual(len(users), 1)
        self.assertEqual(users[0]["id"], "radiologist")
        self.assertEqual(users[0]["name"], "Ronald Radiologist")
        
        # Cannot insert same user twice
        self.assertRaises(Exception, self._database.insert_user, radiologist)
    
    def test_protocol(self) :
        sponsor = { "id" : "bpc", "name" : "Big Pharmaceutical Company" }
        protocol = { "id" : "6dfd7305-10ac-4c90-8c05-e48f2f2fd88d",
                     "name" : "Foobaril, phase 2",
                     "sponsor" : "bpc"}
        self._database.insert_user(sponsor)
        self._database.insert_protocol(protocol)
        
        protocols = list(self._database.query({}, collection="protocols"))
        self.assertEqual(len(protocols), 1)
        self.assertEqual(protocols[0]["id"], "6dfd7305-10ac-4c90-8c05-e48f2f2fd88d")
        self.assertEqual(protocols[0]["name"], "Foobaril, phase 2")
        self.assertEqual(protocols[0]["sponsor"], "bpc")
        
        # Cannot add protocol with unknown promoter
        protocol["sponsor"] = "unknown"
        self.assertRaises(Exception, self._database.insert_protocol, protocol)
    
    def test_dataset(self) :
        sponsor = { "id" : "bpc", "name" : "Big Pharmaceutical Company" }
        protocol = { "id" : "6dfd7305-10ac-4c90-8c05-e48f2f2fd88d",
                     "name" : "Foobaril, phase 2",
                     "sponsor" : "bpc"}
        self._database.insert_user(sponsor)
        self._database.insert_protocol(protocol)
        
        dataset = medipy.io.dicom.parse(
            os.path.join(self._dicom_directory, "BRAINIX/2182114/801/00070001"))
        
        self.assertEqual(dataset.patients_name, "BRAINIX")
        self._database.de_identify(dataset)
        self._database.set_clinical_trial_informations(dataset, 
            "bpc", "6dfd7305-10ac-4c90-8c05-e48f2f2fd88d", "Sim^Ho")
        self.assertFalse("patients_name" in dataset)
        self.assertEqual(dataset.clinical_trial_sponsor_name, "bpc")
        self.assertEqual(dataset.clinical_trial_protocol_id, 
                         "6dfd7305-10ac-4c90-8c05-e48f2f2fd88d")
        self.assertEqual(dataset.clinical_trial_subject_id, "Sim^Ho")
        
        self._database.insert_dataset(dataset)
        
        documents = list(self._database.query({},  ["(0008,103e)"]))
        self.assertEqual(len(documents), 1)
        self.assertEqual(set(documents[0].keys()), set(["_id", "(0008,103e)"]))
        self.assertEqual(documents[0]["(0008,103e)"], "T1/3D/FFE/C")

if __name__ == '__main__':
    unittest.main()
