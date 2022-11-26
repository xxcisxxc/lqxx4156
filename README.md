[![Lint Checkings (clang-format & cppcheck)](https://github.com/xxcisxxc/lqxx4156/actions/workflows/lint-check.yml/badge.svg)](https://github.com/xxcisxxc/lqxx4156/actions/workflows/lint-check.yml)
[![Unit Testing](https://github.com/xxcisxxc/lqxx4156/actions/workflows/unit-test.yml/badge.svg)](https://github.com/xxcisxxc/lqxx4156/actions/workflows/unit-test.yml)
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

## Format check

![format-check](/documents/format-check.png)

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
## Test

```
mkdir build && cd build && cmake .. && make
docker run -d -p7474:7474 -p7687:7687 -e NEO4J_AUTH=neo4j/hello4156 neo4j
sleep 60
./tasklists/test_tasklists && ./tasks/test_tasks && ./users/test_users && ./api/test_api && ./db/DBTest 
```

## Third-Party

### Position

```
external/cpp-jwt (v1.4-16-g4b66cf7)
external/googletest (release-1.8.0-3078-gd9bb8412)
external/json (v3.11.2-15-ga3e6e26d)
external/liboauthcpp (heads/master)
external/httplib.h
```

### Source

```
https://github.com/nlohmann/json.git
https://github.com/arun11299/cpp-jwt.git
https://github.com/google/googletest.git
https://github.com/sirikata/liboauthcpp.git
https://github.com/yhirose/cpp-httplib
https://github.com/majensen/libneo4j-client
```

## RESTful API definition

### Http returned status code definition
- 200 for all successful requests
- 500 for all failed requests
- 4XX for wrong paths or unknown resources

### Users

POST /v1/users/register

Register a new user in our system. User should put his or her email and password in the basic auth field of the http request. A name in the request body is optional. The email will be the only method of identification. The email must be valid and can not be duplicated.
- basic auth:

```
{${email}:${password}}
```
- body
```
{
    "name": "Alice"
}
```
- return
If the registration operation is success, a success message will be included in the returned response body. If the registration was unsuccessful, relative message will be shown.
If 
```
{
    "msg": "success"
}
```

- status code: 200 on success, 500 for all failed situations.

POST /v1/users/login

Login for a registered user. The user should include his or her email and password in the basic auth field. A message and token will be returned if successful. The user can then use this token to access our services by including the token in the basic auth field of his or her request.
- basic auth:
```
{${email}:${password}}
```

- return
```
{
    "msg" "success",
    "token": "fjkvajkvh4iugh2789erofhb"
}
```

- status code: 200 on success, 500 for all failed situations.

### Task Lists

GET /v1/task_lists/{task_list_name}?other=xxx@gmail.com

Get the information of one certain task lists. If a user is provided, try to get the {task_list_name} from the corresponding user. Token should be included in the request. Return failed message if the token is invalid or the task list does not exist.

- basic auth:

```
{${token}:}
```

- return
```
{
    "msg" "success",
    "data":
    {
        "name": "some name",
        "content": "some content",
        "date": "some date"
    }
}
```

POST /v1/task_lists/{task_list_name}?other=xxx@gmail.com

Update one task list. If a user if provided, try to update the {task_list_name} from the corresponding user. The name of the task list can not be modified.

- basic auth:

```
{${token}:}
```

- body
```
{
    "content": "some content"
    "date": "some date"
}
```

- return
```
{
    "msg": "success"
}
```

GET /v1/task_lists/?share=false

Get the names of all the task lists that are owned by the user. Token should be included in the request.

- basic auth:

```
{${token}:}

```
- return
```
{
    "msg" "success",
    "data":
    [
        "name1",
        "name2",
        "name3"
    ]
}
```

GET v1/task_lists/?share=true

Get the information of all the task lists that are shared from other users to the user. Token should be included in the request.

- basic auth:

```
{${token}:}

```

- return
```
{
	"data":[
		{
			"user": "a@a.com",
			"list": "list0",
			"permission": "write"
		},
		{
			"user": "a@b.com",
			"list": "list1",
			"permission": "read"
		}
	]
}
```

POST /v1/task_lists/create

Create a new task list for a user.  Token should be included in the request. Task list name should be included in the request body. If the name is duplicated, a number suffix will be automatically added.

- basic auth:

```
{${token}:}
```

- body
```
{
    "name": "task_list1",
    "content": "some content",
    "date": "some date"
}
```

- return
```
{
    "msg": "success",
    "name" "task_list1(1)"
}
```

DEL /v1/task_lists/{task_list_name}

Delete a task list.

- basic auth:

```
{${token}:}
```

- return
```
{
    "msg": "success"
}
```

### Tasks

GET /v1/task_lists/{task_list_name}/tasks/{task_name}?other=xxx.gmail.com

Get the information of one certain task. If a user is provided, try to get the task info from the specified tasklist of the provided user. Token should be included in the request. The name of the task list should be specified in the path. Return failed message if the token is invalid or the task does not exist.

- basic auth:

```
{${token}:}
```

- return
```
{
    "msg" "success",
    "data":
    {
        "name": "some name",
        "content": "some content",
        "date": "some date"
    }
}
```

GET /v1/task_lists/{task_list_name}/tasks?other=xxx@gmail.com

Get the names of all the tasks in a certain task list. If a user is provided, try to get the tasks from the specified tasklist of the provided user. Token should be included in the request.

- basic auth:

```
{${token}:}
```

- return
```
{
    "msg" "success",
    "data":
    [
        "name1",
        "name2",
        "name3"
    ]
}
```

POST /v1/task_lists/{task_list_name}/tasks/create?other=xxx@gmail.com

Create a new task for a user and a task list. If `other` is provided, try to add task to the specified tasklist of the provided user. Task list name should be included in the path.  Task name should be included in the request body. If the name is duplicated, a number suffix will be automatically added. Token should be included in the request. 

- basic auth:

```
{${token}:}
```

- body
```
{
    "name": "task"
}
```

- return
```
{
    "msg": "success",
    "name" "task(1)"
}
```

POST /v1/task_lists/{task_list_name}/tasks/{task_name}?other=xxx@gmail.com

Update one task. If `other` is provided, try to update the task for the specified tasklist of the provided user. The name of the task can not be modified. Token should be included in the request. 

- basic auth:

```
{${token}:}
```

- body
```
{
    "content": "some content"
    "date": "some date"
}
```

- return
```
{
    "msg": "success"
}
```

DEL /v1/task_lists/{task_list_name}/tasks/{task_name}?other=xxx@gmail.com

Delete a task. If `other` is provided, try to delete the task from the specified tasklist of the provided user. Token should be included in the request. 

- basic auth:

```
{${token}:}
```

- return
```
{
    "msg": "success"
}
```


POST v1/share/{task_list_name}/create

Create/Update sharing status for a specific tasklist, ie share the tasklist to others with provided permissions. Token should be included in the request.

- basic auth:

```
{${token}:}
```

- body
```
{
    "user_permission": 
    [
        {
            "user": "a",
            "permission": true
        },
        {
            "user": "b",
            "permission": false
        },
        {
            "user": "c",
            "permission": true
        }
    ]
}
```

- return
```
{
	"msg": "success"
}
```


DEL v1/share/{task_list_name}

Delete sharing status for a tasklist, ie stop sharing of tasklist to the provided users. Token should be included in the request.

- basic auth:

```
{${token}:}
```

- body
```
{
    "user_list": [email1, email2, email3 ...]
}
```

- return
```
{
	"msg": "success"
}
```


GET v1/share/{task_list_name}

Get the sharing status of a specific tasklist, ie check a list of users with whom the tasklist is shared, and the corresponding permission they have. Token should be included in the request.

- basic auth:

```
{${token}:}
```

- body
```
{
	"data" : [
        {
            "user": "a",
            "permission": "read"
        },
        {
            "user": "b",
            "permission": "write"
        },
        {
            "user": "c",
            "permission": "read"
        }
    ]
}
```

- return
```
{
	"msg": "success"
}
```
