[![Lint Checkings (clang-format & cppcheck)](https://github.com/xxcisxxc/lqxx4156/actions/workflows/lint-check.yml/badge.svg)](https://github.com/xxcisxxc/lqxx4156/actions/workflows/lint-check.yml)
[![Unit Testing](https://github.com/xxcisxxc/lqxx4156/actions/workflows/unit-test.yml/badge.svg)](https://github.com/xxcisxxc/lqxx4156/actions/workflows/unit-test.yml)
[![Integration Testing](https://github.com/xxcisxxc/lqxx4156/actions/workflows/integration-test.yml/badge.svg)](https://github.com/xxcisxxc/lqxx4156/actions/workflows/integration-test.yml)
[![System Testing](https://github.com/xxcisxxc/lqxx4156/actions/workflows/system-test.yml/badge.svg)](https://github.com/xxcisxxc/lqxx4156/actions/workflows/system-test.yml)
[![Coverage Badge](https://img.shields.io/endpoint?url=https://gist.githubusercontent.com/xxcisxxc/565ca3bce573af66c61e5e3cb5429264/raw/coverage.json)](https://gist.githubusercontent.com/xxcisxxc/565ca3bce573af66c61e5e3cb5429264)

# Task Management Service

Project for COMS W4156

**Members**:

| Name         | UNI    |
| ------------ | ------ |
| Xincheng Xie | xx2365 |
| Shichen Xu   | sx2314 |
| Jiting Liu   | jl6247 |
| Chu Qin      | cq2238 |

**Group Name**: lqxx

*Language & Platform*: C++ & Ubuntu 20.04 LTS / 22.04 LTS

*Github*: https://github.com/xxcisxxc/lqxx4156

## Description

Our service is used for task management. Users can create their own task lists, add tasks into lists,
and view and manage the tasks. Multiple users can have their own task lists. Different users can share their own task lists so that multiple
users can cooperate on one task lists.

## Install our Service

```bash
# Step 1: Install Environment
./build.sh install

# Step 2 (optional): Run all the tests
./build.sh test

# Step 3: Compile
./build.sh build

# Step 4: Run service in the background
./build.sh run
```

## Our Service URL

https://lqxx4156.tk/
(Inaccessible after 12/19/2022)

## Visit our Sample Client

https://xxcisxxc.github.io/client/
(Inaccessible after 12/19/2022)

## Service RESTful API

### Swagger

https://app.swaggerhub.com/apis-docs/lqxx4156/Task-Management/v1

## Various Checkings

See badges at the top. Note that lint checking includes format checking and static analysis.

### Format checking and Static Analysis

We use `clang-format` for format checking and `cppcheck` for static analysis. For the result, please see the lint-checking badge.

### Coverage

We use `gcov` for coverage. Please see the report in Coverage in github actions.

## Third-Party

Please refer to `.gitmodules` to get their file positions and source.

## Some Interesting Notes

### Database

We use graph database neo4j as backend. However, trying to connect to database is really annoying. The widely used [neo4j C driver](https://github.com/cleishm/libneo4j-client) totally does not work for the new version neo4j. Here we use [this library](https://github.com/majensen/libneo4j-client), which tries to merge into that driver (not yet) and supports part of neo4j connection protocol.

### Support SSL

At first, we just use IP address to connect our service. We start to think how to support SSL because when deploying our service, secure link is unable to send query to insecure server.

First, you need to [register a domain](https://my.freenom.com/) and add your server IP address to the list.

Then, you need to get a certificate, we use this [link](https://certbot.eff.org/instructions?ws=other&os=ubuntufocal)

Finally, SSL usually supports by your library and you will need the private keys you just generated. Here is an [simple example](https://gist.github.com/xxcisxxc/26c217e59a110a8a9ee6ddcf6ba63c34#file-runserver-ssl-py).