import React, { useState } from "react";
import ReactDOM from "react-dom";
import { useNavigate, useHistory } from "react-router-dom";

import "./Homepage.css"

function Homepage() {
  const navigate = useNavigate();

  function onButtonClick() {
    navigate('/Login')
  }
  
    return (
      <div className="welcome">
        <p>Welcome to our website!</p>
        <p>Your personal TODO list manager.</p>
        <button onClick={onButtonClick} className="goButton">
          Let's Go!
        </button>
      </div>
    );
}

export default Homepage;
