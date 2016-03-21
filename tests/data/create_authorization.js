db = connect("localhost:27017/${DOPAMINE_TEST_DATABASE}");
j = { "principal_name" : "", "principal_type" : "", "service" : "Echo", "dataset" : {} }
db.authorization.insert(j)

j = { "principal_name" : "", "principal_type" : "", "service" : "Query", "dataset" : {} }
db.authorization.insert(j)

j = { "principal_name" : "", "principal_type" : "", "service" : "Retrieve", "dataset" : {} }
db.authorization.insert(j)

j = { "principal_name" : "", "principal_type" : "", "service" : "Store", "dataset" : {} }
db.authorization.insert(j)

