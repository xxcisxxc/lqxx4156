import {useEffect} from  "react"

function Main() {

    function afterCreateSuccessful(data) {
        var table = document.getElementById("tasklistsTable");
        var row = table.insertRow(0);
        var newCell = row.insertCell();
        var newText = document.createTextNode(data.name);
        newCell.appendChild(newText);
        console.log("successful")
        console.log(data)
    }

    function onCreateError(error) {
        console.log("dealt with error")
        console.log(error)
    }

    function afterGetAllTaskListSuccessful(data) {
        console.log(data)
        const arr = data.data
        if (arr !== null) {
            arr.forEach(element => {
                var table = document.getElementById("tasklistsTable");
                var row = table.insertRow(0);
                var newCell = row.insertCell();
                var newText = document.createTextNode(element);
                newCell.appendChild(newText);
            });
        }
    }

    function onGetAllTaskListError(error) {
        console.log(error)
    }

    
    function createTaskList() {
        let tasklistName = prompt("Please enter the tasklist name:", "MyTasklist");
        console.log(window.localStorage.getItem('token'))
        if (tasklistName != null && tasklistName != "") {
            const requestOptions = {
                method: 'POST',
                headers: {
                    'Accept': '*/*',
                    'Connection': 'keep-alive',
                    'Content-Type': 'application/json',
                    'Authorization': 'Basic ' + btoa(window.localStorage.getItem('token'))
                },
                body: JSON.stringify({
                    "name": tasklistName,
                    "visibility": "private"
                })
            };

            fetch("http://173.199.114.233:3001/v1/task_lists/create", requestOptions)
            .then((response) => response.json())
            .then((data) => afterCreateSuccessful(data))
            .catch((error) => onCreateError(error));
        }
    }

    function deleteTaskList() {
        function inTable(tasklistName) {
            var rows = document.getElementById("tasklistsTable").rows;
            for ( var i = 0; i < rows.length; i++ ) {
                console.log(rows[i])
                var fullname = rows[i].getElementsByTagName("td");
                fullname = fullname[0].innerHTML;
                if (fullname == tasklistName) return true;
            }
            return false;
        }
        let tasklistName = prompt("Please enter the tasklist name:", "MyTasklist");
        if (tasklistName != null && tasklistName != "" && inTable(tasklistName)) {
            const requestOptions = {
                method: 'DELETE',
                headers: {
                    'Accept': '*/*',
                    'Connection': 'keep-alive',
                    'Content-Type': 'application/javascript',
                    'Authorization': 'Basic ' + btoa(window.localStorage.getItem('token'))
                }
            };

            fetch("http://173.199.114.233:3001/v1/task_lists/" + tasklistName, requestOptions)
            .then((response) => response.json())
            .then((data) => afterCreateSuccessful(data))
            .catch((error) => onCreateError(error));
        }
    }

    useEffect(() => {
        // Update the document title using the browser API
        const requestOptions = {
            method: "GET",
            headers: {
              'Accept': '*/*',
              'Connection': 'keep-alive',
              'Content-Type': "application/json",
              'Authorization': 'Basic '+ btoa(window.localStorage.getItem('token')), 
            }
        };
        const fetchData = async () => {
            await fetch("http://173.199.114.233:3001/v1/task_lists", requestOptions)
            .then((response) => response.json())
            .then((data) => afterGetAllTaskListSuccessful(data))
            .catch((error) => onGetAllTaskListError(error));
        }

        fetchData();
        console.log(fetchData);
    });

    console.log("hey")
    return (
        <div>
            <div className="buttons">
                <button onClick={createTaskList}>Create List</button>
                <button onClick={deleteTaskList}>Delete List</button>
            </div>
            <div>
                <table className="tasklists" id="tasklistsTable"></table>
            </div>
        </div>
    )
}

export default Main;