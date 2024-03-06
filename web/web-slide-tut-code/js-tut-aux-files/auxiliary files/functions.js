function calculateSquareRoot() {
    var number = document.getElementById("number").value;
    var squareRoot = Math.sqrt(number);
    document.getElementById("result").innerHTML = 
    "The square root of " + number + " is " + squareRoot;
}