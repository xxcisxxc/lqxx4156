import React, { useState } from "react";
import { useNavigate } from "react-router-dom";

import "./Register.css";

function Register() {
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
        navigate("/Login");
      }
      else {
        // navigate("/Something")
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


    function onRegisterSuccessful(response) {
      console.log(response.json())
      if (response.status == 200) {
        navigate("/Login")
        console.log("successful")
      }
    }
  
    function onRegisterError(error) {
      console.log("error")
      console.log(error.json())
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
    fetch("http://173.199.114.233:3001/v1/users/register", requestOptions)
      .then((response) => onRegisterSuccessful(response))
      .catch((error) => onRegisterError(error));
    };

  // Not removing this for now, can be used for fomatting check later
  const renderErrorMessage = (name) =>
    name === errorMessages.name && (
      <div className="error">{errorMessages.message}</div>
    );

  // JSX code for login form
  const renderForm = (
    <div>
      <div className="form">
      <form onSubmit={handleSubmit}>
        <div className="input-container">
          <label>Username </label>
          <input type="text" name="uname" required />
          {renderErrorMessage("uname")}
        </div>
        <div className="input-container">
          <label>Password </label>
          <input type="password" name="pw" required />
          {renderErrorMessage("pw")}
        </div>
        <div className="button-container">
          <input type="submit" />
        </div>
      </form>
      </div>
    </div>
  );

  return (
    <div className="app">
      <div className="register-form">
        <div className="title">Sign Up</div>
        {renderForm}
      </div>
    </div>
  );
}

export default Register;
