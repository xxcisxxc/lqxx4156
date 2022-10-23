# lqxx4156
project for coms w4156

Members:
| Name         | UNI    |
| ------------ | ------ |
| Xincheng Xie | xx2365 |
| Shichen Xu   | sx2314 |
| Jiting Liu   | jl6247 |
| Chu Qin      | cq2238 |

Group Name: lqxx

Language & Platform: C++ & Linux

Github: https://github.com/xxcisxxc/lqxx4156

## Compile & Run
Pull lastest version of code, ensure docker and docker-compose is properly installed,
go to project root directory.

```
cd deploy

# to run the main backend service
docker-compose run backend

# to run the tests
# modify the commands of the test service in 'docker-compose.yml' to run your own unit test
docker-compose run backend_test

# to create an interactive bash environment
# by doing so you can run whatever commands you want in the docker container
docker-compose run bash

# after run clean-ups
# by doing so you can remove stopped containers and used resources
docker-compose down
```