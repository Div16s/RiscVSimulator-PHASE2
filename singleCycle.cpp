#include<bits/stdc++.h>
using namespace std;

#define M 32


//Global declration
//for storing values of registers
map<int,int>registerFile;

int lastBit=0;

//main programme memory of the program
map<int,int>main_mem;
// program counter
int pc=0,nextPc,branchPc;  // global declration of programme counter 

string opcode;            //stores opcode of the running programe
int isBranchTaken=0;     //variable to identify branch taken or not
char instr_type;         //stores the instruction type of the running instruction
bitset<32> fetchResult; // stores the fetch result in binary;
int  rs1, rs2, rd,immediate,func3,func7,aluop_result,op1,op2;
string  alu_op;        // variable for determining alu operation type

//takes integer input of opcode and return the instruction type R,S U J B etc;
map<string,char> pair_of_instr_type_opcode;

// instruction memry is stored as integer string
map<int,string> instructrion_memory_map;
string hex2bin(string);   //function to convert hexadecimal to binary
string dec2hex();        //function to convert decimal to hexadecimal
void RiscVSimulator();   //function to run risc-v simulator
void fetch();
void decode();
void execute();
void memory();
void write_back();


int clk=1;

int main(){

    freopen("output.txt","w",stdout); 

    //making pair of opcode and instruction type ;
    pair_of_instr_type_opcode["0110011"]='R';
    pair_of_instr_type_opcode["0010011"]='I';
    pair_of_instr_type_opcode["0000011"]='I';
    pair_of_instr_type_opcode["0100011"]='S';
    pair_of_instr_type_opcode["1100011"]='B';
    pair_of_instr_type_opcode["1101111"]='J';
    pair_of_instr_type_opcode["1100111"]='I';
    pair_of_instr_type_opcode["0110111"]='U';
    pair_of_instr_type_opcode["0010111"]='U';


    // initialisation of register file 
    for(int i=0;i<32;i++){
        registerFile[i]=0;
    }

    registerFile[2]=2147483632;
    for(int i=0;i<1e7;i++){
        main_mem[i]=0;
    }
    // main_mem[100]=3;

    //taking instruction from mc dumb file

    ifstream myfile;
    
    myfile.open("instruction.mc");
    ofstream outfile;
    outfile.open("instruction_copy.mc");
    int counter_next_instruction=0;//counter 

    if(myfile.is_open()){
    while(myfile){
        string temp_instruction;
        myfile>>temp_instruction;

        // stores instruction for execution in map stl
        instructrion_memory_map[counter_next_instruction]=temp_instruction;
        
        //makes the instrction of the format "0x\d   xxxxxxxx"
        if(outfile.is_open() && temp_instruction!=""){
            outfile<<"0x"<<counter_next_instruction<<" "<<temp_instruction<<endl;
            counter_next_instruction+=4;
        }else if(outfile.is_open()){
            instructrion_memory_map[counter_next_instruction]="-1";
        }
        else {
            cout<<"Error in file2 opening"<<endl;
        }
        
    }
    }else{
        cout<<"Error in file opening"<<endl;
    }




    // Risc V simulation begins()
    int counter =1010;
    while (counter--)
    {
        RiscVSimulator();
        if(instructrion_memory_map[pc]=="-1"){
            exit(-1);
        }
    }
    
}

//function for finding funct3
int find_func3(){
    int func3=0,mul=1;
    for(int i=12;i<=14;i++){

        if(fetchResult[i]!=0){
            func3=func3+mul;
        }
        mul*=2;
    }
    // cout<<"func3 : "<<func3;
    return func3;
}
//evaluates func7 value;
int find_func7(){
    int func7=0,mul=1;
    for(int i=25;i<=31;i++){

        if(fetchResult[i]!=0){
            func7=func7+mul;
        }
        mul*=2;
    }
    // cout<<"func7 : "<<func7;
    return func7;
}

//evaluates opcode
string find_opcode(){
    string opcode="";
    for(int i=6;i>=0;i--){
        if(fetchResult[i]==0){
            opcode+='0';
        }else{
           opcode+='1';
        }  
    }
    // cout<<"OPcode : "<<opcode<<" ";
    return opcode;
}

//evaluates source register 1
int find_rs1(){
    int rs1=0, mul=1;
    for(int i=15;i<=19;i++){
        if(fetchResult[i]!=0){
            rs1=rs1+mul;
        }
        mul*=2;
    }
    
    // cout<<"rs1 : "<<rs1;
    return rs1;
}

//evaluates source register 2
int find_rs2(){
    int rs2=0, mul=1;
    for(int i=20;i<=24;i++){
        if(fetchResult[i]!=0){
            rs2=rs2+mul;
        }
        mul*=2;
    }
    // cout<<"rs2 : "<<rs2;
    return rs2;
}

//evaluates destination register
int find_rd(){
    int rd=0, mul=1;
    for(int i=7;i<=11;i++){
        if(fetchResult[i]!=0){
            rd=rd+mul;
        }
        mul*=2;
    }
    // cout<<"rd : "<<rd;
    return rd;
}

//evaluates immediate
int find_immed(char instruction_type){
    int immed=0,mul=1;
    switch (instruction_type) {
        case 'R':
            break;

        case 'I':
            for(int i=20;i<=31;i++){
                if(fetchResult[i]!=0){
                    immed=immed+mul;
                }
                mul*=2;
            }
            if(fetchResult[31]==1){
                immed-=4096;
            }
            break;

        case 'S':
            for(int i=7;i<=11;i++){
                if(fetchResult[i]!=0){
                    immed=immed+mul;
                }
                mul*=2;
            }
            for(int i=25;i<=31;i++){
                if(fetchResult[i]!=0){
                    immed=immed+mul;    
                }
                mul*=2;
            }
            if(fetchResult[31]==1){
                immed-=4096;
            }
            break;
     
        case 'B':
            for(int i=8;i<=11;i++){
                if(fetchResult[i]!=0){
                    immed=immed+mul;
                }
                mul*=2;
            }
            for(int i=25;i<=30;i++){
                if(fetchResult[i]!=0){
                    immed=immed+mul;    
                }
                mul*=2;
            }
            for(int i=7;i<=7;i++){
                if(fetchResult[i]!=0){
                    immed=immed+mul;    
                }
                mul*=2;
            }
            for(int i=31;i<=31;i++){
                if(fetchResult[i]!=0){
                    immed=immed+mul;    
                }
                mul*=2;
            }
            immed<<=1;
            if(fetchResult[31]==1){
                immed-=4096*2;
            }
            break;
        
        case 'U':
            for(int i=12;i<=31;i++){
                if(fetchResult[i]!=0){
                    immed=immed+mul;
                }
                mul*=2;
            }
            if(fetchResult[31]==1){
                immed-=1048576;
            }
            break;
    
        case 'J':
            if (opcode=="1101111"){
                for(int i=21;i<=30;i++){
                    if(fetchResult[i]!=0){
                        immed=immed+mul;
                    }
                    mul*=2;
                }
                for(int i=20;i<=20;i++){
                    if(fetchResult[i]!=0){
                        immed=immed+mul;
                    }
                    mul*=2;
                }
                for(int i=12;i<=19;i++){
                    if(fetchResult[i]!=0){
                        immed=immed+mul;
                    }
                    mul*=2;
                }
                for(int i=31;i<=31;i++){
                    if(fetchResult[i]!=0){
                        immed=immed+mul;
                    }
                    mul*=2;
                }
                //left shift to ensure - 0th bit is aligned;
                immed<<=1;
                // cout<<"Immediat: "<<immed<<endl;
                if(fetchResult[31]==1){
                    immed-=1048576*2;
                }
            
            } else {
                cout<<"error in Jal instruction";
            }

            break;
    
        default:
            break;
    }
    // cout<<"opcode is "<< opcode;
    // cout<<"Immediate is of type  "<<instruction_type<<"the value is : "<<immed;
    return immed;
}

//hex 2 binary 
string hex2bin(string s){
     string binary_num = "";

    for (int i = 0; i < s.length(); i++) {
        switch (s[i]) {
            case '0': binary_num += "0000"; break;
            case '1': binary_num += "0001"; break;
            case '2': binary_num += "0010"; break;
            case '3': binary_num += "0011"; break;
            case '4': binary_num += "0100"; break;
            case '5': binary_num += "0101"; break;
            case '6': binary_num += "0110"; break;
            case '7': binary_num += "0111"; break;
            case '8': binary_num += "1000"; break;
            case '9': binary_num += "1001"; break;
            case 'A': case 'a': binary_num += "1010"; break;
            case 'B': case 'b': binary_num += "1011"; break;
            case 'C': case 'c': binary_num += "1100"; break;
            case 'D': case 'd': binary_num += "1101"; break;
            case 'E': case 'e': binary_num += "1110"; break;
            case 'F': case 'f': binary_num += "1111"; break;
            default:
                return "";
        }
    }
    return binary_num;

}

//function to run single phase risc-v simulator
void RiscVSimulator() {
    fetch(); clk++;
    decode(); clk++;
    execute(); clk++;
    memory(); clk++;
    write_back(); clk++;
};



void memory(){
    if(instr_type=='I' && opcode=="0000011") {
        //at this point aluop_result will carry the location of the memory
        aluop_result = main_mem[aluop_result];
        cout<<"MEMORY: "<<alu_op<<" operation is performed in the memory, loaded value ( in op2): "<<aluop_result<<endl;
    } else if(instr_type=='S') {
        //at this point aluop_result will carry the location of the memory
        main_mem[aluop_result]=op2;
        cout<<"MEMORY: "<<alu_op<<" operation is performed in the memory, stored value (of op2): "<<op2<<endl;
    } else {
        cout<<"MEMORY: No memory operation to perform.\n";
    }
    
    // for(int i=0;i<=51;i++){
    //     cout<<main_mem[4*i-1]<<" ";
    // }
}

void write_back(){
    if(instr_type!='S' && instr_type!='B' && rd!=0) {
        registerFile[rd]=aluop_result;
        cout<<"WRITEBACK: writing "<<aluop_result<<" to "<<"Register R"<<find_rd()<<endl;
        //cout<<endl;
    } else {
        cout<<"WRITEBACK: No write back! "<<endl;
        //cout<<endl;
    }
    cout<<"%";
    for (int i=0; i<32; i++) {
        cout<<"x"<<i<<":"<<registerFile[i]<<"%";
    }
    cout<<"clk:"<<clk<<"%";
    cout<<"\n$\n";
}

void execute(){
    if(alu_op=="add"){
        aluop_result = op1+op2;
    }
    else if(alu_op=="sub"){
        aluop_result = op1-op2;
    }
    else if(alu_op=="xor"){
        aluop_result = op1^op2;
    }
    else if(alu_op=="or"){
        aluop_result = op1|op2;
    }
    else if(alu_op=="and"){
        aluop_result = op1&op2;
    }
    else if(alu_op=="sll"){
        aluop_result = op1<<op2;
    }
    else if(alu_op=="srl"){
        aluop_result = op1>>op2;
    }
    else if(alu_op=="sra"){
        aluop_result = op1>>op2;
    }
    else if(alu_op=="slt"){
        aluop_result = (rs1 < rs2)?1:0;
    }


    //immediate starts
    else if(alu_op=="addi"){
        aluop_result = op1+immediate;
    }
    else if(alu_op=="subi"){
        aluop_result = op1-immediate;
    }
    else if(alu_op=="xori"){
        aluop_result = op1^immediate;
    }
    else if(alu_op=="ori"){
        aluop_result = op1|immediate;
    }
    else if(alu_op=="andi"){
        aluop_result = op1&immediate;
    }
    else if(alu_op=="slli"){
        aluop_result = op1<<immediate;
    }
    else if(alu_op=="srli"){
        aluop_result = op1>>immediate;
    }
    else if(alu_op=="srai"){
        aluop_result = op1>>immediate;
    }
    else if(alu_op=="slti"){
        aluop_result = (rs1 <immediate)?1:0;
    }
    else if (alu_op=="jalr"){
        isBranchTaken=1;
        branchPc = op1+immediate;
        aluop_result=pc+4;
    }

    // load instructions execution
    else if(alu_op=="load_byte"){
        int newImmed= (~((~0)<<8))&immediate;
        aluop_result = op1 + newImmed;

    }
    else if(alu_op=="load_half"){
        int newImmed= (~((~0)<<16))&immediate;
        aluop_result = op1 + newImmed;

    }
    else if(alu_op=="load_word"){
        aluop_result = op1 + immediate;
        cout<<" aluop_result "<<aluop_result<<op1<<" "<<immediate<<endl;

    }


    // store instructions execution
    else if(alu_op=="store_byte"){
        int newImmed= (~((~0)<<8))&immediate;
        aluop_result = op1 + newImmed;

    }
    else if(alu_op=="store_half"){
        int newImmed= (~((~0)<<16))&immediate;
        aluop_result = op1 + newImmed;

    }
    else if(alu_op=="store_word"){
        aluop_result = op1 + immediate;

    }

    //branch starts
    else if(alu_op=="beq"){
        if(op1==op2){
            isBranchTaken=1;
        }
        branchPc = pc+immediate;
    }
    else if(alu_op=="bne"){
        if(op1!=op2){
            isBranchTaken=1;
        }
        branchPc = pc+immediate;
    }
    else if(alu_op=="blt"){
        if(op1<op2){
            isBranchTaken=1;
        }
        branchPc = pc+immediate;
    }
    else if(alu_op=="bge"){
        cout<<"op1 is "<<op1<<" "<<"op2 is "<<op2<<endl;
        if(op1>=op2){
            isBranchTaken=1;
        }
        branchPc = pc+immediate;
    }

    //jal starts
    else if(alu_op=="jal"){
        isBranchTaken=1;
        branchPc=pc+immediate;
        aluop_result=pc+4;
    }

    //lui and auipc

    else if(alu_op=="lui"){
        aluop_result=immediate<<12;
    }
    else if(alu_op=="auipc"){
        aluop_result=pc+(immediate<<12);
    }
    else{
        cout<<"EXECUTE: pls check there is some error! "<<endl;
    }

    cout<<"First PC IS "<<pc<<endl;
    cout<<"EXECUTE: Alu Operation : "<<alu_op<<" "<<" Operation Result : "<<aluop_result<<endl;
    //branch selection decision
     if(isBranchTaken){
            pc=branchPc;
             cout<<"EXECUTE: Branch is taken"<<endl;
    }else{
            pc=nextPc;
            cout<<"EXECUTE: Branch is not taken"<<endl;
    }
    isBranchTaken=0;
    cout<<"Second PC IS "<<pc<<endl;
}

void fetch(){
    string current_instruction=instructrion_memory_map[pc];
    string s=hex2bin(current_instruction);
    bitset<M> bset1(s);
    cout<<"FETCH: Instruction  "<<current_instruction<<" fetched from address "<<"0x"<<pc;
    fetchResult=bset1;
    nextPc=pc+4;
};

void decode(){
    opcode=find_opcode();
    int func3=find_func3();
    int func7=find_func7();
    cout<<endl;
    instr_type=pair_of_instr_type_opcode[opcode];
    switch (instr_type)
    {
        case 'R':
            if(func3==0){
                if(func7==0) {
                    alu_op="add";
                } else if(func7==32) {
                    alu_op="sub";
                } else {
                    alu_op="error";
                    cout<<"Error in R type add or sub operation";
                }
            } else if(func3==4) {
                alu_op="xor";
            } else if(func3==6) {
                alu_op="or";
            } else if(func3==7) {
                alu_op="and";
            } else if(func3==1) {
                alu_op="sll";
            } else if(func3==5) {
                if(func7==0){
                    alu_op="srl";
                }else{
                    alu_op="sra";
                }
            } else if(func3==2) {
                alu_op="slt";
            }
            cout<<"DECODE: Operation is "<<alu_op<<" first operand R"<<find_rs1()<< " Second operand R"<<find_rs2()<< " destination register R"<<find_rd()<<endl;
            break;
        
    case 'I':
        if(opcode=="0010011"){
                if(func3==0){
                alu_op="addi";   
            }else if(func3==4){
                alu_op="xori";
            }else if(func3==6){
                alu_op="ori";
            }else if(func3==7){
                alu_op="andi";
            }else if(func3==1){
                alu_op="slli";
            }else if(func3==5){
                if(func7==0){
                    alu_op="srli";
                }else{
                    alu_op="srai";
                }
            }else if(func3==2){
                alu_op="slti";
            } 
        }else if(opcode=="0000011"){
            //load 
            if (func3==0) {
                alu_op="load_byte";
            } else if (func3==1) {
                alu_op="load_half";
            } else if (func3==2) {
                alu_op="load_word";
            }  
        }else if(opcode=="1100111"){
                alu_op="jalr";
        }
         cout<<"DECODE: Operation is "<<alu_op<<" first operand R"<<find_rs1()<< " Immediate "<<find_immed('I')<< " destination register R"<<find_rd()<<endl;
        break;

    //store
    case 'S':
        if (func3==0) {
            alu_op="store_byte";
        } else if (func3==1) {
            alu_op="store_half";
        } else if (func3==2) {
            alu_op="store_word";
        }
        cout<<"DECODE: Operation is "<<alu_op<<" first operand R"<<find_rs1()<< " Second operand R"<<find_rs2()<< " Immediate "<<find_immed('S')<<endl;
        break; 
  
    //branch
    case 'B':
        if(func3==0){
            alu_op="beq";
        }
        else if(func3==1){
            alu_op="bne";
        }
        else if(func3==4){
            alu_op="blt";
        }
        else if(func3==5){
            alu_op="bge";
        }
        else{
            alu_op="error";
        }
        cout<<"DECODE: Operation is "<<alu_op<<" first operand R"<<find_rs1()<< " Second operand R"<<find_rs2()<< " Immediate "<<find_immed('B')<<endl;
        break;
    case 'J': 
        if(opcode=="1101111") {
            alu_op="jal";
            cout<<"DECODE: Operation is "<<alu_op<<" Destination Register is R"<<find_rd()<<  " Immediate "<<find_immed('J')<<endl;
        }
        break;
    case 'U':
        if(opcode=="0110111") {
            alu_op="lui";
        }else{
            alu_op="auipc";
        }
        cout<<"DECODE: Operation is "<<alu_op<<" Destination Register is R"<<find_rd()<<  " Immediate "<<find_immed('U')<<endl;
        break;
    default:
        break;
    }
    rs1 = find_rs1();
    rs2 = find_rs2();

    rd = find_rd();
    immediate = find_immed(instr_type);
    
    op1=registerFile[rs1];
    op2=registerFile[rs2];
    
    cout<<"func3 : "<<func3<<" func7 : "<<func7<<endl;

}

string dec2hex(){
    char hexadecimal[100];
    string s;
    int quotient, remainder;
    int i = 0;
    int decimal=pc;
    while (decimal > 0)
    {
        quotient = decimal / 16;
        remainder = decimal % 16;

        if (remainder < 10)
        {
            hexadecimal[i] = remainder + 48;
            i++;
        }
        else
        {
            hexadecimal[i] = remainder + 55;
            i++;
        }

        decimal = quotient;
    }

    reverse(hexadecimal, hexadecimal + i);

    cout << "The hexadecimal equivalent is: ";
    for (int j = 0; j < i; j++){
        s+=hexadecimal[j];
    }
        cout<<s;

    return s;
}