db = connect("localhost:27017/${DOPAMINE_TEST_DATABASE}");
db.authorization.drop()

j = { "principal_name" : "", "principal_type" : "", "service" : "Query", "dataset" : { "00100010" : /^Not_Doe_Jane$/ } }
db.authorization.insert(j)

j = { "principal_name" : "", "principal_type" : "", "service" : "Retrieve", "dataset" : { "00100010" : /^Not_Doe_Jane$/ } }
db.authorization.insert(j)

j = { "principal_name" : "", "principal_type" : "", "service" : "Store", "dataset" : { "00100010" : /^Not_Doe_John$/ } }
db.authorization.insert(j)

