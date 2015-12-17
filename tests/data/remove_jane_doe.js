db = connect("localhost:27017/${DOPAMINE_TEST_DATABASE}");
db.datasets.remove( { "00080018.Value" : "2.16.756.5.5.100.3611280983.20092.1364462458.1.0" } )
