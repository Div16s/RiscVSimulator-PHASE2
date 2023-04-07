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
// void RiscVSimulator();   //function to run risc-v simulator
void fetch();
void decode();
void execute();
void memory();
void write_back();

int num_of_inst=0;
int p=0;
double clk=0;

int executeRun=0;
int branchTaken=0;

//STALLS
int jal_dep=0;
int control_dep=0;
int RAW_dep=0;

//fetching stops 
int stop_fetching=0;
int counter_next_instruction=0;//counter

//PIPELINE LIST
list<char> pipeline{'f'};


//OUTPUTS
double total_num_inst=0; //
int load_store_inst=0;   //
int alu_inst=0;          
int control_inst=0;      //  
int num_stalls=0;        //
int num_data_hazards=0;      //
int num_control_hazards=0;  //
int branch_misprediction=0;  //
int stalls_data_hazards=0;  //
int stalls_control_hazards=0;  //



int find_func3(bitset <M> fetchResult);
int find_func7(bitset <M> fetchResult);
string find_opcode(bitset <M> fetchResult);
int find_rs1(bitset <M> fetchResult);
int find_rs2(bitset <M> fetchResult);
int find_rd(bitset <M> fetchResult);
int find_immed(char instruction_type);


//PIPELINES

struct IF_DE {
    bitset <M> inst;
    int pc = -1;
    int next_pc = -1;
    string instruction = "";
    int nop = -1;
} P_IF_DE;

struct DE_EX {
    bitset <M> inst;
    char inst_type;
    int pc = -1, next_pc = -1;
    string alu_op = "";
    int rs1 = -1, rs2 = -1, rd = -1;
    int op1 = -1, op2 = -1;
    string opcode = "";
    int immd = -1;
    string instruction = "";
    int nop = -1;
} P_DE_EX;

struct EX_MA {
    bitset <M> inst;
    int alu_result = -1;
    int pc = -1, next_pc = -1;
    int op1 = -1, op2 = -1;
    int rd = -1;
    char inst_type;
    string opcode = "";
    string alu_op = "";
    int immd = -1;
    string instruction = "";
    int nop = -1;
} P_EX_MA;

struct MA_WB {
    bitset <M> inst;
    int alu_result = -1;
    int rd = -1;
    char inst_type;
    string opcode = "";
    string alu_op = "";
    int immd = -1;
    string instruction = "";
    int pc = -1;
    int nop = -1;
} P_MA_WB;


void exec(list<char> &main_list) {
	int t=0;
	int s=0;

    executeRun=0;
    branchTaken=0;
	for(auto i=main_list.rbegin(); i!=main_list.rend();i++){
		switch(*i){
			case 'f':
				//cout<<"f";
				p++;
				if(p<num_of_inst)
					t=1;
				*i='d';	
                fetch();
				break;
			case 'd':
				//cout<<"d";
				*i='e';
                decode();	
				break;
			case 'e':
				//cout<<"e";
				*i='m';	
                execute();
				break;
			case 'm':
				//cout<<"m";
				*i='w';	
                memory();
				break;
			case 'w':
				//cout<<"w";
				s=1;	
                write_back();
				break;
		}
	}
    
    clk++;


	if(!stop_fetching) {
        main_list.push_front('f');
        total_num_inst++;
    }    
	if(s)
		main_list.pop_back();

    cout<<"%";
    // cout<<"t:"<<total_num_inst<<"%";
    for (int i=0; i<32; i++) {
        cout<<"x"<<i<<":"<<registerFile[i]<<"%";
    }
    cout<<"clock:"<<int(clk)<<"%";
    if(branchTaken){
        pc=branchPc;
        control_dep=1;
    }else{
        pc+=4;
    }
    cout<<"pc:"<<pc<<"%";
    cout<<"RAW_dep:"<<RAW_dep<<"%";
    cout<<"JAL_dep:"<<jal_dep<<"%";
    cout<<"CNTR_dep:"<<control_dep<<"%";
    // cout<<"Stop_Fetching:"<<stop_fetching<<"%";
    cout<<"\n$\n";

    if (jal_dep) {
        P_IF_DE.nop=1;
        P_DE_EX.nop=1;
        jal_dep=0;
        // total_num_inst-=2;

        // num_stalls+=2;
        // stalls_control_hazards+=2;
        // num_control_hazards++; 
    }  

    if (control_dep) {
        P_IF_DE.nop=1;
        P_DE_EX.nop=1;
        control_dep=0;
        total_num_inst-=2;

        num_stalls+=2;
        stalls_control_hazards+=2;
        branch_misprediction++;
        num_control_hazards++;
    } 
}

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

            ////////////
            instructrion_memory_map[counter_next_instruction+4]="-1";
            instructrion_memory_map[counter_next_instruction+8]="-1";

        }
        else {
            cout<<"Error in file2 opening"<<endl;
        }
        
    }
    }else{
        cout<<"Error in file opening"<<endl;
    }


    num_of_inst=counter_next_instruction/4;
    // cout<<"num_of_inst: "<<num_of_inst<<endl;

	while(!pipeline.empty()){
		exec(pipeline);
	}
    total_num_inst-=2;
    
    cout<<"%";
    for (int i=0; i<32; i++) {
        cout<<"x"<<i<<":"<<registerFile[i]<<"%";
    }
    cout<<"clock:"<<int(clk)<<"%";
    cout<<"%Total number of Cycles:"<<int(clk)<<"%"; 
    cout<<"Total instruction executed:"<<total_num_inst<<"%";
    cout<<"CPI:"<<(clk/total_num_inst)<<"%";
    cout<<"Number of Data Transfer (load/store):"<<load_store_inst<<"%";
    cout<<"Number of ALU Instruction:"<<alu_inst<<"%";
    cout<<"Number of Control Instructions:"<<control_inst<<"%"; 
    cout<<"Number of stalls in pipeline:"<<num_stalls<<"%";
    cout<<"Number of data hazards:"<<num_data_hazards<<"%";
    cout<<"Number of control hazards:"<<num_control_hazards<<"%";  
    cout<<"Number of branch misprediction:"<<branch_misprediction<<"%";
    cout<<"Number of stalls due to data hazards:"<<stalls_data_hazards<<"%";
    cout<<"Number of stalls due to control hazards:"<<stalls_control_hazards;
    cout<<"$";
    
}

//function to run single phase risc-v simulator
// void RiscVSimulator() {
//     fetch();
//     decode();
//     execute();
//     memory();
//     write_back();  
// };

void show_output() {
    clk++;
    cout<<"%";
    for (int i=0; i<32; i++) {
        cout<<"x"<<i<<":"<<registerFile[i]<<"%";
    }
    cout<<"clock:"<<int(clk)<<"%";
    // if(branchTaken){
    //     pc=branchPc;
    //     control_dep=1;
    // }else{
    //     //pc=nextPc;
    //     pc+=4;
    // }
    cout<<"pc:"<<pc<<"%";
    cout<<"RAW_dep:"<<RAW_dep<<"%";
    cout<<"JAL_dep:"<<jal_dep<<"%";
    cout<<"CNTR_dep:"<<control_dep<<"%";
    cout<<"\n$\n";

    // if (jal_dep) {
    //     P_IF_DE.nop=1;
    //     P_DE_EX.nop=1;
    //     jal_dep=0;
    // }  

    // if (control_dep) {
    //     P_IF_DE.nop=1;
    //     P_DE_EX.nop=1;
    //     control_dep=0;
    // } 
}

void show_stall(string str) {
    cout<<"... STALL ... "<<str<<'\n';
}

int check_dep(bitset <M> d1, bitset <M> d2) {
    //d1 depends on d2
    
    string opcode_d1 = find_opcode(d1);
    string opcode_d2 = find_opcode(d2);

    char type_d1 = pair_of_instr_type_opcode[opcode_d1];
    char type_d2 = pair_of_instr_type_opcode[opcode_d2];   

    if (type_d1 == 'U' || type_d1 == 'J' || type_d2 == 'S' || type_d2 == 'B') return 0;   

    int rs1_d1 = find_rs1(d1);
    int rs2_d1 = find_rs2(d1);

    int rd_d2 = find_rd(d2);

    if (rs1_d1 == rd_d2 && rs1_d1 !=0) return 1;
    if ((type_d1 == 'R' || type_d1 == 'S' || type_d1 == 'B') && (rs2_d1 == rd_d2) && (rs2_d1 != 0)) return 1;
    return 0;
}


void fetch(){
    string current_instruction=instructrion_memory_map[pc];
    string s=hex2bin(current_instruction);
    bitset<M> bset1(s);
    cout<<"\n... FETCHING ... "<<current_instruction<<"\n";
    cout<<"FETCH: Instruction  "<<current_instruction<<" fetched from address "<<"0x"<<pc;
    fetchResult=bset1;
    nextPc=pc+4;

    //FILLING PIPELINE IF_DE
    P_IF_DE.inst = fetchResult;
    P_IF_DE.pc = pc;
    P_IF_DE.next_pc = nextPc;
    P_IF_DE.instruction = current_instruction;
    P_IF_DE.nop=0;
};

void decode(){
    if (!P_IF_DE.nop) {
   
    rd = find_rd(P_IF_DE.inst);
    rs1 = find_rs1(P_IF_DE.inst);
    rs2 = find_rs2(P_IF_DE.inst);

    if (check_dep(P_IF_DE.inst, P_DE_EX.inst) && P_DE_EX.nop==0 && jal_dep==0) {
        RAW_dep=3;
        
        cout<<"... DECODE STOPPED ...\n";
        cout<<"... FETCH STOPPED ...\n";
        show_output();
        
    
        write_back();
        memory(); 
        show_stall("EXECUTE");
        cout<<"... DECODE STOPPED ...\n";
        cout<<"... FETCH STOPPED ...\n";
        show_output();

        write_back(); 
        show_stall("MEMORY");
        show_stall("EXECUTE");
        cout<<"... DECODE STOPPED ...\n";
        cout<<"... FETCH STOPPED ...\n";

        show_output();

        show_stall("WRITE back");
        show_stall("MEMORY");
        show_stall("EXECUTE");
        
    } else if (check_dep(P_IF_DE.inst, P_MA_WB.inst) && P_MA_WB.nop==0 && jal_dep==0) {
        RAW_dep=1;

        cout<<"... DECODE STOPPED ...\n";
        cout<<"... FETCH STOPPED ...\n";
        show_output();

        write_back();
        memory(); 
        show_stall("EXECUTE");
        // cout<<"... DECODE STOPPED ...\n";
        // cout<<"... FETCH STOPPED ...\n";
        // show_output();

        // write_back();
        // show_stall("MEMORY");
        // show_stall("EXECUTE");
        
    } 

    opcode=find_opcode(P_IF_DE.inst);
    int func3=find_func3(P_IF_DE.inst);
    int func7=find_func7(P_IF_DE.inst);
    // cout<<'\n';
    instr_type=pair_of_instr_type_opcode[opcode];

    // rs1 = find_rs1(P_IF_DE.inst);
    // rs2 = find_rs2(P_IF_DE.inst);
    // rd = find_rd(P_IF_DE.inst);
    immediate = find_immed(instr_type);


    //CHECKING RAW DEPENDENCY
    

    cout<<"\n... DECODING ... "<<P_IF_DE.instruction<<'\n';

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
            cout<<"DECODE: Operation is "<<alu_op<<" first operand R"<<rs1<< " Second operand R"<<rs2<< " destination register R"<<rd<<endl;
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
         cout<<"DECODE: Operation is "<<alu_op<<" first operand R"<<rs1<< " Immediate "<<immediate<< " destination register R"<<rd<<endl;
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
        cout<<"DECODE: Operation is "<<alu_op<<" first operand R"<<rs1<< " Second operand R"<<rs2<< " Immediate "<<immediate<<endl;
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
        cout<<"DECODE: Operation is "<<alu_op<<" first operand R"<<rs1<< " Second operand R"<<rs2<< " Immediate "<<immediate<<endl;
        break;
    case 'J': 
        if(opcode=="1101111") {
            alu_op="jal";
            cout<<"DECODE: Operation is "<<alu_op<<" Destination Register is R"<<rd<<" Immediate "<<immediate<<endl;
        }
        break;
    case 'U':
        if(opcode=="0110111") {
            alu_op="lui";
        }else{
            alu_op="auipc";
        }
        cout<<"DECODE: Operation is "<<alu_op<<" Destination Register is R"<<rd<<" Immediate "<<immediate<<endl;
        break;
    default:
        break;
    }
    
    // rs1 = find_rs1(P_IF_DE.inst);
    // rs2 = find_rs2(P_IF_DE.inst);
    // rd = find_rd(P_IF_DE.inst);
    // immediate = find_immed(instr_type);
    
    op1=registerFile[rs1];
    op2=registerFile[rs2];
    
    cout<<"func3 : "<<func3<<" func7 : "<<func7<<endl;
    } else {
        cout<<"\n... STALL ... DECODE ... "<<P_IF_DE.instruction<<'\n';
    }

    //FILLING PIPELINE DE_EX
    P_DE_EX.inst = P_IF_DE.inst;
    P_DE_EX.pc = P_IF_DE.pc;                   // --
    P_DE_EX.next_pc = P_IF_DE.next_pc;
    P_DE_EX.alu_op = alu_op;
    P_DE_EX.rs1 = rs1;
    P_DE_EX.rs2 = rs2;
    P_DE_EX.rd = rd;
    P_DE_EX.op1 = op1;
    P_DE_EX.op2 = op2;
    P_DE_EX.inst_type = instr_type;
    P_DE_EX.opcode = opcode;
    P_DE_EX.immd = immediate;
    P_DE_EX.instruction = P_IF_DE.instruction;
    P_DE_EX.nop = P_IF_DE.nop;  

    if (RAW_dep == 3) {
        P_EX_MA.nop=1;
        P_MA_WB.nop=1;
        RAW_dep = 0;

        num_stalls+=2;
        stalls_data_hazards+=2;

        num_data_hazards++;
    }

    if (RAW_dep == 1) {
        P_EX_MA.nop=1;
        RAW_dep = 0;

        num_stalls+=1;
        stalls_data_hazards+=1;

        num_data_hazards++;
    }
}

void execute(){
    if(P_DE_EX.instruction == "-1" && !P_DE_EX.nop){
        stop_fetching=1;
    } 

    if (!P_DE_EX.nop) {
    executeRun=1;
    cout<<"... EXECUTING ... "<<P_DE_EX.instruction<<'\n';
    if(P_DE_EX.alu_op=="add"){
        alu_inst++;
        P_EX_MA.alu_result = P_DE_EX.op1+P_DE_EX.op2;
    }
    else if(P_DE_EX.alu_op=="sub"){
        alu_inst++;
        P_EX_MA.alu_result = P_DE_EX.op1-P_DE_EX.op2;
    }
    else if(P_DE_EX.alu_op=="xor"){
        alu_inst++;
        P_EX_MA.alu_result = P_DE_EX.op1^P_DE_EX.op2;
    }
    else if(P_DE_EX.alu_op=="or"){
        alu_inst++;
        P_EX_MA.alu_result = P_DE_EX.op1|P_DE_EX.op2;
    }
    else if(P_DE_EX.alu_op=="and"){
        alu_inst++;
        P_EX_MA.alu_result = P_DE_EX.op1&P_DE_EX.op2;
    }
    else if(P_DE_EX.alu_op=="sll"){
        alu_inst++;
        P_EX_MA.alu_result = P_DE_EX.op1<<P_DE_EX.op2;
    }
    else if(P_DE_EX.alu_op=="srl"){
        alu_inst++;
        P_EX_MA.alu_result = P_DE_EX.op1>>P_DE_EX.op2;
    }
    else if(P_DE_EX.alu_op=="sra"){
        alu_inst++;
        P_EX_MA.alu_result = P_DE_EX.op1>>P_DE_EX.op2;
    }
    else if(P_DE_EX.alu_op=="slt"){
        alu_inst++;
        P_EX_MA.alu_result = (P_DE_EX.rs1 < P_DE_EX.rs2)?1:0;
    }


    //immediate starts
    else if(P_DE_EX.alu_op=="addi"){
        alu_inst++;
        P_EX_MA.alu_result = P_DE_EX.op1+P_DE_EX.immd;
    }
    else if(P_DE_EX.alu_op=="subi"){
        alu_inst++;
        P_EX_MA.alu_result = P_DE_EX.op1-P_DE_EX.immd;
    }
    else if(P_DE_EX.alu_op=="xori"){
        alu_inst++;
        P_EX_MA.alu_result = P_DE_EX.op1^P_DE_EX.immd;
    }
    else if(P_DE_EX.alu_op=="ori"){
        alu_inst++;
        P_EX_MA.alu_result = P_DE_EX.op1|P_DE_EX.immd;
    }
    else if(P_DE_EX.alu_op=="andi"){
        alu_inst++;
        P_EX_MA.alu_result = P_DE_EX.op1&P_DE_EX.immd;
    }
    else if(P_DE_EX.alu_op=="slli"){
        alu_inst++;
        P_EX_MA.alu_result = P_DE_EX.op1<<P_DE_EX.immd;
    }
    else if(P_DE_EX.alu_op=="srli"){
        alu_inst++;
        P_EX_MA.alu_result = P_DE_EX.op1>>P_DE_EX.immd;
    }
    else if(P_DE_EX.alu_op=="srai"){
        alu_inst++;
        P_EX_MA.alu_result = P_DE_EX.op1>>P_DE_EX.immd;
    }
    else if(P_DE_EX.alu_op=="slti"){
        alu_inst++;
        P_EX_MA.alu_result = (P_DE_EX.rs1 <P_DE_EX.immd)?1:0;
    }
    else if (P_DE_EX.alu_op=="jalr"){
        isBranchTaken=1;
        branchPc = P_DE_EX.op1+P_DE_EX.immd;
        P_EX_MA.alu_result=P_DE_EX.pc+4;
        jal_dep=1;
    }

    // load instructions execution
    else if(P_DE_EX.alu_op=="load_byte"){
        int newImmed= (~((~0)<<8))&P_DE_EX.immd;
        P_EX_MA.alu_result = P_DE_EX.op1 + newImmed;

    }
    else if(P_DE_EX.alu_op=="load_half"){
        int newImmed= (~((~0)<<16))&P_DE_EX.immd;
        P_EX_MA.alu_result = P_DE_EX.op1 + newImmed;

    }
    else if(P_DE_EX.alu_op=="load_word"){
        P_EX_MA.alu_result = P_DE_EX.op1 + P_DE_EX.immd;
        // cout<<" aluop_result "<<P_EX_MA.alu_result<<op1<<" "<<immediate<<endl;

    }


    // store instructions execution
    else if(P_DE_EX.alu_op=="store_byte"){
        int newImmed= (~((~0)<<8))&P_DE_EX.immd;
        P_EX_MA.alu_result = P_DE_EX.op1 + newImmed;

    }
    else if(P_DE_EX.alu_op=="store_half"){
        int newImmed= (~((~0)<<16))&P_DE_EX.immd;
        P_EX_MA.alu_result = P_DE_EX.op1 + newImmed;

    }
    else if(P_DE_EX.alu_op=="store_word"){
        P_EX_MA.alu_result = P_DE_EX.op1 + P_DE_EX.immd;

    }

    //branch starts
    else if(P_DE_EX.alu_op=="beq"){
        if(P_DE_EX.op1==P_DE_EX.op2){
            isBranchTaken=1;
        }
        branchPc = P_DE_EX.pc+P_DE_EX.immd;
        control_inst++;
    }
    else if(P_DE_EX.alu_op=="bne"){
        if(P_DE_EX.op1!=P_DE_EX.op2){
            isBranchTaken=1;
        }
        branchPc = P_DE_EX.pc+P_DE_EX.immd;
        control_inst++;
    }
    else if(P_DE_EX.alu_op=="blt"){
        if(P_DE_EX.op1<P_DE_EX.op2){
            isBranchTaken=1;
        }
        branchPc = P_DE_EX.pc+P_DE_EX.immd;
        control_inst++;
    }
    else if(P_DE_EX.alu_op=="bge"){
        cout<<"op1 is "<<P_DE_EX.op1<<" "<<"op2 is "<<P_DE_EX.op2<<endl;
        if(P_DE_EX.op1>=P_DE_EX.op2){
            isBranchTaken=1;
        }
        branchPc = P_DE_EX.pc+P_DE_EX.immd;
        control_inst++;
    }

    //jal starts
    else if(P_DE_EX.alu_op=="jal"){
        isBranchTaken=1;
        branchPc=P_DE_EX.pc+P_DE_EX.immd;
        P_EX_MA.alu_result=P_DE_EX.pc+4;
        
        //Making two stalls
        jal_dep=1;
    }

    //lui and auipc

    else if(P_DE_EX.alu_op=="lui"){
        P_EX_MA.alu_result=P_DE_EX.immd<<12;
    }
    else if(P_DE_EX.alu_op=="auipc"){
        P_EX_MA.alu_result=P_DE_EX.pc+(P_DE_EX.immd<<12);
    }
    else{
        cout<<"EXECUTE: pls check there is some error! "<<endl;
    }

    cout<<"First PC iS "<<P_DE_EX.pc<<endl;
    cout<<"EXECUTE: Alu Operation : "<<P_DE_EX.alu_op<<" "<<" Operation Result : "<<P_EX_MA.alu_result<<endl;
    //branch selection decision
    
    //isBranchTaken is a flag indicator 
    if(isBranchTaken){
        //pc=branchPc;
        cout<<"EXECUTE: Branch is taken"<<endl;
        cout<<"Second PC iS "<<branchPc<<endl;
        branchTaken=1;
    }else{
        //pc=nextPc;
        //pc+=4;
        cout<<"EXECUTE: Branch is not taken"<<endl;
        cout<<"Second PC iS "<<P_DE_EX.pc+4<<endl;
    }
    isBranchTaken=0;
    } else {
        cout<<"\n... STALL ... EXECUTE ... "<<P_DE_EX.instruction<<'\n';
    }

    //FILLING PIPEPLINE EX_MA
    P_EX_MA.inst = P_DE_EX.inst;
    P_EX_MA.pc = P_DE_EX.pc;                                //--
    P_EX_MA.op1 = P_DE_EX.op1;
    P_EX_MA.op2 = P_DE_EX.op2;
    P_EX_MA.rd = P_DE_EX.rd;
    P_EX_MA.inst_type = P_DE_EX.inst_type;
    P_EX_MA.opcode = P_DE_EX.opcode;
    P_EX_MA.alu_op = P_DE_EX.alu_op;
    P_EX_MA.immd = P_DE_EX.immd;
    P_EX_MA.instruction = P_DE_EX.instruction; 
    P_EX_MA.nop = P_DE_EX.nop;  
}

void memory(){
    //laod 
    if (!P_EX_MA.nop) {
        cout<<"... ACCESSING MEMORY ... "<<P_EX_MA.instruction<<'\n';
        if(P_EX_MA.inst_type=='I' && P_EX_MA.opcode=="0000011") {
            //at this point aluop_result will carry the location of the memory
            P_EX_MA.alu_result = main_mem[P_EX_MA.alu_result];
            load_store_inst++;
            cout<<"MEMORY: "<<alu_op<<" operation is performed in the memory, loaded value (in op2): "<<P_EX_MA.alu_result<<endl;
        } else if(P_EX_MA.inst_type=='S') {
            //at this point aluop_result will carry the location of the memory
            main_mem[P_EX_MA.alu_result]=P_EX_MA.op2;
            load_store_inst++;
            cout<<"MEMORY: "<<P_EX_MA.alu_op<<" operation is performed in the memory, stored value (of op2): "<<P_EX_MA.op2<<endl;
        } else {
            cout<<"MEMORY: No memory operation to perform.\n";
        }

        cout<<'\n';
    } else {
        cout<<"\n... STALL ... MEMORY ... "<<P_EX_MA.instruction<<'\n';
    }

    //FILLING PIPELINE MA_WB
    P_MA_WB.alu_result = P_EX_MA.alu_result;
    P_MA_WB.inst = P_EX_MA.inst;
    P_MA_WB.rd = P_EX_MA.rd;
    P_MA_WB.inst_type = P_EX_MA.inst_type;
    P_MA_WB.alu_op = P_EX_MA.alu_op;
    P_MA_WB.immd = P_EX_MA.immd;
    P_MA_WB.instruction = P_EX_MA.instruction; 
    P_MA_WB.pc = P_EX_MA.pc;
    P_MA_WB.nop = P_EX_MA.nop;
}

void write_back(){
    if (!P_MA_WB.nop) {
        cout<<"\n... WRITING back ... "<<P_MA_WB.instruction<<'\n';
        if(P_MA_WB.inst_type!='S' && P_MA_WB.inst_type!='B' && P_MA_WB.rd!=0) {
            registerFile[P_MA_WB.rd]=P_MA_WB.alu_result;
            cout<<"WRITEBACK: writing "<<P_MA_WB.alu_result<<" to "<<"Register R"<<P_MA_WB.rd<<endl;
        } else {
            cout<<"WRITEBACK: No write back! "<<endl;
        }
    } else {
        cout<<"... STALL ... WRITE back ... "<<P_MA_WB.instruction<<'\n';
    }
    cout<<'\n';
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

//function for finding funct3
int find_func3(bitset <M> fetchResult){
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
int find_func7(bitset <M> fetchResult){
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
string find_opcode(bitset <M> fetchResult){
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
int find_rs1(bitset <M> fetchResult){
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
int find_rs2(bitset <M> fetchResult){
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
int find_rd(bitset <M> fetchResult){
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
