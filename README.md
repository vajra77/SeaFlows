# SeaFlows
A simple suite to capture, store and retrieve sFlow data from network devices.

## Operations
The suite is composed of backend which provides collection and parsing of
sFlow datagrams that are then stored as traffic flows into RRD (Round-Robin Database) files.
The frontend provides a RESTful API to query stored flows data, currently the Python API is fully
functional, while Go implementation is **experimental** and not working.

### Backend
Backend is a multi-threaded application (*seaflows*), each thread listens on a separate
UDP port and independently stores decoded flow data into a subset of RRD files.

### Frontend
Both frontend implementations (Python and Go) should provide the same
set of API primitives

## Install

### Backend

### Frontend

#### Python


## Setup
