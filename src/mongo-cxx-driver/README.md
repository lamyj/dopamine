# MongoDB C++ Driver [![Build Status](https://travis-ci.org/mongodb/mongo-cxx-driver.svg?branch=legacy)](https://travis-ci.org/mongodb/mongo-cxx-driver)
Welcome to the MongoDB C++ Driver!

Please see our [wiki page](https://github.com/mongodb/mongo-cxx-driver/wiki/Download-and-Compile-the-Legacy-Driver) for information about building, testing, and using the driver.

  Version [1.1.1](https://github.com/mongodb/mongo-cxx-driver/releases/tag/legacy-1.1.1) of the
  C++ legacy driver has been released. Please report any bugs or issues in the C++
  [JIRA project](http://jira.mongodb.org/browse/CXX).

You should only use the "legacy" branch if you had been previously using
the "26compat" branch (or the driver inside of the server source) and want to
benefit from incremental improvements while having the same overall
API.

> **Note:** As of MongoDB 2.6.0-rc1, it is no longer possible to build the driver from the server sources: this repository is the only approved source for C++ driver builds.

## Repository Overview

| Branch   | Stability   | Development       | Purpose                                                      |
| -------- | ------------| ----------------- | -----------------------------------------------------        |
| master   | Stable      | Stable Evolution  | New C++11 driver                                             |
| legacy   | Stable      | Stable Evolution  | Primary stable C++ driver release                            |
| 26compat | Stable      | Maintenance Only  | Drop in replacement for users of existing 2.6 era C++ driver |

Please note that stable branches are only production quality at stable release tags. Other
commits or pre-release tags on a stable branch represent ongoing development work towards the
next stable release, and therefore may be unstable.

## Components

  - `libmongoclient.[so|dylib|dll]` - The shared mongoclient library (but see notes)
  - `libmongoclient.a` - The static mongoclient library

## Building and Usage

 - [Download and Compile](https://github.com/mongodb/mongo-cxx-driver/wiki/Download%20and%20Compile)
 - [Tutorial](https://github.com/mongodb/mongo-cxx-driver/wiki/Tutorial)

## Bugs and Issues

  See http://jira.mongodb.org/browse/CXX

## Notes

  Use of the shared library is experimental on windows and is currently
  discouraged. This is primarily due to the complexity of ensuring a matching
  implementation of STL types between library and consumer code. This problem
  is unique to windows, as the consistent use of system libraries largely
  mitigates this danger.

## Documentation

  http://docs.mongodb.org/ecosystem/drivers/cpp/

## Mailing Lists and IRC

  http://dochub.mongodb.org/core/community

## License

  The source files in this repository are made available under the terms of the
  Apache License, version 2.0.
