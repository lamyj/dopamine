#!/bin/sh

# Create Temporary directory
DIRECTORY=`mktemp -d`

# Set environment variables for SCP unit tests
if [ -z "$DOPAMINE_TEST_DATABASE" ]
then
    export DOPAMINE_TEST_DATABASE=dopamine_test
fi

if [ -z "$DOPAMINE_TEST_LISTENINGPORT" ]
then
    export DOPAMINE_TEST_LISTENINGPORT=11112
fi

if [ -z "$DOPAMINE_TEST_WRITINGPORT" ]
then
    export DOPAMINE_TEST_WRITINGPORT=11113
fi

# Set Environment variables for Authenticator LDAP
if [ -z "$TEST_LDAP_SERVER" ]
then
    export TEST_LDAP_SERVER=ldap://localhost
fi

if [ -z "$TEST_LDAP_BASE" ]
then
    export TEST_LDAP_BASE=cn=Users,dc=ToBeDefined,dc=local
fi

if [ -z "$TEST_LDAP_BIND" ]
then
    export TEST_LDAP_BIND=%user@ToBeDefined.local
fi

if [ -z "$TEST_LDAP_USER" ]
then
    export TEST_LDAP_USER=TOBEDEFINED
fi

if [ -z "$TEST_LDAP_PASSWORD" ]
then
    export TEST_LDAP_PASSWORD=ToB3D3fin3d
fi

# Create PACS Configuration file
cat > ${DIRECTORY}/config  << EOF
[logger]
priority=ERROR

[dicom]
storage_path=${DIRECTORY}/temp_dir
allowed_peers=*
port=${DOPAMINE_TEST_LISTENINGPORT}

[database]
hostname=localhost
port=27017
dbname=${DOPAMINE_TEST_DATABASE}
indexlist=SOPInstanceUID;PatientName;PatientID;SeriesInstanceUID;SeriesDescription;StudyInstanceUID;StudyDescription

[authenticator]
type=None

[listAddressPort]
allowed=LOCAL,REMOTE
LOCAL=localhost:${DOPAMINE_TEST_WRITINGPORT}
REMOTE=localhost:${DOPAMINE_TEST_LISTENINGPORT}

EOF

# Create Dump Dataset File for Dataset Creation
cat > ${DIRECTORY}/dataset << EOF

# Dicom-File-Format

# Dicom-Meta-Information-Header
# Used TransferSyntax: Little Endian Explicit
(0002,0000) UL 192                                      #   4, 1 FileMetaInformationGroupLength
(0002,0001) OB 00\01                                    #   2, 1 FileMetaInformationVersion
(0002,0002) UI =MRImageStorage                          #  26, 1 MediaStorageSOPClassUID
(0002,0003) UI [2.16.756.5.5.100.3611280983.20092.1364462458.1.0] #  48, 1 MediaStorageSOPInstanceUID
(0002,0010) UI =LittleEndianExplicit                    #  20, 1 TransferSyntaxUID
(0002,0012) UI [1.2.276.0.7230010.3.0.3.5.4]            #  28, 1 ImplementationClassUID
(0002,0013) SH [OFFIS_DCMTK_354]                        #  16, 1 ImplementationVersionName

# Dicom-Data-Set
# Used TransferSyntax: Little Endian Explicit
(0008,0008) CS [ORIGINAL\PRIMARY\OTHER]                 #  22, 3 ImageType
(0008,0012) DA [20140827]                               #   8, 1 InstanceCreationDate
(0008,0013) TM [103310]                                 #   6, 1 InstanceCreationTime
(0008,0016) UI =MRImageStorage                          #  26, 1 SOPClassUID
(0008,0018) UI [2.16.756.5.5.100.3611280983.20092.1364462458.1.0] #  48, 1 SOPInstanceUID
(0008,0020) DA [20130328]                               #   8, 1 StudyDate
(0008,0022) DA [20130328]                               #   8, 1 AcquisitionDate
(0008,0030) TM [101009]                                 #   6, 1 StudyTime
(0008,0032) TM [101818]                                 #   6, 1 AcquisitionTime
(0008,0050) SH (no value available)                     #   0, 0 AccessionNumber
(0008,0060) CS [MR]                                     #   2, 1 Modality
(0008,0070) LO [Bruker BioSpin MRI GmbH]                #  24, 1 Manufacturer
(0008,0080) LO [STRASBOURG]                             #  11, 1 InstitutionName
(0008,0090) PN (no value available)                     #   0, 0 ReferringPhysicianName
(0008,1010) SH [Station]                                #  8, 1 StationName
(0010,0010) PN [Doe^Jane]                               #  9, 1 PatientName
(0010,0020) LO [dopamine_test_01]                       #  17, 1 PatientID
(0010,0030) DA (no value available)                     #   0, 0 PatientBirthDate
(0010,0040) CS [F]                                      #   2, 1 PatientSex
(0010,1030) DS [5]                                      #   2, 1 PatientWeight
(0018,0020) CS [RM\IR]                                  #   6, 2 ScanningSequence
(0018,0021) CS [NONE]                                   #   4, 1 SequenceVariant
(0018,0022) CS (no value available)                     #   0, 0 ScanOptions
(0018,0023) CS [2D]                                     #   2, 1 MRAcquisitionType
(0018,0024) SH [FAIR_EPI (pvm)]                         #  14, 1 SequenceName
(0018,0050) DS [0.8]                                    #   4, 1 SliceThickness
(0018,0080) DS [18000]                                  #   6, 1 RepetitionTime
(0018,0081) DS [33]                                     #   2, 1 EchoTime
(0018,0082) DS [35.37627273]                            #  12, 1 InversionTime
(0018,0083) DS [1]                                      #   2, 1 NumberOfAverages
(0018,0084) DS [200.3334861]                            #  12, 1 ImagingFrequency
(0018,0085) SH [1H]                                     #   2, 1 ImagedNucleus
(0018,0088) DS [0.8]                                    #   4, 1 SpacingBetweenSlices
(0018,0089) IS [107]                                    #   4, 1 NumberOfPhaseEncodingSteps
(0018,0091) IS [107]                                    #   4, 1 EchoTrainLength
(0018,0094) DS [100]                                    #   4, 1 PercentPhaseFieldOfView
(0018,0095) DS [3337.783712]                            #  12, 1 PixelBandwidth
(0018,1020) LO [ParaVision 5.1]                         #  14, 1 SoftwareVersions
(0018,1030) LO [Protocol]                               #   9, 1 ProtocolName
(0018,1310) US 107\0\0\107                              #   8, 4 AcquisitionMatrix
(0018,1312) CS [COL]                                    #   4, 1 InPlanePhaseEncodingDirection
(0018,1314) DS [90]                                     #   2, 1 FlipAngle
(0018,5100) CS [HFS]                                    #   4, 1 PatientPosition
(0020,000d) UI [2.16.756.5.5.100.3611280983.19057.1364461809.7789] #  50, 1 StudyInstanceUID
(0020,000e) UI [2.16.756.5.5.100.3611280983.20092.1364462458.1] #  46, 1 SeriesInstanceUID
(0020,0010) SH [Study_id]                               #   9, 1 StudyID
(0020,0011) IS [196609]                                 #   6, 1 SeriesNumber
(0020,0012) IS [1]                                      #   2, 1 AcquisitionNumber
(0020,0013) IS [1]                                      #   2, 1 InstanceNumber
(0020,0032) DS [-15\-15\-1.6]                           #  12, 3 ImagePositionPatient
(0020,0037) DS [1\6.123031769e-17\0\-6.123031769e-17\1\0] #  40, 6 ImageOrientationPatient
(0020,0052) UI [2.16.756.5.5.100.3611280983.20092.1364462458.1.6.15.18] #  54, 1 FrameOfReferenceUID
(0020,1002) IS [75]                                     #   2, 1 ImagesInAcquisition
(0020,1040) LO (no value available)                     #   0, 0 PositionReferenceIndicator
(0020,1041) DS [-1.6]                                   #   4, 1 SliceLocation
(0028,0002) US 1                                        #   2, 1 SamplesPerPixel
(0028,0004) CS [MONOCHROME2]                            #  12, 1 PhotometricInterpretation
(0028,0010) US 128                                      #   2, 1 Rows
(0028,0011) US 128                                      #   2, 1 Columns
(0028,0030) DS [0.234375\0.234375]                      #  18, 2 PixelSpacing
(0028,0100) US 16                                       #   2, 1 BitsAllocated
(0028,0101) US 16                                       #   2, 1 BitsStored
(0028,0102) US 15                                       #   2, 1 HighBit
(0028,0103) US 1                                        #   2, 1 PixelRepresentation
(0028,0106) US 3                                        #   2, 1 SmallestImagePixelValue
(0028,0107) US 32766                                    #   2, 1 LargestImagePixelValue
(0028,1050) DS [16385]                                  #   6, 1 WindowCenter
(0028,1051) DS [32764]                                  #   6, 1 WindowWidth
(0028,1055) LO [MinMax]                                 #   6, 1 WindowCenterWidthExplanation
EOF

# Export configuration file path
export DOPAMINE_TEST_CONFIG=${DIRECTORY}/config
export DOPAMINE_TEST_DICOMFILE=${DIRECTORY}/temp_dir/2014/12/15/AB2411EA/3B11F27C/BE5F493E

export DOPAMINE_TEST_OUTPUTDIR=${DIRECTORY}/output
mkdir ${DOPAMINE_TEST_OUTPUTDIR}

# Create JavaScript to initialize mongo database
cat > ${DIRECTORY}/create_db.js << EOF
db = connect("localhost:27017/${DOPAMINE_TEST_DATABASE}");
j = { "username" : "*", "authorizedAction" : [ 1, 16, 32, 33 ] }
db.authorization.insert(j)

j = { "00080008" : [  "CS",  [  "ORIGINAL",  "PRIMARY",  "OTHER" ] ], "00080012" : [  "DA",  "20140827" ], "00080013" : [  "TM",  "103310" ], "00080016" : [  "UI",  "1.2.840.10008.5.1.4.1.1.4" ], "00080018" : [  "UI",  "2.16.756.5.5.100.3611280983.20092.1364462458.1.0" ], "00080020" : [  "DA",  "20130328" ], "00080022" : [  "DA",  "20130328" ], "00080030" : [  "TM",  "101009" ], "00080032" : [  "TM",  "101818" ], "00080050" : [  "SH",  null ], "00080060" : [  "CS",  "MR" ], "00080070" : [  "LO",  "Bruker BioSpin MRI GmbH" ], "00080080" : [  "LO",  "STRASBOURG" ], "00080090" : [  "PN",  null ], "00081010" : [  "SH",  "Station" ], "00100010" : [  "PN",  "Doe^Jane" ], "00100020" : [  "LO",  "dopamine_test_01" ], "00100030" : [  "DA",  null ], "00100040" : [  "CS",  "F" ], "00101030" : [  "DS",  5 ], "00180020" : [  "CS",  [  "RM",  "IR" ] ], "00180021" : [  "CS",  "NONE" ], "00180022" : [  "CS",  null ], "00180023" : [  "CS",  "2D" ], "00180024" : [  "SH",  "FAIR_EPI (pvm)" ], "00180050" : [  "DS",  0.8 ], "00180080" : [  "DS",  18000 ], "00180081" : [  "DS",  33 ], "00180082" : [  "DS",  35.37627273 ], "00180083" : [  "DS",  1 ], "00180084" : [  "DS",  200.3334861 ], "00180085" : [  "SH",  "1H" ], "00180088" : [  "DS",  0.8 ], "00180089" : [  "IS",  107 ], "00180091" : [  "IS",  107 ], "00180094" : [  "DS",  100 ], "00180095" : [  "DS",  3337.783712 ], "00181020" : [  "LO",  "ParaVision 5.1" ], "00181030" : [  "LO",  "Protocol" ], "00181310" : [  "US",  [  107,  0,  0,  107 ] ], "00181312" : [  "CS",  "COL" ], "00181314" : [  "DS",  90 ], "00185100" : [  "CS",  "HFS" ], "0020000d" : [  "UI",  "2.16.756.5.5.100.3611280983.19057.1364461809.7789" ], "0020000e" : [  "UI",  "2.16.756.5.5.100.3611280983.20092.1364462458.1" ], "00200010" : [  "SH",  "Study_id" ], "00200011" : [  "IS",  196609 ], "00200012" : [  "IS",  1 ], "00200013" : [  "IS",  1 ], "00200032" : [  "DS",  [  -15,  -15,  -1.6 ] ], "00200037" : [  "DS",  [  1,  6.123031769e-17,  0,  -6.123031769e-17,  1,  0 ] ], "00200052" : [  "UI",  "2.16.756.5.5.100.3611280983.20092.1364462458.1.6.15.18" ], "00201002" : [  "IS",  75 ], "00201040" : [  "LO",  null ], "00201041" : [  "DS",  -1.6 ], "00280002" : [  "US",  1 ], "00280004" : [  "CS",  "MONOCHROME2" ], "00280010" : [  "US",  128 ], "00280011" : [  "US",  128 ], "00280030" : [  "DS",  [  0.234375,  0.234375 ] ], "00280100" : [  "US",  16 ], "00280101" : [  "US",  16 ], "00280102" : [  "US",  15 ], "00280103" : [  "US",  1 ], "00280106" : [  "US",  3 ], "00280107" : [  "US",  32766 ], "00281050" : [  "DS",  16385 ], "00281051" : [  "DS",  32764 ], "00281055" : [  "LO",  "MinMax" ], "location" : "${DOPAMINE_TEST_DICOMFILE}" }
db.datasets.insert(j)

j = { "00080018" : [  "UI",  "2.16.756.5.5.100.3611280983.20092.1364462499.1.0" ], "0020000d" : [  "UI",  "2.16.756.5.5.100.3611280983.19057.1364461809.9999" ], "0020000e" : [  "UI",  "2.16.756.5.5.100.3611280983.20092.1364462499.1" ], "location" : "" }
db.datasets.insert(j)

j = { "00080018" : [  "UI",  "2.16.756.5.5.100.3611280983.20092.1364462488.1.0" ], "0020000d" : [  "UI",  "2.16.756.5.5.100.3611280983.19057.1364461809.8888" ], "0020000e" : [  "UI",  "2.16.756.5.5.100.3611280983.20092.1364462488.1" ], "location" : "/tmp/error_no_file.dcm" }
db.datasets.insert(j)
EOF

# Create JavaScript to remove mongo database
cat > ${DIRECTORY}/delete_db.js << EOF
db = connect("localhost:27017/${DOPAMINE_TEST_DATABASE}");
db.dropDatabase()
EOF

# Create sub directory for Dataset
mkdir "${DIRECTORY}/temp_dir/" "${DIRECTORY}/temp_dir/2014/" "${DIRECTORY}/temp_dir/2014/12/" "${DIRECTORY}/temp_dir/2014/12/15/" "${DIRECTORY}/temp_dir/2014/12/15/AB2411EA/" "${DIRECTORY}/temp_dir/2014/12/15/AB2411EA/3B11F27C"

# Create Dataset
dump2dcm ${DIRECTORY}/dataset "${DOPAMINE_TEST_DICOMFILE}"

# Make sure Database is empty
mongo --quiet ${DIRECTORY}/delete_db.js

# Create Database
mongo --quiet ${DIRECTORY}/create_db.js

./src/appli/dopamine &

# Execute unit tests
ctest --no-compress-output -T Test || true

termscu localhost ${DOPAMINE_TEST_LISTENINGPORT}

# Remove Database
mongo --quiet ${DIRECTORY}/delete_db.js

# Remove all temporary files
rm -rf ${DIRECTORY}
