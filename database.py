import logging
import os

import medipy.io.dicom
from pymongo import Connection

class Database(object) :
    def __init__(self, db_name, fs_db, *args, **kwargs) :
        self._connection = Connection(*args, **kwargs)
        self._mongo_db = self._connection[db_name]
        self._fs_db = fs_db
        if not os.path.isdir(self._fs_db) :
            os.makedirs(self._fs_db)
    
    def insert_user(self, user) :
        if self._mongo_db.users.find_one({"id" : user["id"]}) :
            raise Exception(
                "Cannot insert user \"{0}\" : already exists".format(
                user["id"]))
        
        self._mongo_db.users.insert(user)
        
    def insert_protocol(self, protocol) :
        if self._mongo_db.protocols.find_one({"id" : protocol["id"]}) :
            raise Exception(
                "Cannot insert protocol \"{0}\" : already exists".format(
                protocol["id"]))
                
        if not self._mongo_db.users.find_one({"id" : protocol["sponsor"]}) :
            raise Exception(
                "Cannot insert protocol \"{0}\" : no such sponsor : \"{1}\"".format(
                protocol["name"], protocol["sponsor"]))
        
        self._mongo_db.protocols.insert(protocol)
    
    def insert_file(self, filename) :
        raise NotImplementedError("insert_file is not implemented")
    
    def insert_dataset(self, dataset) :
        if self._mongo_db.documents.find_one({"_original_sop_instance_uid" : dataset.sop_instance_uid}) :
            raise Exception("Cannot insert dataset : already in database")
        
        document = {"_original_sop_instance_uid" : dataset.sop_instance_uid}
        for key, value in dataset.items() :
            if key.private :
                logging.debug("TODO : do we store private elements in database ?")
                continue
            
            if key in [0x7fe00010] :
                continue
            if key.group/256 == 0x60 and key.element == 0x3000 :
                # Overlay Data (60xx,3000)
                continue
            
            document[key] = value
        
        document = Database._to_document(document)
        self._mongo_db.documents.insert(document)
        
        # Store on file system : how to do this ?
        logging.debug("TODO : store to filesystem")
    
    def query(self, query, fields=None, collection="documents") :
        """ Query a collection from the database (defaults to the 
            "documents" collection). If the query dictionary contains
            keys that are names of DICOM elements, they are translated
            to their tag values.
            
            >>> db.query({"series_description":"T1 SE TRANS 44C"})
            >>> db.query({"(0008,103e)":"T1 SE TRANS 44C"})
        """
        
        modified_query = {}
        for key, value in query.items() :
            if key in medipy.io.dicom.dictionary.name_dictionary :
                modified_key = medipy.io.dicom.dictionary.name_dictionary[key]
                modified_key = str(medipy.io.dicom.Tag(modified_key))
            else :
                modified_key = key
            modified_query[modified_key] = value
        return self._mongo_db[collection].find(modified_query, fields)
    
    def remove(self, query) :
        raise NotImplementedError("remove is not implemented")
    
    def de_identify(self, dataset, can_reidentify=False) :
        """ Remove all DICOM elements that can identify a patient from the
            dataset. If can_reidentify is True, store the de-identification
            information for later re-identification.
        """
        
        if can_reidentify :
            raise NotImplementedError("Re-identification is not implemented")
        
        del dataset["patients_name"]
        logging.debug("TODO : establish a list of identification attributes")
    
    def set_clinical_trial_informations(self, dataset, sponsor, protocol, subject) :
        """ Set the Clinical Trial elements of the dataset while ensuring
            database coherence.
        """
        
        if not self._mongo_db.users.find_one({"id" : sponsor}) :
            raise Exception(
                "No such sponsor : \"{0}\"".format(sponsor))
        if not self._mongo_db.protocols.find_one({"id" : protocol}) :
            raise Exception(
                "No such protocol : \"{0}\"".format(protocol))
        
        dataset.clinical_trial_sponsor_name = sponsor
        dataset.clinical_trial_protocol_id = protocol
        dataset.clinical_trial_subject_id = subject
    
    @staticmethod
    def _to_document(element) :
        """ Transform an element of a dataset to be stored in the database.
        """
        
        if isinstance(element, (basestring, int, float, unicode)) :
            return element
        elif isinstance(element, list) :
            return [Database._to_document(x) for x in element]
        elif isinstance(element, (dict, medipy.io.dicom.DataSet)) :
            return dict(
                [(str(k), Database._to_document(v)) for k,v in element.items()])
        else :
            raise Exception("Unknown type : {0}".format(type(element)))
