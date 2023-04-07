const express= require("express");
const fsPromises = require('fs').promises;
const fs = require("fs");
const path = require('path');
const ejs = require("ejs");
const bodyParser = require("body-parser");
const cors = require("cors");
const { spawn } = require('node:child_process');

const app = express();


const whitelist = ["http://localhost:3000"];

const corsOption = {
    origin: (origin, callback) => {
        console.log("CORS Origin: " + origin);
        if (whitelist.indexOf(origin) !== -1 || !origin) {
            callback(null, true);
        } else {
            callback(new Error("Not allowed by CORS"));
        }
    },
    optionsSuccessStatus: 200
}

app.use(cors(corsOption));

app.use(bodyParser.urlencoded({extended:true}));
app.use(express.json());
app.set("view engine",  "ejs");



app.get("/", function(req, res){
    res.sendFile(__dirname + "/index.html");
    console.log(__dirname);
    console.log("first")

});

app.post("/pipeline/:btb/:df",async(req,res)=>{
    const btb = req.params.btb;
    const df = req.params.df;
    
    console.log("Pipeline working started")
    //console.log(req.body)
    await fsPromises.writeFile('instruction.mc', req.body.data, function(error){   
        if(error){
            console.log(`error is ${error.message}`);
        }
    })
    
    const ls = async() => {
        if (btb == 0 && df == 0) {console.log("nodf_nobtb\n"); await spawn("cmd.exe", ['/c', 'pipeline_nodf_nobtb.exe']);}
        else if (btb == 1 && df == 0) {console.log("nodf_btb\n"); await spawn("cmd.exe", ['/c', 'pipeline_nodf_btb.exe']);}
        else if (btb == 0 && df == 1) {console.log("df_nobtb\n"); await spawn("cmd.exe", ['/c', 'pipeline_df_nobtb.exe']);}
        else if (btb == 1 && df == 1) {console.log("df_btb\n"); await spawn("cmd.exe", ['/c', 'pipeline_df_btb.exe']);}
    }
    await ls();
    const data = await fsPromises.readFile(path.join(__dirname,"output.txt"),"utf-8");
        
    const newData = await divide(data);
    newData.unshift(data);

    res.send(newData);
});




app.post("/singleCycle",async(req,res)=>{
    console.log("SingleCycle working started")
    //console.log(req.body)
    await fsPromises.writeFile('instruction.mc', req.body.data, function(error){   
        if(error){
            console.log(`error is ${error.message}`);
        }
    })
    
    const ls = async() => await spawn("cmd.exe", ['/c', 'singleCycle.exe']);
    await ls();

    const data = await fsPromises.readFile(path.join(__dirname,"output.txt"),"utf-8");
    // console.log(data);    
    const newData = await divide(data);
    newData.unshift(data);
    
    res.send(newData);
});

const divide = async(data) => {
    const newData = data.split('$');    
    return newData;       
};

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
