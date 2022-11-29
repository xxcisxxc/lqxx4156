import React, { useState } from "react";
import { useNavigate } from "react-router-dom";


import "./Login.css";

function Login() {
  const navigate = useNavigate();

  // React States
  const [errorMessages, setErrorMessages] = useState({});
  const [isSubmitted, setIsSubmitted] = useState(false);


  const errors = {
    uname: "invalid username",
    pass: "invalid password",
  };

  const handleResponse = (response) => {
    if (response.status === 200) {
        navigate("/Main");
      }
      else {
        // Error
      }
  }

  const handleError = (error) => {
    console.log(error);
  }

  const handleSubmit = (event) => {
    event.preventDefault();

    const formData = event.target.elements;
    const username = formData[0].value
    const password = formData[1].value

    function onLoginSuccessful(response) {
      var json = response.json()
      console.log(response)
      if (response.status == 200) {
        console.log("success")
        return json
      }
    }

    function afterLoginSuccessful(data) {
      console.log(data)
      console.log(data.token)
      window.localStorage.setItem("token", data.token)
      navigate("/Main")
    }

    function onLoginError(error) {
      console.log(error)
    }

    const requestOptions = {
      method: "POST",
      headers: {
        'Accept': '*/*',
        'Connection': 'keep-alive',
        'Content-Type': "application/json",
        'Authorization': 'Basic '+ btoa(username + ":" + password), 
      },
      body: JSON.stringify({}),
    };
    fetch("http://173.199.114.233:3001/v1/users/login", requestOptions)
      .then((response) => onLoginSuccessful(response))
      .then((data) => afterLoginSuccessful(data))
      .catch((error) => onLoginError(error));
  };

  // Generate JSX code for error message
  const renderErrorMessage = (name) =>
    name === errorMessages.name && (
      <div className="error">{errorMessages.message}</div>
    );

  // JSX code for login form
  const renderForm = (
    <div className="form">
      <form onSubmit={handleSubmit}>
        <div className="input-container">
          <label>Username </label>
          <input type="text" name="uname" required />
          {renderErrorMessage("uname")}
        </div>
        <div className="input-container">
          <label>Password </label>
          <input type="password" name="pass" required />
          {renderErrorMessage("pass")}
        </div>
        <div className="button-container">
          <input type="submit" />
        </div>
        <div className="sign-up">
          <label>Don't have an account?</label>
          <a href="/Register">Sign up</a>
        </div>
      </form>
    </div>
  );

  return (
    <div className="app">
      <div className="login-form">
        <div className="title">Sign In</div>
        {renderForm}
      </div>
    </div>
  );
}

export default Login;
