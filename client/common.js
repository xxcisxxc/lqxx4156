async function logOut(err_out) {
  const requestOptions = {
    method: "POST",
    headers: {
    'Accept': 'application/json',
    'Authorization': 'Basic '+ btoa(window.localStorage.getItem('token')), 
    }
  };
  const response = await fetch("https://lqxx4156.tk/v1/users/logout", requestOptions);
  const data = await response.json();
  if (response.status == 200) {
    window.location.href = "index.html";
  } else {
    document.getElementById(err_out).style.color = "red";
    document.getElementById(err_out).innerHTML = "Error: " + JSON.stringify(data);
    setTimeout(function() {
      window.location.href = "index.html";
    }, 1500);
  }
}

function ret2page(page) {
  window.location.href = page;
}