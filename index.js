const express= require("express");
const fsPromises = require('fs').promises;
const fs = require("fs");
const path = require('path');
const bodyParser = require("body-parser");
const { spawn } = require('node:child_process');
const app=express();
const ejs = require("ejs");

app.use(bodyParser.urlencoded({extended:true}));

app.set("view engine",  "ejs");



app.get("/", function(req, res){
    res.sendFile(__dirname + "/index.html");
    console.log(__dirname);
    console.log("first")

});


// app.post("/",function(req,res){
//     console.log("working started")
//     fs.writeFile('instruction.mc', req.body.INPUT, function(error){
//         if(error){
//             console.log(`error is ${error.message}`);
//         }

        
//     })

    app.post("/",async(req,res)=>{
        console.log("working started")
        console.log(req.body.input1)
        fs.writeFile('instruction.mc', req.body.input1, function(error){
            
            if(error){
                console.log(`error is ${error.message}`);
            }
    
            
        })
    

    const  ls = spawn("./a.out", [], {shell : true});
    // ls.stdout.on('data', async(data) => {
    // // await fsPromises.writeFile(path.join(__dirname, "output.txt"), data);

    // console.log(`stdout: ${data}`);

        
        const data = await fsPromises.readFile(path.join(__dirname,"output.txt"),"utf-8");
        res.render("output", {
            data:data
        });
        
    
    })




app.listen(8000,function(){
    console.log("server is running at port 8000");
});




// ls.stderr.on('data', (data) => {
//   console.error(`stderr: ${data}`);
// });

// ls.on('close', (code) => {
//   console.log(`child process exited with code ${code}`);
// });








// const defaults = {
//     cwd: undefined,
//     env: 'C:\Windows\System32',
// };
//const child = spawn('g++', ["code.cpp"], defaults, {shell : true});

// ls.error.on('error',function(err) {
//   console.log(`error: ${err}`);
// })
// var ls;


// const express = require("express")
// const bodyParser = require("body-parser");
// const fs = require("fs");
// const fsPromises = require("fs").promises;
// const app = express()
// app.use(bodyParser.urlencoded({extended:true}));

// const port = 3000

// app.get('/', (req, res) => {
//   res.sendFile(__dirname + '/index.html');
// })


// app.post("/",function(req,res){
//     console.log(req.body.INPUT);
    
//     res.send("Thank you!")
// })




//  app.listen(3000, function(){
//     console.log("Server is running on port 3000.");
//  })
