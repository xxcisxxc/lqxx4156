# Meeting document for 22-10-10

## HTTP interfaces definition

### Users

POST /v1/users/register

- body
```
{
    "name": "Alice",
    "email": "alice@columbia.edu",
    "passwd": "123456"
}
```
- return
```
{
    "msg": "success"
}
```

POST /v1/users/login
- body
```
{
    "email": "alice@columbia.edu",
    "passwd": "123456"
}
```

- return
```
{
    "msg" "success",
    "token": "fjkvajkvh4iugh2789erofhb"
}
```

POST /v1/users/logout
- auth: token

### Task Lists

GET /v1/task_lists/{task_list_name}

(TODO)
```
GET /v1/task_lists/

```

POST /v1/task_lists/create

- body
```
{
    "name": "task_list1"
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

- return
```
{
    "msg": "success"
}
```

### Tasks

GET /v1/task_lists/{task_list_name}/tasks/{task_name}

Same as above.