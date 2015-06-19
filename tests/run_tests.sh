#!/bin/sh

# Start MongoDB
pgrep mongod
retval=$?
if [ "$retval" = 1 ]
then
    /usr/bin/mongod --quiet > /dev/null 2>&1 &
    sleep 1
fi 

pgrep mongod
retval=$?
if [ "$retval" = 1 ]
then
    exit 1
fi

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

# Create PACS Configuration file
cat > ${DIRECTORY}/badconfig  << EOF
[logger]
priority=ERROR

[dicom]
storage_path=${DIRECTORY}/temp_dir
allowed_peers=*
port=${DOPAMINE_TEST_LISTENINGPORT}

[database]
hostname=localhost
port=1
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
cat > ${DIRECTORY}/dopamine_test_file_01 << EOF

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
(0008,0090) PN [Gregory^House]                          #  14, 1 ReferringPhysicianName
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

cat > ${DIRECTORY}/dopamine_test_file_02 << EOF

# Dicom-File-Format

# Dicom-Meta-Information-Header
# Used TransferSyntax: Little Endian Explicit
(0002,0000) UL 198                                      #   4, 1 FileMetaInformationGroupLength
(0002,0001) OB 00\01                                    #   2, 1 FileMetaInformationVersion
(0002,0002) UI =MRImageStorage                          #  26, 1 MediaStorageSOPClassUID
(0002,0003) UI [1.2.276.0.7230010.3.1.4.8323329.4396.1430723598.811429] #  54, 1 MediaStorageSOPInstanceUID
(0002,0010) UI =LittleEndianExplicit                    #  20, 1 TransferSyntaxUID
(0002,0012) UI [1.2.276.0.7230010.3.0.3.6.0]            #  28, 1 ImplementationClassUID
(0002,0013) SH [OFFIS_DCMTK_360]                        #  16, 1 ImplementationVersionName

# Dicom-Data-Set
# Used TransferSyntax: Little Endian Explicit
(0008,0008) CS [ORIGINAL\PRIMARY]                       #  16, 2 ImageType
(0008,0012) DA [20150504]                               #   8, 1 InstanceCreationDate
(0008,0013) TM [091318]                                 #   6, 1 InstanceCreationTime
(0008,0016) UI =MRImageStorage                          #  26, 1 SOPClassUID
(0008,0018) UI [1.2.276.0.7230010.3.1.4.8323329.4396.1430723598.811429] #  54, 1 SOPInstanceUID
(0008,0020) DA [20150219]                               #   8, 1 StudyDate
(0008,0021) DA [20150219]                               #   8, 1 SeriesDate
(0008,0022) DA [20150219]                               #   8, 1 AcquisitionDate
(0008,0030) TM [093002]                                 #   6, 1 StudyTime
(0008,0031) TM [103006]                                 #   6, 1 SeriesTime
(0008,0032) TM [102507]                                 #   6, 1 AcquisitionTime
(0008,0050) SH (no value available)                     #   0, 0 AccessionNumber
(0008,0060) CS [MR]                                     #   2, 1 Modality
(0008,0070) LO [Bruker BioSpin MRI GmbH]                #  24, 1 Manufacturer
(0008,0080) LO [ICUDE Strasbourg]                       #  16, 1 InstitutionName
(0008,0090) PN [House^Gregory^^Dr.]                     #  19, 1 ReferringPhysicianName
(0008,1010) SH [Biospec]                                #   8, 1 StationName
(0008,1030) LO [PROTOCOLE_TOUT]                         #  14, 1 StudyDescription
(0008,103e) LO [T2_TurboRARE]                           #  12, 1 SeriesDescription
(0008,1050) PN [Number^thirteen]                        #  16, 1 PerformingPhysicianName
(0010,0010) PN [Mouse^Mickey]                           #  13, 1 PatientName
(0010,0020) LO [id123]                                  #   6, 1 PatientID
(0010,0030) DA [20150219]                               #   8, 1 PatientBirthDate
(0010,0040) CS [M]                                      #   2, 1 PatientSex
(0010,1030) DS [0.001]                                  #   6, 1 PatientWeight
(0010,2201) LO [to]                                     #   2, 1 PatientSpeciesDescription
(0010,2203) CS (no value available)                     #   0, 0 PatientSexNeutered
(0010,2210) CS [QUADRUPED]                              #  10, 1 AnatomicalOrientationType
(0010,2292) LO (no value available)                     #   0, 0 PatientBreedDescription
(0010,2293) SQ (Sequence with undefined length #=0)     # u/l, 1 PatientBreedCodeSequence
(fffe,e0dd) na (SequenceDelimitationItem)               #   0, 0 SequenceDelimitationItem
(0010,2294) SQ (Sequence with undefined length #=0)     # u/l, 1 BreedRegistrationSequence
(fffe,e0dd) na (SequenceDelimitationItem)               #   0, 0 SequenceDelimitationItem
(0010,2297) PN (no value available)                     #   0, 0 ResponsiblePerson
(0010,2299) LO (no value available)                     #   0, 0 ResponsibleOrganization
(0010,4000) LT (no value available)                     #   0, 0 PatientComments
(0018,0020) CS [RM\IR]                                  #   6, 2 ScanningSequence
(0018,0021) CS [NONE]                                   #   4, 1 SequenceVariant
(0018,0022) CS (no value available)                     #   0, 0 ScanOptions
(0018,0023) CS [2D]                                     #   2, 1 MRAcquisitionType
(0018,0024) SH [Bruker:RARE]                            #  12, 1 SequenceName
(0018,0050) DS [0.8]                                    #   4, 1 SliceThickness
(0018,0080) DS [6000]                                   #   4, 1 RepetitionTime
(0018,0081) DS [36]                                     #   2, 1 EchoTime
(0018,0082) DS (no value available)                     #   0, 0 InversionTime
(0018,0083) DS [4]                                      #   2, 1 NumberOfAverages
(0018,0084) DS [300.333]                                #   8, 1 ImagingFrequency
(0018,0085) SH [1H]                                     #   2, 1 ImagedNucleus
(0018,0088) DS [0.8]                                    #   4, 1 SpacingBetweenSlices
(0018,0089) IS [96]                                     #   2, 1 NumberOfPhaseEncodingSteps
(0018,0091) IS [8]                                      #   2, 1 EchoTrainLength
(0018,0095) DS [156.25]                                 #   6, 1 PixelBandwidth
(0018,1020) LO [ParaVision6.0]                          #  14, 1 SoftwareVersions
(0018,1030) LO [T2_TurboRARE]                           #  12, 1 ProtocolName
(0018,1310) US 0\0\0\0                                  #   8, 4 AcquisitionMatrix
(0018,1312) CS (no value available)                     #   0, 0 InPlanePhaseEncodingDirection
(0018,1314) DS [90]                                     #   2, 1 FlipAngle
(0018,5100) CS [HFS]                                    #   4, 1 PatientPosition
(0020,000d) UI [2.16.756.5.5.100.1333920868.19866.1424334602.23] #  48, 1 StudyInstanceUID
(0020,000e) UI [2.16.756.5.5.100.1333920868.31960.1424338206.1] #  46, 1 SeriesInstanceUID
(0020,0010) SH [FLIIAM^19022015]                        #  16, 1 StudyID
(0020,0011) IS [50001]                                  #   6, 1 SeriesNumber
(0020,0013) IS [1]                                      #   2, 1 InstanceNumber
(0020,0032) DS [-7.35943\-8.78292\-7.85765]             #  26, 3 ImagePositionPatient
(0020,0037) DS [1\0\0\0\1\0]                            #  12, 6 ImageOrientationPatient
(0020,0052) UI [2.16.756.5.5.100.1333920868.31960.1424338206.1] #  46, 1 FrameOfReferenceUID
(0020,0060) CS (no value available)                     #   0, 0 Laterality
(0020,1002) IS [3]                                      #   1, 1 ImagesInAcquisition
(0020,1040) LO (no value available)                     #   0, 0 PositionReferenceIndicator
(0020,4000) LT (no value available)                     #   0, 0 ImageComments
(0028,0002) US 1                                        #   2, 1 SamplesPerPixel
(0028,0004) CS [MONOCHROME2]                            #  12, 1 PhotometricInterpretation
(0028,0010) US 128                                      #   2, 1 Rows
(0028,0011) US 128                                      #   2, 1 Columns
(0028,0030) DS [0.125\0.125]                            #  12, 2 PixelSpacing
(0028,0100) US 16                                       #   2, 1 BitsAllocated
(0028,0101) US 16                                       #   2, 1 BitsStored
(0028,0102) US 15                                       #   2, 1 HighBit
(0028,0103) US 0                                        #   2, 1 PixelRepresentation
(0028,1052) DS [-32768]                                 #   6, 1 RescaleIntercept
(0028,1053) DS [1]                                      #   2, 1 RescaleSlope
(0028,1054) LO [US]                                     #   2, 1 RescaleType
EOF

cat > ${DIRECTORY}/dopamine_test_file_03 << EOF

# Dicom-File-Format

# Dicom-Meta-Information-Header
# Used TransferSyntax: Little Endian Explicit
(0002,0000) UL 198                                      #   4, 1 FileMetaInformationGroupLength
(0002,0001) OB 00\01                                    #   2, 1 FileMetaInformationVersion
(0002,0002) UI =MRImageStorage                          #  26, 1 MediaStorageSOPClassUID
(0002,0003) UI [1.2.276.0.7230010.3.1.4.8323329.4396.1430723598.811430] #  54, 1 MediaStorageSOPInstanceUID
(0002,0010) UI =LittleEndianExplicit                    #  20, 1 TransferSyntaxUID
(0002,0012) UI [1.2.276.0.7230010.3.0.3.6.0]            #  28, 1 ImplementationClassUID
(0002,0013) SH [OFFIS_DCMTK_360]                        #  16, 1 ImplementationVersionName

# Dicom-Data-Set
# Used TransferSyntax: Little Endian Explicit
(0008,0008) CS [ORIGINAL\PRIMARY]                       #  16, 2 ImageType
(0008,0012) DA [20150504]                               #   8, 1 InstanceCreationDate
(0008,0013) TM [091318]                                 #   6, 1 InstanceCreationTime
(0008,0016) UI =MRImageStorage                          #  26, 1 SOPClassUID
(0008,0018) UI [1.2.276.0.7230010.3.1.4.8323329.4396.1430723598.811430] #  54, 1 SOPInstanceUID
(0008,0020) DA [20150219]                               #   8, 1 StudyDate
(0008,0021) DA [20150219]                               #   8, 1 SeriesDate
(0008,0022) DA [20150219]                               #   8, 1 AcquisitionDate
(0008,0030) TM [093002]                                 #   6, 1 StudyTime
(0008,0031) TM [103006]                                 #   6, 1 SeriesTime
(0008,0032) TM [102507]                                 #   6, 1 AcquisitionTime
(0008,0050) SH (no value available)                     #   0, 0 AccessionNumber
(0008,0060) CS [MR]                                     #   2, 1 Modality
(0008,0070) LO [Bruker BioSpin MRI GmbH]                #  24, 1 Manufacturer
(0008,0080) LO [ICUDE Strasbourg]                       #  16, 1 InstitutionName
(0008,0090) PN [House^Gregory^^Dr.]                     #  19, 1 ReferringPhysicianName
(0008,1010) SH [Biospec]                                #   8, 1 StationName
(0008,1030) LO [PROTOCOLE_TOUT]                         #  14, 1 StudyDescription
(0008,103e) LO [T2_TurboRARE]                           #  12, 1 SeriesDescription
(0008,1050) PN [Number^thirteen]                        #  16, 1 PerformingPhysicianName
(0010,0010) PN [Mouse^Mickey]                           #  13, 1 PatientName
(0010,0020) LO [id123]                                  #   6, 1 PatientID
(0010,0030) DA [20150219]                               #   8, 1 PatientBirthDate
(0010,0040) CS [M]                                      #   2, 1 PatientSex
(0010,1030) DS [0.001]                                  #   6, 1 PatientWeight
(0010,2201) LO [to]                                     #   2, 1 PatientSpeciesDescription
(0010,2203) CS (no value available)                     #   0, 0 PatientSexNeutered
(0010,2210) CS [QUADRUPED]                              #  10, 1 AnatomicalOrientationType
(0010,2292) LO (no value available)                     #   0, 0 PatientBreedDescription
(0010,2293) SQ (Sequence with undefined length #=0)     # u/l, 1 PatientBreedCodeSequence
(fffe,e0dd) na (SequenceDelimitationItem)               #   0, 0 SequenceDelimitationItem
(0010,2294) SQ (Sequence with undefined length #=0)     # u/l, 1 BreedRegistrationSequence
(fffe,e0dd) na (SequenceDelimitationItem)               #   0, 0 SequenceDelimitationItem
(0010,2297) PN (no value available)                     #   0, 0 ResponsiblePerson
(0010,2299) LO (no value available)                     #   0, 0 ResponsibleOrganization
(0010,4000) LT (no value available)                     #   0, 0 PatientComments
(0018,0020) CS [RM\IR]                                  #   6, 2 ScanningSequence
(0018,0021) CS [NONE]                                   #   4, 1 SequenceVariant
(0018,0022) CS (no value available)                     #   0, 0 ScanOptions
(0018,0023) CS [2D]                                     #   2, 1 MRAcquisitionType
(0018,0024) SH [Bruker:RARE]                            #  12, 1 SequenceName
(0018,0050) DS [0.8]                                    #   4, 1 SliceThickness
(0018,0080) DS [6000]                                   #   4, 1 RepetitionTime
(0018,0081) DS [36]                                     #   2, 1 EchoTime
(0018,0082) DS (no value available)                     #   0, 0 InversionTime
(0018,0083) DS [4]                                      #   2, 1 NumberOfAverages
(0018,0084) DS [300.333]                                #   8, 1 ImagingFrequency
(0018,0085) SH [1H]                                     #   2, 1 ImagedNucleus
(0018,0088) DS [0.8]                                    #   4, 1 SpacingBetweenSlices
(0018,0089) IS [96]                                     #   2, 1 NumberOfPhaseEncodingSteps
(0018,0091) IS [8]                                      #   2, 1 EchoTrainLength
(0018,0095) DS [156.25]                                 #   6, 1 PixelBandwidth
(0018,1020) LO [ParaVision6.0]                          #  14, 1 SoftwareVersions
(0018,1030) LO [T2_TurboRARE]                           #  12, 1 ProtocolName
(0018,1310) US 0\0\0\0                                  #   8, 4 AcquisitionMatrix
(0018,1312) CS (no value available)                     #   0, 0 InPlanePhaseEncodingDirection
(0018,1314) DS [90]                                     #   2, 1 FlipAngle
(0018,5100) CS [HFS]                                    #   4, 1 PatientPosition
(0020,000d) UI [2.16.756.5.5.100.1333920868.19866.1424334602.23] #  48, 1 StudyInstanceUID
(0020,000e) UI [2.16.756.5.5.100.1333920868.31960.1424338206.1] #  46, 1 SeriesInstanceUID
(0020,0010) SH [FLIIAM^19022015]                        #  16, 1 StudyID
(0020,0011) IS [50001]                                  #   6, 1 SeriesNumber
(0020,0013) IS [2]                                      #   2, 1 InstanceNumber
(0020,0032) DS [-7.35943\-8.78292\-7.05765]             #  26, 3 ImagePositionPatient
(0020,0037) DS [1\0\0\0\1\0]                            #  12, 6 ImageOrientationPatient
(0020,0052) UI [2.16.756.5.5.100.1333920868.31960.1424338206.1] #  46, 1 FrameOfReferenceUID
(0020,0060) CS (no value available)                     #   0, 0 Laterality
(0020,1002) IS [3]                                      #   1, 1 ImagesInAcquisition
(0020,1040) LO (no value available)                     #   0, 0 PositionReferenceIndicator
(0020,4000) LT (no value available)                     #   0, 0 ImageComments
(0028,0002) US 1                                        #   2, 1 SamplesPerPixel
(0028,0004) CS [MONOCHROME2]                            #  12, 1 PhotometricInterpretation
(0028,0010) US 128                                      #   2, 1 Rows
(0028,0011) US 128                                      #   2, 1 Columns
(0028,0030) DS [0.125\0.125]                            #  12, 2 PixelSpacing
(0028,0100) US 16                                       #   2, 1 BitsAllocated
(0028,0101) US 16                                       #   2, 1 BitsStored
(0028,0102) US 15                                       #   2, 1 HighBit
(0028,0103) US 0                                        #   2, 1 PixelRepresentation
(0028,1052) DS [-32768]                                 #   6, 1 RescaleIntercept
(0028,1053) DS [1]                                      #   2, 1 RescaleSlope
(0028,1054) LO [US]                                     #   2, 1 RescaleType
EOF

cat > ${DIRECTORY}/dopamine_test_file_04 << EOF

# Dicom-File-Format

# Dicom-Meta-Information-Header
# Used TransferSyntax: Little Endian Explicit
(0002,0000) UL 198                                      #   4, 1 FileMetaInformationGroupLength
(0002,0001) OB 00\01                                    #   2, 1 FileMetaInformationVersion
(0002,0002) UI =MRImageStorage                          #  26, 1 MediaStorageSOPClassUID
(0002,0003) UI [1.2.276.0.7230010.3.1.4.8323329.4396.1430723598.811431] #  54, 1 MediaStorageSOPInstanceUID
(0002,0010) UI =LittleEndianExplicit                    #  20, 1 TransferSyntaxUID
(0002,0012) UI [1.2.276.0.7230010.3.0.3.6.0]            #  28, 1 ImplementationClassUID
(0002,0013) SH [OFFIS_DCMTK_360]                        #  16, 1 ImplementationVersionName

# Dicom-Data-Set
# Used TransferSyntax: Little Endian Explicit
(0008,0008) CS [ORIGINAL\PRIMARY]                       #  16, 2 ImageType
(0008,0012) DA [20150504]                               #   8, 1 InstanceCreationDate
(0008,0013) TM [091318]                                 #   6, 1 InstanceCreationTime
(0008,0016) UI =MRImageStorage                          #  26, 1 SOPClassUID
(0008,0018) UI [1.2.276.0.7230010.3.1.4.8323329.4396.1430723598.811431] #  54, 1 SOPInstanceUID
(0008,0020) DA [20150219]                               #   8, 1 StudyDate
(0008,0021) DA [20150219]                               #   8, 1 SeriesDate
(0008,0022) DA [20150219]                               #   8, 1 AcquisitionDate
(0008,0030) TM [093002]                                 #   6, 1 StudyTime
(0008,0031) TM [103006]                                 #   6, 1 SeriesTime
(0008,0032) TM [102507]                                 #   6, 1 AcquisitionTime
(0008,0050) SH (no value available)                     #   0, 0 AccessionNumber
(0008,0060) CS [MR]                                     #   2, 1 Modality
(0008,0070) LO [Bruker BioSpin MRI GmbH]                #  24, 1 Manufacturer
(0008,0080) LO [ICUDE Strasbourg]                       #  16, 1 InstitutionName
(0008,0090) PN [House^Gregory^^Dr.]                     #  19, 1 ReferringPhysicianName
(0008,1010) SH [Biospec]                                #   8, 1 StationName
(0008,1030) LO [PROTOCOLE_TOUT]                         #  14, 1 StudyDescription
(0008,103e) LO [T2_TurboRARE]                           #  12, 1 SeriesDescription
(0008,1050) PN [Number^thirteen]                        #  16, 1 PerformingPhysicianName
(0010,0010) PN [Mouse^Mickey]                           #  13, 1 PatientName
(0010,0020) LO [id123]                                  #   6, 1 PatientID
(0010,0030) DA [20150219]                               #   8, 1 PatientBirthDate
(0010,0040) CS [M]                                      #   2, 1 PatientSex
(0010,1030) DS [0.001]                                  #   6, 1 PatientWeight
(0010,2201) LO [to]                                     #   2, 1 PatientSpeciesDescription
(0010,2203) CS (no value available)                     #   0, 0 PatientSexNeutered
(0010,2210) CS [QUADRUPED]                              #  10, 1 AnatomicalOrientationType
(0010,2292) LO (no value available)                     #   0, 0 PatientBreedDescription
(0010,2293) SQ (Sequence with undefined length #=0)     # u/l, 1 PatientBreedCodeSequence
(fffe,e0dd) na (SequenceDelimitationItem)               #   0, 0 SequenceDelimitationItem
(0010,2294) SQ (Sequence with undefined length #=0)     # u/l, 1 BreedRegistrationSequence
(fffe,e0dd) na (SequenceDelimitationItem)               #   0, 0 SequenceDelimitationItem
(0010,2297) PN (no value available)                     #   0, 0 ResponsiblePerson
(0010,2299) LO (no value available)                     #   0, 0 ResponsibleOrganization
(0010,4000) LT (no value available)                     #   0, 0 PatientComments
(0018,0020) CS [RM\IR]                                  #   6, 2 ScanningSequence
(0018,0021) CS [NONE]                                   #   4, 1 SequenceVariant
(0018,0022) CS (no value available)                     #   0, 0 ScanOptions
(0018,0023) CS [2D]                                     #   2, 1 MRAcquisitionType
(0018,0024) SH [Bruker:RARE]                            #  12, 1 SequenceName
(0018,0050) DS [0.8]                                    #   4, 1 SliceThickness
(0018,0080) DS [6000]                                   #   4, 1 RepetitionTime
(0018,0081) DS [36]                                     #   2, 1 EchoTime
(0018,0082) DS (no value available)                     #   0, 0 InversionTime
(0018,0083) DS [4]                                      #   2, 1 NumberOfAverages
(0018,0084) DS [300.333]                                #   8, 1 ImagingFrequency
(0018,0085) SH [1H]                                     #   2, 1 ImagedNucleus
(0018,0088) DS [0.8]                                    #   4, 1 SpacingBetweenSlices
(0018,0089) IS [96]                                     #   2, 1 NumberOfPhaseEncodingSteps
(0018,0091) IS [8]                                      #   2, 1 EchoTrainLength
(0018,0095) DS [156.25]                                 #   6, 1 PixelBandwidth
(0018,1020) LO [ParaVision6.0]                          #  14, 1 SoftwareVersions
(0018,1030) LO [T2_TurboRARE]                           #  12, 1 ProtocolName
(0018,1310) US 0\0\0\0                                  #   8, 4 AcquisitionMatrix
(0018,1312) CS (no value available)                     #   0, 0 InPlanePhaseEncodingDirection
(0018,1314) DS [90]                                     #   2, 1 FlipAngle
(0018,5100) CS [HFS]                                    #   4, 1 PatientPosition
(0020,000d) UI [2.16.756.5.5.100.1333920868.19866.1424334602.23] #  48, 1 StudyInstanceUID
(0020,000e) UI [2.16.756.5.5.100.1333920868.31960.1424338206.1] #  46, 1 SeriesInstanceUID
(0020,0010) SH [FLIIAM^19022015]                        #  16, 1 StudyID
(0020,0011) IS [50001]                                  #   6, 1 SeriesNumber
(0020,0013) IS [3]                                      #   2, 1 InstanceNumber
(0020,0032) DS [-7.35943\-8.78292\-6.25765]             #  26, 3 ImagePositionPatient
(0020,0037) DS [1\0\0\0\1\0]                            #  12, 6 ImageOrientationPatient
(0020,0052) UI [2.16.756.5.5.100.1333920868.31960.1424338206.1] #  46, 1 FrameOfReferenceUID
(0020,0060) CS (no value available)                     #   0, 0 Laterality
(0020,1002) IS [3]                                      #   1, 1 ImagesInAcquisition
(0020,1040) LO (no value available)                     #   0, 0 PositionReferenceIndicator
(0020,4000) LT (no value available)                     #   0, 0 ImageComments
(0028,0002) US 1                                        #   2, 1 SamplesPerPixel
(0028,0004) CS [MONOCHROME2]                            #  12, 1 PhotometricInterpretation
(0028,0010) US 128                                      #   2, 1 Rows
(0028,0011) US 128                                      #   2, 1 Columns
(0028,0030) DS [0.125\0.125]                            #  12, 2 PixelSpacing
(0028,0100) US 16                                       #   2, 1 BitsAllocated
(0028,0101) US 16                                       #   2, 1 BitsStored
(0028,0102) US 15                                       #   2, 1 HighBit
(0028,0103) US 0                                        #   2, 1 PixelRepresentation
(0028,1052) DS [-32768]                                 #   6, 1 RescaleIntercept
(0028,1053) DS [1]                                      #   2, 1 RescaleSlope
(0028,1054) LO [US]                                     #   2, 1 RescaleType
EOF


cat > ${DIRECTORY}/dopamine_test_file_05 << EOF

# Dicom-File-Format

# Dicom-Meta-Information-Header
# Used TransferSyntax: Little Endian Explicit
(0002,0000) UL 192                                      #   4, 1 FileMetaInformationGroupLength
(0002,0001) OB 00\01                                    #   2, 1 FileMetaInformationVersion
(0002,0002) UI =MRImageStorage                          #  26, 1 MediaStorageSOPClassUID
(0002,0003) UI [2.16.756.5.5.100.3611280983.20092.9964462499.1.0] #  48, 1 MediaStorageSOPInstanceUID
(0002,0010) UI =LittleEndianExplicit                    #  20, 1 TransferSyntaxUID
(0002,0012) UI [1.2.276.0.7230010.3.0.3.5.4]            #  28, 1 ImplementationClassUID
(0002,0013) SH [OFFIS_DCMTK_354]                        #  16, 1 ImplementationVersionName

# Dicom-Data-Set
# Used TransferSyntax: Little Endian Explicit
(0008,0008) CS [ORIGINAL\PRIMARY\OTHER]                 #  22, 3 ImageType
(0008,0012) DA [20140828]                               #   8, 1 InstanceCreationDate
(0008,0013) TM [113410]                                 #   6, 1 InstanceCreationTime
(0008,0016) UI =MRImageStorage                          #  26, 1 SOPClassUID
(0008,0018) UI [2.16.756.5.5.100.3611280983.20092.9964462499.1.0] #  48, 1 SOPInstanceUID
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
(0010,0010) PN [Doe^John]                               #  9, 1 PatientName
(0010,0020) LO [dopamine_test_02]                       #  17, 1 PatientID
(0010,0030) DA (no value available)                     #   0, 0 PatientBirthDate
(0010,0040) CS [M]                                      #   2, 1 PatientSex
(0010,1030) DS [6]                                      #   2, 1 PatientWeight
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
(0020,000d) UI [2.16.756.5.5.100.3611280983.19057.9964462499.7789] #  50, 1 StudyInstanceUID
(0020,000e) UI [2.16.756.5.5.100.3611280983.20092.9964462499.1] #  46, 1 SeriesInstanceUID
(0020,0010) SH [Study_id]                               #   9, 1 StudyID
(0020,0011) IS [196609]                                 #   6, 1 SeriesNumber
(0020,0012) IS [1]                                      #   2, 1 AcquisitionNumber
(0020,0013) IS [1]                                      #   2, 1 InstanceNumber
(0020,0032) DS [-15\-15\-1.6]                           #  12, 3 ImagePositionPatient
(0020,0037) DS [1\6.123031769e-17\0\-6.123031769e-17\1\0] #  40, 6 ImageOrientationPatient
(0020,0052) UI [2.16.756.5.5.100.3611280983.20092.9964462499.1.6.15.18] #  54, 1 FrameOfReferenceUID
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

cat > ${DIRECTORY}/dopamine_test_file_06 << EOF

# Dicom-File-Format

# Dicom-Meta-Information-Header
# Used TransferSyntax: Little Endian Explicit
(0002,0000) UL 192                                      #   4, 1 FileMetaInformationGroupLength
(0002,0001) OB 00\01                                    #   2, 1 FileMetaInformationVersion
(0002,0002) UI =MRImageStorage                          #  26, 1 MediaStorageSOPClassUID
(0002,0003) UI [2.16.756.5.5.100.3611280983.20092.9964462499.1.9] #  48, 1 MediaStorageSOPInstanceUID
(0002,0010) UI =LittleEndianExplicit                    #  20, 1 TransferSyntaxUID
(0002,0012) UI [1.2.276.0.7230010.3.0.3.5.4]            #  28, 1 ImplementationClassUID
(0002,0013) SH [OFFIS_DCMTK_354]                        #  16, 1 ImplementationVersionName

# Dicom-Data-Set
# Used TransferSyntax: Little Endian Explicit
(0008,0008) CS [ORIGINAL\PRIMARY\OTHER]                 #  22, 3 ImageType
(0008,0012) DA [20140828]                               #   8, 1 InstanceCreationDate
(0008,0013) TM [113410]                                 #   6, 1 InstanceCreationTime
(0008,0016) UI =MRImageStorage                          #  26, 1 SOPClassUID
(0008,0018) UI [2.16.756.5.5.100.3611280983.20092.9964462499.1.9] #  48, 1 SOPInstanceUID
(0008,0020) DA [20130328]                               #   8, 1 StudyDate
(0008,0022) DA [20130328]                               #   8, 1 AcquisitionDate
(0008,0030) TM [101009]                                 #   6, 1 StudyTime
(0008,0032) TM [101818]                                 #   6, 1 AcquisitionTime
(0008,0050) SH (no value available)                     #   0, 0 AccessionNumber
(0008,0060) CS [MR]                                     #   2, 1 Modality
(0008,0070) LO [Bruker BioSpin MRI GmbH]                #  24, 1 Manufacturer
(0008,0080) LO [STRASBOURG]                             #  11, 1 InstitutionName
(0008,0090) PN [Greg^House]                             #  11, 1 ReferringPhysicianName
(0008,1010) SH [Station]                                #   8, 1 StationName
(0010,0010) PN [Doe^John]                               #   9, 1 PatientName
(0010,0020) LO [dopamine_test_02]                       #  17, 1 PatientID
(0010,0030) DA (no value available)                     #   0, 0 PatientBirthDate
(0010,0040) CS [M]                                      #   2, 1 PatientSex
(0010,1030) DS [6]                                      #   2, 1 PatientWeight
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
(0020,000d) UI [2.16.756.5.5.100.3611280983.19057.9964462499.7780] #  50, 1 StudyInstanceUID
(0020,000e) UI [2.16.756.5.5.100.3611280983.20092.9964462499.1] #  46, 1 SeriesInstanceUID
(0020,0010) SH [Study_id]                               #   9, 1 StudyID
(0020,0011) IS [196609]                                 #   6, 1 SeriesNumber
(0020,0012) IS [1]                                      #   2, 1 AcquisitionNumber
(0020,0013) IS [1]                                      #   2, 1 InstanceNumber
(0020,0032) DS [-15\-15\-1.6]                           #  12, 3 ImagePositionPatient
(0020,0037) DS [1\6.123031769e-17\0\-6.123031769e-17\1\0] #  40, 6 ImageOrientationPatient
(0020,0052) UI [2.16.756.5.5.100.3611280983.20092.9964462499.1.6.15.18] #  54, 1 FrameOfReferenceUID
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

cat > ${DIRECTORY}/dopamine_test_file_07 << EOF

# Dicom-File-Format

# Dicom-Meta-Information-Header
# Used TransferSyntax: Little Endian Explicit
(0002,0000) UL 198                                      #   4, 1 FileMetaInformationGroupLength
(0002,0001) OB 00\01                                    #   2, 1 FileMetaInformationVersion
(0002,0002) UI =MRImageStorage                          #  26, 1 MediaStorageSOPClassUID
(0002,0003) UI [1.2.276.0.7230010.3.1.4.8323329.7922.1432822445.489112] #  54, 1 MediaStorageSOPInstanceUID
(0002,0010) UI =LittleEndianExplicit                    #  20, 1 TransferSyntaxUID
(0002,0012) UI [1.2.276.0.7230010.3.0.3.6.0]            #  28, 1 ImplementationClassUID
(0002,0013) SH [OFFIS_DCMTK_360]                        #  16, 1 ImplementationVersionName

# Dicom-Data-Set
# Used TransferSyntax: Little Endian Explicit
(0008,0008) CS [ORIGINAL\PRIMARY]                       #  16, 2 ImageType
(0008,0012) DA [20150528]                               #   8, 1 InstanceCreationDate
(0008,0013) TM [161405]                                 #   6, 1 InstanceCreationTime
(0008,0016) UI =MRImageStorage                          #  26, 1 SOPClassUID
(0008,0018) UI [1.2.276.0.7230010.3.1.4.8323329.7922.1432822445.489112] #  54, 1 SOPInstanceUID
(0008,0020) DA [20120801]                               #   8, 1 StudyDate
(0008,0021) DA (no value available)                     #   0, 0 SeriesDate
(0008,0022) DA [20120801]                               #   8, 1 AcquisitionDate
(0008,0030) TM [181049]                                 #   6, 1 StudyTime
(0008,0031) TM (no value available)                     #   0, 0 SeriesTime
(0008,0032) TM [171138]                                 #   6, 1 AcquisitionTime
(0008,0050) SH (no value available)                     #   0, 0 AccessionNumber
(0008,0060) CS [MR]                                     #   2, 1 Modality
(0008,0070) LO [Bruker BioSpin MRI GmbH]                #  24, 1 Manufacturer
(0008,0080) LO [FLI-IAM]                                #   8, 1 InstitutionName
(0008,0090) PN (no value available)                     #   0, 0 ReferringPhysicianName
(0008,1010) SH [BioSpec 47/40]                          #  14, 1 StationName
(0008,1030) LO (no value available)                     #   0, 0 StudyDescription
(0008,103e) LO [Protocol_00]                            #  12, 1 SeriesDescription
(0008,1050) PN [fli_iam]                                #   8, 1 PerformingPhysicianName
(0010,0010) PN [Gonzales^Speedy]                        #  16, 1 PatientName
(0010,0020) LO [subject_03]                             #  10, 1 PatientID
(0010,0030) DA (no value available)                     #   0, 0 PatientBirthDate
(0010,0040) CS [M]                                      #   2, 1 PatientSex
(0010,1030) DS [5]                                      #   2, 1 PatientWeight
(0010,2201) LO [to]                                     #   2, 1 PatientSpeciesDescription
(0010,2203) CS (no value available)                     #   0, 0 PatientSexNeutered
(0010,2210) CS [QUADRUPED]                              #  10, 1 AnatomicalOrientationType
(0010,2292) LO (no value available)                     #   0, 0 PatientBreedDescription
(0010,2293) SQ (Sequence with undefined length #=0)     # u/l, 1 PatientBreedCodeSequence
(fffe,e0dd) na (SequenceDelimitationItem)               #   0, 0 SequenceDelimitationItem
(0010,2294) SQ (Sequence with undefined length #=0)     # u/l, 1 BreedRegistrationSequence
(fffe,e0dd) na (SequenceDelimitationItem)               #   0, 0 SequenceDelimitationItem
(0010,2297) PN (no value available)                     #   0, 0 ResponsiblePerson
(0010,2299) LO (no value available)                     #   0, 0 ResponsibleOrganization
(0010,4000) LT (no value available)                     #   0, 0 PatientComments
(0018,0020) CS [RM\IR]                                  #   6, 2 ScanningSequence
(0018,0021) CS [NONE]                                   #   4, 1 SequenceVariant
(0018,0022) CS (no value available)                     #   0, 0 ScanOptions
(0018,0023) CS [2D]                                     #   2, 1 MRAcquisitionType
(0018,0024) SH [FLASH (pvm)]                            #  12, 1 SequenceName
(0018,0050) DS [2]                                      #   2, 1 SliceThickness
(0018,0080) DS [100]                                    #   4, 1 RepetitionTime
(0018,0081) DS [6]                                      #   2, 1 EchoTime
(0018,0082) DS (no value available)                     #   0, 0 InversionTime
(0018,0083) DS [1]                                      #   2, 1 NumberOfAverages
(0018,0084) DS [188.484]                                #   8, 1 ImagingFrequency
(0018,0085) SH [19F]                                    #   4, 1 ImagedNucleus
(0018,0088) DS [56.5685]                                #   8, 1 SpacingBetweenSlices
(0018,0089) IS [128]                                    #   4, 1 NumberOfPhaseEncodingSteps
(0018,0091) IS [1]                                      #   2, 1 EchoTrainLength
(0018,0095) DS [390.625]                                #   8, 1 PixelBandwidth
(0018,1020) LO [ParaVision5.1]                          #  14, 1 SoftwareVersions
(0018,1030) LO [Protocol_00]                            #  12, 1 ProtocolName
(0018,1310) US 128\0\0\128                              #   8, 4 AcquisitionMatrix
(0018,1312) CS [COL]                                    #   4, 1 InPlanePhaseEncodingDirection
(0018,1314) DS [30]                                     #   2, 1 FlipAngle
(0018,5100) CS [HFS]                                    #   4, 1 PatientPosition
(0020,000d) UI [2.16.756.5.5.100.3611280983.15764.1343833849.7296] #  50, 1 StudyInstanceUID
(0020,000e) UI [2.16.756.5.5.100.3611280983.22096.1343833916.3] #  46, 1 SeriesInstanceUID
(0020,0010) SH [2]                                      #   2, 1 StudyID
(0020,0011) IS [10001]                                  #   6, 1 SeriesNumber
(0020,0013) IS [1]                                      #   2, 1 InstanceNumber
(0020,0032) DS [-40\-40\0]                              #  10, 3 ImagePositionPatient
(0020,0037) DS [1\0\0\0\1\0]                            #  12, 6 ImageOrientationPatient
(0020,0052) UI [2.16.756.5.5.100.3611280983.22096.1343833916.3] #  46, 1 FrameOfReferenceUID
(0020,0060) CS (no value available)                     #   0, 0 Laterality
(0020,1002) IS [3]                                      #   2, 1 ImagesInAcquisition
(0020,1040) LO (no value available)                     #   0, 0 PositionReferenceIndicator
(0020,4000) LT (no value available)                     #   0, 0 ImageComments
(0028,0002) US 1                                        #   2, 1 SamplesPerPixel
(0028,0004) CS [MONOCHROME2]                            #  12, 1 PhotometricInterpretation
(0028,0010) US 128                                      #   2, 1 Rows
(0028,0011) US 128                                      #   2, 1 Columns
(0028,0030) DS [0.625\0.625]                            #  12, 2 PixelSpacing
(0028,0100) US 16                                       #   2, 1 BitsAllocated
(0028,0101) US 16                                       #   2, 1 BitsStored
(0028,0102) US 15                                       #   2, 1 HighBit
(0028,0103) US 0                                        #   2, 1 PixelRepresentation
(0028,1052) DS [-32768]                                 #   6, 1 RescaleIntercept
(0028,1053) DS [1]                                      #   2, 1 RescaleSlope
(0028,1054) LO [US]                                     #   2, 1 RescaleType
EOF

cat > ${DIRECTORY}/dopamine_test_file_08 << EOF

# Dicom-File-Format

# Dicom-Meta-Information-Header
# Used TransferSyntax: Little Endian Explicit
(0002,0000) UL 198                                      #   4, 1 FileMetaInformationGroupLength
(0002,0001) OB 00\01                                    #   2, 1 FileMetaInformationVersion
(0002,0002) UI =MRImageStorage                          #  26, 1 MediaStorageSOPClassUID
(0002,0003) UI [1.2.276.0.7230010.3.1.4.8323329.7922.1432822445.489113] #  54, 1 MediaStorageSOPInstanceUID
(0002,0010) UI =LittleEndianExplicit                    #  20, 1 TransferSyntaxUID
(0002,0012) UI [1.2.276.0.7230010.3.0.3.6.0]            #  28, 1 ImplementationClassUID
(0002,0013) SH [OFFIS_DCMTK_360]                        #  16, 1 ImplementationVersionName

# Dicom-Data-Set
# Used TransferSyntax: Little Endian Explicit
(0008,0008) CS [ORIGINAL\PRIMARY]                       #  16, 2 ImageType
(0008,0012) DA [20150528]                               #   8, 1 InstanceCreationDate
(0008,0013) TM [161405]                                 #   6, 1 InstanceCreationTime
(0008,0016) UI =MRImageStorage                          #  26, 1 SOPClassUID
(0008,0018) UI [1.2.276.0.7230010.3.1.4.8323329.7922.1432822445.489113] #  54, 1 SOPInstanceUID
(0008,0020) DA [20120801]                               #   8, 1 StudyDate
(0008,0021) DA (no value available)                     #   0, 0 SeriesDate
(0008,0022) DA [20120801]                               #   8, 1 AcquisitionDate
(0008,0030) TM [181049]                                 #   6, 1 StudyTime
(0008,0031) TM (no value available)                     #   0, 0 SeriesTime
(0008,0032) TM [171138]                                 #   6, 1 AcquisitionTime
(0008,0050) SH (no value available)                     #   0, 0 AccessionNumber
(0008,0060) CS [MR]                                     #   2, 1 Modality
(0008,0070) LO [Bruker BioSpin MRI GmbH]                #  24, 1 Manufacturer
(0008,0080) LO [FLI-IAM]                                #   8, 1 InstitutionName
(0008,0090) PN (no value available)                     #   0, 0 ReferringPhysicianName
(0008,1010) SH [BioSpec 47/40]                          #  14, 1 StationName
(0008,1030) LO (no value available)                     #   0, 0 StudyDescription
(0008,103e) LO [Protocol_00]                            #  12, 1 SeriesDescription
(0008,1050) PN [fli_iam]                                #   8, 1 PerformingPhysicianName
(0010,0010) PN [Gonzales^Speedy]                        #  16, 1 PatientName
(0010,0020) LO [subject_03]                             #  10, 1 PatientID
(0010,0030) DA (no value available)                     #   0, 0 PatientBirthDate
(0010,0040) CS [M]                                      #   2, 1 PatientSex
(0010,1030) DS [5]                                      #   2, 1 PatientWeight
(0010,2201) LO [to]                                     #   2, 1 PatientSpeciesDescription
(0010,2203) CS (no value available)                     #   0, 0 PatientSexNeutered
(0010,2210) CS [QUADRUPED]                              #  10, 1 AnatomicalOrientationType
(0010,2292) LO (no value available)                     #   0, 0 PatientBreedDescription
(0010,2293) SQ (Sequence with undefined length #=0)     # u/l, 1 PatientBreedCodeSequence
(fffe,e0dd) na (SequenceDelimitationItem)               #   0, 0 SequenceDelimitationItem
(0010,2294) SQ (Sequence with undefined length #=0)     # u/l, 1 BreedRegistrationSequence
(fffe,e0dd) na (SequenceDelimitationItem)               #   0, 0 SequenceDelimitationItem
(0010,2297) PN (no value available)                     #   0, 0 ResponsiblePerson
(0010,2299) LO (no value available)                     #   0, 0 ResponsibleOrganization
(0010,4000) LT (no value available)                     #   0, 0 PatientComments
(0018,0020) CS [RM\IR]                                  #   6, 2 ScanningSequence
(0018,0021) CS [NONE]                                   #   4, 1 SequenceVariant
(0018,0022) CS (no value available)                     #   0, 0 ScanOptions
(0018,0023) CS [2D]                                     #   2, 1 MRAcquisitionType
(0018,0024) SH [FLASH (pvm)]                            #  12, 1 SequenceName
(0018,0050) DS [2]                                      #   2, 1 SliceThickness
(0018,0080) DS [100]                                    #   4, 1 RepetitionTime
(0018,0081) DS [6]                                      #   2, 1 EchoTime
(0018,0082) DS (no value available)                     #   0, 0 InversionTime
(0018,0083) DS [1]                                      #   2, 1 NumberOfAverages
(0018,0084) DS [188.484]                                #   8, 1 ImagingFrequency
(0018,0085) SH [19F]                                    #   4, 1 ImagedNucleus
(0018,0088) DS [56.5685]                                #   8, 1 SpacingBetweenSlices
(0018,0089) IS [128]                                    #   4, 1 NumberOfPhaseEncodingSteps
(0018,0091) IS [1]                                      #   2, 1 EchoTrainLength
(0018,0095) DS [390.625]                                #   8, 1 PixelBandwidth
(0018,1020) LO [ParaVision5.1]                          #  14, 1 SoftwareVersions
(0018,1030) LO [Protocol_00]                            #  12, 1 ProtocolName
(0018,1310) US 0\128\128\0                              #   8, 4 AcquisitionMatrix
(0018,1312) CS [ROW]                                    #   4, 1 InPlanePhaseEncodingDirection
(0018,1314) DS [30]                                     #   2, 1 FlipAngle
(0018,5100) CS [HFS]                                    #   4, 1 PatientPosition
(0020,000d) UI [2.16.756.5.5.100.3611280983.15764.1343833849.7296] #  50, 1 StudyInstanceUID
(0020,000e) UI [2.16.756.5.5.100.3611280983.22096.1343833916.3] #  46, 1 SeriesInstanceUID
(0020,0010) SH [2]                                      #   2, 1 StudyID
(0020,0011) IS [10001]                                  #   6, 1 SeriesNumber
(0020,0013) IS [2]                                      #   2, 1 InstanceNumber
(0020,0032) DS [0\-40\40]                               #   8, 3 ImagePositionPatient
(0020,0037) DS [0\1\0\0\0\-1]                           #  12, 6 ImageOrientationPatient
(0020,0052) UI [2.16.756.5.5.100.3611280983.22096.1343833916.3] #  46, 1 FrameOfReferenceUID
(0020,0060) CS (no value available)                     #   0, 0 Laterality
(0020,1002) IS [3]                                      #   2, 1 ImagesInAcquisition
(0020,1040) LO (no value available)                     #   0, 0 PositionReferenceIndicator
(0020,4000) LT (no value available)                     #   0, 0 ImageComments
(0028,0002) US 1                                        #   2, 1 SamplesPerPixel
(0028,0004) CS [MONOCHROME2]                            #  12, 1 PhotometricInterpretation
(0028,0010) US 128                                      #   2, 1 Rows
(0028,0011) US 128                                      #   2, 1 Columns
(0028,0030) DS [0.625\0.625]                            #  12, 2 PixelSpacing
(0028,0100) US 16                                       #   2, 1 BitsAllocated
(0028,0101) US 16                                       #   2, 1 BitsStored
(0028,0102) US 15                                       #   2, 1 HighBit
(0028,0103) US 0                                        #   2, 1 PixelRepresentation
(0028,1052) DS [-32768]                                 #   6, 1 RescaleIntercept
(0028,1053) DS [1]                                      #   2, 1 RescaleSlope
(0028,1054) LO [US]                                     #   2, 1 RescaleType
EOF

cat > ${DIRECTORY}/dopamine_test_file_09 << EOF

# Dicom-File-Format

# Dicom-Meta-Information-Header
# Used TransferSyntax: Little Endian Explicit
(0002,0000) UL 198                                      #   4, 1 FileMetaInformationGroupLength
(0002,0001) OB 00\01                                    #   2, 1 FileMetaInformationVersion
(0002,0002) UI =MRImageStorage                          #  26, 1 MediaStorageSOPClassUID
(0002,0003) UI [1.2.276.0.7230010.3.1.4.8323329.7922.1432822445.489114] #  54, 1 MediaStorageSOPInstanceUID
(0002,0010) UI =LittleEndianExplicit                    #  20, 1 TransferSyntaxUID
(0002,0012) UI [1.2.276.0.7230010.3.0.3.6.0]            #  28, 1 ImplementationClassUID
(0002,0013) SH [OFFIS_DCMTK_360]                        #  16, 1 ImplementationVersionName

# Dicom-Data-Set
# Used TransferSyntax: Little Endian Explicit
(0008,0008) CS [ORIGINAL\PRIMARY]                       #  16, 2 ImageType
(0008,0012) DA [20150528]                               #   8, 1 InstanceCreationDate
(0008,0013) TM [161405]                                 #   6, 1 InstanceCreationTime
(0008,0016) UI =MRImageStorage                          #  26, 1 SOPClassUID
(0008,0018) UI [1.2.276.0.7230010.3.1.4.8323329.7922.1432822445.489114] #  54, 1 SOPInstanceUID
(0008,0020) DA [20120801]                               #   8, 1 StudyDate
(0008,0021) DA (no value available)                     #   0, 0 SeriesDate
(0008,0022) DA [20120801]                               #   8, 1 AcquisitionDate
(0008,0030) TM [181049]                                 #   6, 1 StudyTime
(0008,0031) TM (no value available)                     #   0, 0 SeriesTime
(0008,0032) TM [171138]                                 #   6, 1 AcquisitionTime
(0008,0050) SH (no value available)                     #   0, 0 AccessionNumber
(0008,0060) CS [MR]                                     #   2, 1 Modality
(0008,0070) LO [Bruker BioSpin MRI GmbH]                #  24, 1 Manufacturer
(0008,0080) LO [FLI-IAM]                                #   8, 1 InstitutionName
(0008,0090) PN (no value available)                     #   0, 0 ReferringPhysicianName
(0008,1010) SH [BioSpec 47/40]                          #  14, 1 StationName
(0008,1030) LO (no value available)                     #   0, 0 StudyDescription
(0008,103e) LO [Protocol_00]                            #  12, 1 SeriesDescription
(0008,1050) PN [fli_iam]                                #   8, 1 PerformingPhysicianName
(0010,0010) PN [Gonzales^Speedy]                        #  16, 1 PatientName
(0010,0020) LO [subject_03]                             #  10, 1 PatientID
(0010,0030) DA (no value available)                     #   0, 0 PatientBirthDate
(0010,0040) CS [M]                                      #   2, 1 PatientSex
(0010,1030) DS [5]                                      #   2, 1 PatientWeight
(0010,2201) LO [to]                                     #   2, 1 PatientSpeciesDescription
(0010,2203) CS (no value available)                     #   0, 0 PatientSexNeutered
(0010,2210) CS [QUADRUPED]                              #  10, 1 AnatomicalOrientationType
(0010,2292) LO (no value available)                     #   0, 0 PatientBreedDescription
(0010,2293) SQ (Sequence with undefined length #=0)     # u/l, 1 PatientBreedCodeSequence
(fffe,e0dd) na (SequenceDelimitationItem)               #   0, 0 SequenceDelimitationItem
(0010,2294) SQ (Sequence with undefined length #=0)     # u/l, 1 BreedRegistrationSequence
(fffe,e0dd) na (SequenceDelimitationItem)               #   0, 0 SequenceDelimitationItem
(0010,2297) PN (no value available)                     #   0, 0 ResponsiblePerson
(0010,2299) LO (no value available)                     #   0, 0 ResponsibleOrganization
(0010,4000) LT (no value available)                     #   0, 0 PatientComments
(0018,0020) CS [RM\IR]                                  #   6, 2 ScanningSequence
(0018,0021) CS [NONE]                                   #   4, 1 SequenceVariant
(0018,0022) CS (no value available)                     #   0, 0 ScanOptions
(0018,0023) CS [2D]                                     #   2, 1 MRAcquisitionType
(0018,0024) SH [FLASH (pvm)]                            #  12, 1 SequenceName
(0018,0050) DS [2]                                      #   2, 1 SliceThickness
(0018,0080) DS [100]                                    #   4, 1 RepetitionTime
(0018,0081) DS [6]                                      #   2, 1 EchoTime
(0018,0082) DS (no value available)                     #   0, 0 InversionTime
(0018,0083) DS [1]                                      #   2, 1 NumberOfAverages
(0018,0084) DS [188.484]                                #   8, 1 ImagingFrequency
(0018,0085) SH [19F]                                    #   4, 1 ImagedNucleus
(0018,0088) DS [56.5685]                                #   8, 1 SpacingBetweenSlices
(0018,0089) IS [128]                                    #   4, 1 NumberOfPhaseEncodingSteps
(0018,0091) IS [1]                                      #   2, 1 EchoTrainLength
(0018,0095) DS [390.625]                                #   8, 1 PixelBandwidth
(0018,1020) LO [ParaVision5.1]                          #  14, 1 SoftwareVersions
(0018,1030) LO [Protocol_00]                            #  12, 1 ProtocolName
(0018,1310) US 0\128\128\0                              #   8, 4 AcquisitionMatrix
(0018,1312) CS [ROW]                                    #   4, 1 InPlanePhaseEncodingDirection
(0018,1314) DS [30]                                     #   2, 1 FlipAngle
(0018,5100) CS [HFS]                                    #   4, 1 PatientPosition
(0020,000d) UI [2.16.756.5.5.100.3611280983.15764.1343833849.7296] #  50, 1 StudyInstanceUID
(0020,000e) UI [2.16.756.5.5.100.3611280983.22096.1343833916.3] #  46, 1 SeriesInstanceUID
(0020,0010) SH [2]                                      #   2, 1 StudyID
(0020,0011) IS [10001]                                  #   6, 1 SeriesNumber
(0020,0013) IS [3]                                      #   2, 1 InstanceNumber
(0020,0032) DS [-40\0\40]                               #   8, 3 ImagePositionPatient
(0020,0037) DS [1\0\0\0\0\-1]                           #  12, 6 ImageOrientationPatient
(0020,0052) UI [2.16.756.5.5.100.3611280983.22096.1343833916.3] #  46, 1 FrameOfReferenceUID
(0020,0060) CS (no value available)                     #   0, 0 Laterality
(0020,1002) IS [3]                                      #   2, 1 ImagesInAcquisition
(0020,1040) LO (no value available)                     #   0, 0 PositionReferenceIndicator
(0020,4000) LT (no value available)                     #   0, 0 ImageComments
(0028,0002) US 1                                        #   2, 1 SamplesPerPixel
(0028,0004) CS [MONOCHROME2]                            #  12, 1 PhotometricInterpretation
(0028,0010) US 128                                      #   2, 1 Rows
(0028,0011) US 128                                      #   2, 1 Columns
(0028,0030) DS [0.625\0.625]                            #  12, 2 PixelSpacing
(0028,0100) US 16                                       #   2, 1 BitsAllocated
(0028,0101) US 16                                       #   2, 1 BitsStored
(0028,0102) US 15                                       #   2, 1 HighBit
(0028,0103) US 0                                        #   2, 1 PixelRepresentation
(0028,1052) DS [-32768]                                 #   6, 1 RescaleIntercept
(0028,1053) DS [1]                                      #   2, 1 RescaleSlope
(0028,1054) LO [US]                                     #   2, 1 RescaleType
EOF

# Export configuration file path
export DOPAMINE_TEST_CONFIG=${DIRECTORY}/config
export DOPAMINE_TEST_BADCONFIG=${DIRECTORY}/badconfig

# Export DICOM test file path
export DOPAMINE_TEST_DICOMFILE_01=${DIRECTORY}/Patient01_Study01_Series01_Instance01
export DOPAMINE_TEST_DICOMFILE_02=${DIRECTORY}/Patient02_Study01_Series01_Instance01
export DOPAMINE_TEST_DICOMFILE_03=${DIRECTORY}/Patient02_Study01_Series01_Instance02
export DOPAMINE_TEST_DICOMFILE_04=${DIRECTORY}/Patient02_Study01_Series01_Instance03
export DOPAMINE_TEST_DICOMFILE_05=${DIRECTORY}/Patient03_Study01_Series01_Instance01
export DOPAMINE_TEST_DICOMFILE_06=${DIRECTORY}/Patient03_Study02_Series01_Instance01
export DOPAMINE_TEST_DICOMFILE_07=${DIRECTORY}/Patient04_Study01_Series01_Instance01
export DOPAMINE_TEST_DICOMFILE_08=${DIRECTORY}/Patient04_Study01_Series01_Instance02
export DOPAMINE_TEST_DICOMFILE_09=${DIRECTORY}/Patient04_Study01_Series01_Instance03

#Output Directory
export DOPAMINE_TEST_OUTPUTDIR=${DIRECTORY}/output
mkdir ${DOPAMINE_TEST_OUTPUTDIR}

# Create JavaScript to initialize mongo database
cat > ${DIRECTORY}/create_db.js << EOF
db = connect("localhost:27017/${DOPAMINE_TEST_DATABASE}");
j = { "00080018" : { "vr" : "UI", "Value" : [ "2.16.756.5.5.100.3611280983.20092.1364462499.1.0" ] },
      "0020000d" : { "vr" : "UI", "Value" : [ "2.16.756.5.5.100.3611280983.19057.1364461809.9999" ] },
      "0020000e" : { "vr" : "UI", "Value" : [ "2.16.756.5.5.100.3611280983.20092.1364462499.1" ] },
      "Content" : "" }
db.datasets.insert(j)

j = { "00080018" : { "vr" : "UI", "Value" : [ "2.16.756.5.5.100.3611280983.20092.1364462488.1.0" ] },
      "0020000d" : { "vr" : "UI", "Value" : [ "2.16.756.5.5.100.3611280983.19057.1364461809.8888" ] },
      "0020000e" : { "vr" : "UI", "Value" : [ "2.16.756.5.5.100.3611280983.20092.1364462488.1" ] },
      "Content" : "not_find" }
db.datasets.insert(j)
EOF

export DOPAMINE_TEST_ADD_JANE="mongo --quiet ${DIRECTORY}/insert_jane_doe.js"

cat > ${DIRECTORY}/insert_jane_doe.js << EOF
db = connect("localhost:27017/${DOPAMINE_TEST_DATABASE}");
j = { "00080008" : { "vr" : "CS", "Value" : [  "ORIGINAL",  "PRIMARY",  "OTHER" ] },
      "00080012" : { "vr" : "DA", "Value" : [  "20140827" ] },
      "00080013" : { "vr" : "TM", "Value" : [  "103310" ] },
      "00080016" : { "vr" : "UI", "Value" : [  "1.2.840.10008.5.1.4.1.1.4" ] },
      "00080018" : { "vr" : "UI", "Value" : [  "2.16.756.5.5.100.3611280983.20092.1364462458.1.0" ] },
      "00080020" : { "vr" : "DA", "Value" : [  "20130328" ] },
      "00080022" : { "vr" : "DA", "Value" : [  "20130328" ] },
      "00080030" : { "vr" : "TM", "Value" : [  "101009" ] },
      "00080032" : { "vr" : "TM", "Value" : [  "101818" ] },
      "00080050" : { "vr" : "SH", "Value" : null },
      "00080060" : { "vr" : "CS", "Value" : [  "MR" ] },
      "00080070" : { "vr" : "LO", "Value" : [  "Bruker BioSpin MRI GmbH" ] },
      "00080080" : { "vr" : "LO", "Value" : [  "STRASBOURG" ] },
      "00080090" : { "vr" : "PN", "Value" : [  {  "Alphabetic" : "Gregory^House" } ] },
      "00081010" : { "vr" : "SH", "Value" : [  "Station" ] },
      "00100010" : { "vr" : "PN", "Value" : [  {  "Alphabetic" : "Doe^Jane" } ] },
      "00100020" : { "vr" : "LO", "Value" : [  "dopamine_test_01" ] },
      "00100030" : { "vr" : "DA", "Value" : null },
      "00100040" : { "vr" : "CS", "Value" : [  "F" ] },
      "00101030" : { "vr" : "DS", "Value" : [  5 ] },
      "00180020" : { "vr" : "CS", "Value" : [  "RM",  "IR" ] },
      "00180021" : { "vr" : "CS", "Value" : [  "NONE" ] },
      "00180022" : { "vr" : "CS", "Value" : null },
      "00180023" : { "vr" : "CS", "Value" : [  "2D" ] },
      "00180024" : { "vr" : "SH", "Value" : [  "FAIR_EPI (pvm)" ] },
      "00180050" : { "vr" : "DS", "Value" : [  0.8 ] },
      "00180080" : { "vr" : "DS", "Value" : [  18000 ] },
      "00180081" : { "vr" : "DS", "Value" : [  33 ] },
      "00180082" : { "vr" : "DS", "Value" : [  35.37627273 ] },
      "00180083" : { "vr" : "DS", "Value" : [  1 ] },
      "00180084" : { "vr" : "DS", "Value" : [  200.3334861 ] },
      "00180085" : { "vr" : "SH", "Value" : [  "1H" ] },
      "00180088" : { "vr" : "DS", "Value" : [  0.8 ] },
      "00180089" : { "vr" : "IS", "Value" : [  107 ] },
      "00180091" : { "vr" : "IS", "Value" : [  107 ] },
      "00180094" : { "vr" : "DS", "Value" : [  100 ] },
      "00180095" : { "vr" : "DS", "Value" : [  3337.783712 ] },
      "00181020" : { "vr" : "LO", "Value" : [  "ParaVision 5.1" ] },
      "00181030" : { "vr" : "LO", "Value" : [  "Protocol" ] },
      "00181310" : { "vr" : "US", "Value" : [  107,  0,  0,  107 ] },
      "00181312" : { "vr" : "CS", "Value" : [  "COL" ] },
      "00181314" : { "vr" : "DS", "Value" : [  90 ] },
      "00185100" : { "vr" : "CS", "Value" : [  "HFS" ] },
      "0020000d" : { "vr" : "UI", "Value" : [  "2.16.756.5.5.100.3611280983.19057.1364461809.7789" ] },
      "0020000e" : { "vr" : "UI", "Value" : [  "2.16.756.5.5.100.3611280983.20092.1364462458.1" ] },
      "00200010" : { "vr" : "SH", "Value" : [  "Study_id" ] },
      "00200011" : { "vr" : "IS", "Value" : [  196609 ] },
      "00200012" : { "vr" : "IS", "Value" : [  1 ] },
      "00200013" : { "vr" : "IS", "Value" : [  1 ] },
      "00200032" : { "vr" : "DS", "Value" : [  -15,  -15,  -1.6 ] },
      "00200037" : { "vr" : "DS", "Value" : [  1,  6.123031769e-17,  0,  -6.123031769e-17,  1,  0 ] },
      "00200052" : { "vr" : "UI", "Value" : [  "2.16.756.5.5.100.3611280983.20092.1364462458.1.6.15.18" ] },
      "00201002" : { "vr" : "IS", "Value" : [  75 ] },
      "00201040" : { "vr" : "LO", "Value" : null },
      "00201041" : { "vr" : "DS", "Value" : [  -1.6 ] },
      "00280002" : { "vr" : "US", "Value" : [  1 ] },
      "00280004" : { "vr" : "CS", "Value" : [  "MONOCHROME2" ] },
      "00280010" : { "vr" : "US", "Value" : [  128 ] },
      "00280011" : { "vr" : "US", "Value" : [  128 ] },
      "00280030" : { "vr" : "DS", "Value" : [  0.234375,  0.234375 ] },
      "00280100" : { "vr" : "US", "Value" : [  16 ] },
      "00280101" : { "vr" : "US", "Value" : [  16 ] },
      "00280102" : { "vr" : "US", "Value" : [  15 ] },
      "00280103" : { "vr" : "US", "Value" : [  1 ] },
      "00280106" : { "vr" : "US", "Value" : [  3 ] },
      "00280107" : { "vr" : "US", "Value" : [  32766 ] },
      "00281050" : { "vr" : "DS", "Value" : [  16385 ] },
      "00281051" : { "vr" : "DS", "Value" : [  32764 ] },
      "00281055" : { "vr" : "LO", "Value" : [  "MinMax" ] },
      "Content" : new BinData(0,"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAABESUNNAgAAAFVMBADAAAAAAgABAE9CAAACAAAAAAECAAIAVUkaADEuMi44NDAuMTAwMDguNS4xLjQuMS4xLjQAAgADAFVJMAAyLjE2Ljc1Ni41LjUuMTAwLjM2MTEyODA5ODMuMjAwOTIuMTM2NDQ2MjQ1OC4xLjACABAAVUkUADEuMi44NDAuMTAwMDguMS4yLjEAAgASAFVJHAAxLjIuMjc2LjAuNzIzMDAxMC4zLjAuMy42LjAAAgATAFNIEABPRkZJU19EQ01US18zNjAgCAAIAENTFgBPUklHSU5BTFxQUklNQVJZXE9USEVSCAASAERBCAAyMDE0MDgyNwgAEwBUTQYAMTAzMzEwCAAWAFVJGgAxLjIuODQwLjEwMDA4LjUuMS40LjEuMS40AAgAGABVSTAAMi4xNi43NTYuNS41LjEwMC4zNjExMjgwOTgzLjIwMDkyLjEzNjQ0NjI0NTguMS4wCAAgAERBCAAyMDEzMDMyOAgAIgBEQQgAMjAxMzAzMjgIADAAVE0GADEwMTAwOQgAMgBUTQYAMTAxODE4CABQAFNIAAAIAGAAQ1MCAE1SCABwAExPGABCcnVrZXIgQmlvU3BpbiBNUkkgR21iSCAIAIAATE8KAFNUUkFTQk9VUkcIAJAAUE4OAEdyZWdvcnleSG91c2UgCAAQEFNICABTdGF0aW9uIBAAEABQTggARG9lXkphbmUQACAATE8QAGRvcGFtaW5lX3Rlc3RfMDEQADAAREEAABAAQABDUwIARiAQADAQRFMCADUgGAAgAENTBgBSTVxJUiAYACEAQ1MEAE5PTkUYACIAQ1MAABgAIwBDUwIAMkQYACQAU0gOAEZBSVJfRVBJIChwdm0pGABQAERTBAAwLjggGACAAERTBgAxODAwMCAYAIEARFMCADMzGACCAERTDAAzNS4zNzYyNzI3MyAYAIMARFMCADEgGACEAERTDAAyMDAuMzMzNDg2MSAYAIUAU0gCADFIGACIAERTBAAwLjggGACJAElTBAAxMDcgGACRAElTBAAxMDcgGACUAERTBAAxMDAgGACVAERTDAAzMzM3Ljc4MzcxMiAYACAQTE8OAFBhcmFWaXNpb24gNS4xGAAwEExPCABQcm90b2NvbBgAEBNVUwgAawAAAAAAawAYABITQ1MEAENPTCAYABQTRFMCADkwGAAAUUNTBABIRlMgIAANAFVJMgAyLjE2Ljc1Ni41LjUuMTAwLjM2MTEyODA5ODMuMTkwNTcuMTM2NDQ2MTgwOS43Nzg5ACAADgBVSS4AMi4xNi43NTYuNS41LjEwMC4zNjExMjgwOTgzLjIwMDkyLjEzNjQ0NjI0NTguMSAAEABTSAgAU3R1ZHlfaWQgABEASVMGADE5NjYwOSAAEgBJUwIAMSAgABMASVMCADEgIAAyAERTDAAtMTVcLTE1XC0xLjYgADcARFMoADFcNi4xMjMwMzE3NjllLTE3XDBcLTYuMTIzMDMxNzY5ZS0xN1wxXDAgAFIAVUk2ADIuMTYuNzU2LjUuNS4xMDAuMzYxMTI4MDk4My4yMDA5Mi4xMzY0NDYyNDU4LjEuNi4xNS4xOCAAAhBJUwIANzUgAEAQTE8AACAAQRBEUwQALTEuNigAAgBVUwIAAQAoAAQAQ1MMAE1PTk9DSFJPTUUyICgAEABVUwIAgAAoABEAVVMCAIAAKAAwAERTEgAwLjIzNDM3NVwwLjIzNDM3NSAoAAABVVMCABAAKAABAVVTAgAQACgAAgFVUwIADwAoAAMBVVMCAAEAKAAGAVVTAgADACgABwFVUwIA/n8oAFAQRFMGADE2Mzg1ICgAURBEUwYAMzI3NjQgKABVEExPBgBNaW5NYXg=") }
db.datasets.insert(j)
EOF

export DOPAMINE_TEST_DEL_JANE="mongo --quiet ${DIRECTORY}/remove_jane_doe.js"

cat > ${DIRECTORY}/remove_jane_doe.js << EOF
db = connect("localhost:27017/${DOPAMINE_TEST_DATABASE}");
db.datasets.remove( { "00080018.Value" : "2.16.756.5.5.100.3611280983.20092.1364462458.1.0" } )
EOF

export DOPAMINE_TEST_ADD_AUTH="mongo --quiet ${DIRECTORY}/create_authorization.js"

# Create JavaScript to initialize Authorization
cat > ${DIRECTORY}/create_authorization.js << EOF
db = connect("localhost:27017/${DOPAMINE_TEST_DATABASE}");
j = { "principal_name" : "", "principal_type" : "", "service" : "Echo", "dataset" : {} }
db.authorization.insert(j)

j = { "principal_name" : "", "principal_type" : "", "service" : "Query", "dataset" : {} }
db.authorization.insert(j)

j = { "principal_name" : "", "principal_type" : "", "service" : "Retrieve", "dataset" : {} }
db.authorization.insert(j)

j = { "principal_name" : "", "principal_type" : "", "service" : "Store", "dataset" : {} }
db.authorization.insert(j)
EOF

export DOPAMINE_TEST_SPE_AUTH="mongo --quiet ${DIRECTORY}/create_specific_authorization.js"

# Create JavaScript to initialize Authorization
cat > ${DIRECTORY}/create_specific_authorization.js << EOF
db = connect("localhost:27017/${DOPAMINE_TEST_DATABASE}");
db.authorization.drop()

j = { "principal_name" : "", "principal_type" : "", "service" : "Query", "dataset" : { "00100010" : /^Not_Doe_Jane$/ } }
db.authorization.insert(j)

j = { "principal_name" : "", "principal_type" : "", "service" : "Retrieve", "dataset" : { "00100010" : /^Not_Doe_Jane$/ } }
db.authorization.insert(j)

j = { "principal_name" : "", "principal_type" : "", "service" : "Store", "dataset" : { "00100010" : /^Not_Doe_John$/ } }
db.authorization.insert(j)
EOF

export DOPAMINE_TEST_DEL_AUTH="mongo --quiet ${DIRECTORY}/remove_authorization.js"

# Create JavaScript to remove Authorization
cat > ${DIRECTORY}/remove_authorization.js << EOF
db = connect("localhost:27017/${DOPAMINE_TEST_DATABASE}");
db.authorization.drop()
EOF

# Create JavaScript to remove mongo database
cat > ${DIRECTORY}/delete_db.js << EOF
db = connect("localhost:27017/${DOPAMINE_TEST_DATABASE}");
db.dropDatabase()
EOF

# Create Dataset
dump2dcm ${DIRECTORY}/dopamine_test_file_01 "${DOPAMINE_TEST_DICOMFILE_01}"
dump2dcm ${DIRECTORY}/dopamine_test_file_02 "${DOPAMINE_TEST_DICOMFILE_02}"
dump2dcm ${DIRECTORY}/dopamine_test_file_03 "${DOPAMINE_TEST_DICOMFILE_03}"
dump2dcm ${DIRECTORY}/dopamine_test_file_04 "${DOPAMINE_TEST_DICOMFILE_04}"
dump2dcm ${DIRECTORY}/dopamine_test_file_05 "${DOPAMINE_TEST_DICOMFILE_05}"
dump2dcm ${DIRECTORY}/dopamine_test_file_06 "${DOPAMINE_TEST_DICOMFILE_06}"
dump2dcm ${DIRECTORY}/dopamine_test_file_07 "${DOPAMINE_TEST_DICOMFILE_07}"
dump2dcm ${DIRECTORY}/dopamine_test_file_08 "${DOPAMINE_TEST_DICOMFILE_08}"
dump2dcm ${DIRECTORY}/dopamine_test_file_09 "${DOPAMINE_TEST_DICOMFILE_09}"

# Make sure Database is empty
mongo --quiet ${DIRECTORY}/delete_db.js

# Create Database
mongo --quiet ${DIRECTORY}/create_db.js

# Create Authorization
mongo --quiet ${DIRECTORY}/create_authorization.js

# Get data tests
export DOPAMINE_TEST_DATA=${DIRECTORY}/Data
wget -P ${DOPAMINE_TEST_DATA} http://www.dclunie.com/images/charset/charsettests.20070405.tar.bz2
tar -C ${DOPAMINE_TEST_DATA} -xf ${DOPAMINE_TEST_DATA}/charsettests.20070405.tar.bz2

export DOPAMINE_BUILD_DIR=${PWD}

./src/appli/dopamine &

sleep 1

# Execute unit tests
ctest --no-compress-output -T Test || true

termscu localhost ${DOPAMINE_TEST_LISTENINGPORT}

sleep 1

nosetests --with-xunit --xunit-file=${DOPAMINE_BUILD_DIR}/../generatedJUnitFiles/nosetests.xml -w ${DOPAMINE_BUILD_DIR}/../tests/code

# Remove Database
mongo --quiet ${DIRECTORY}/delete_db.js

# Remove all temporary files
rm -rf ${DIRECTORY}
