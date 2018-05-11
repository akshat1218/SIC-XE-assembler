//this program is used to create the intermediate file from the input file. It consist of 6 parts of each line which are 
//1.Line number
//2.Label
//3.OPCODE
//4.Infomation of OPCODE
//5.Starting point of location counter
//6.End point of location counter

#include<iostream>
#include<cstdlib>
#include<fstream>
#include<string>
#include<cstdio>
#include<map>

using namespace std;

#include "TABLES.cpp"
#include "HEXA_DEC.cpp"


//this method takes the input as a line and breaks the line into words and store them in word[]
void convert_line_to_words(string a, string word[],int &count){
	for(int i=0;i<5;i++){
		word[i]="";
	}
	int j=0;
	if (j==a.length()||a[j]=='.'){
		return;
	}
	for (j=0;j<a.length();){
		while((a[j]==' '||a[j]=='\t')&&j<a.length()){
			++j;
		}
		if (j==a.length()){
			break;
		}
		while((!(a[j]==' '||a[j]=='\t'))&&j<a.length()){
			word[count]=word[count]+a[j];
			++j;
		}
		++count;
	}
}

ifstream input;
ofstream intermediate,error;

int block_count;
//it contains the total number of blocks
int line;
//it represents the line number
hexa LOCCTR; 
//it contains the location counter
string current_block;
//it consist of the information of the current block
bool error_flag=0;

int lit_count=0;
int lit_before_LTORG=0;

void pass(){
	create();
	input.open("input.txt");
	intermediate.open("intermediate.txt");
	error.open("error.txt");
	
	int line=5;
	string read_line;
	string word[5];
	int count=0;
	//it reads the line from input and stores in it
	getline(input,read_line);
	//count represent of the number of words in a line
	convert_line_to_words(read_line,word,count);

	
	//it runs if the input file is empty
	if (count==0){
		intermediate<<line<<endl;
		intermediate<<"$"<<endl;
		intermediate<<read_line<<endl;
		intermediate<<""<<endl;
		intermediate<<""<<endl;
		intermediate<<""<<endl;
		line+=5;
	}
	
	LOCCTR="0";
	
	//first daefault block is defined
    BLOCK["DEFAULT"].num=0;
    BLOCK["DEFAULT"].address=LOCCTR;
    BLOCK["DEFAULT"].length="0";
    BLOCK["DEFAULT"].exist='y';
    current_block="DEFAULT";
    block_count++;
    
    
    line=5;
    if (word[0]=="START"){
    	intermediate<<line<<endl;
		intermediate<<""<<endl;
		intermediate<<word[0]<<endl;
		intermediate<<LOCCTR<<endl;
		intermediate<<LOCCTR<<endl;
		intermediate<<""<<endl;
		line+=5;
	}
	else if (word[1]=="START"){
		intermediate<<line<<endl;
		intermediate<<word[0]<<endl;
		intermediate<<word[1]<<endl;
		intermediate<<LOCCTR<<endl;
		intermediate<<LOCCTR<<endl;
		intermediate<<""<<endl;
		line+=5;
	}
	else{
		error_flag=1;
		error << "No Start Statement found";
		exit(1);
	}
	/*IT IS THE LOOP IN WHICH IT READS THE LINE FROM INPUT
	BREAK THEM INTO WORDS 
	AND THEN FORM THE INTERMEDIATE FILE
	IT ALSO CALCULATES THE LOCATION ADDRESS*/
	while (true){
		getline(input,read_line);
		count=0;
		convert_line_to_words(read_line,word,count);
		
		//cout <<"A";
		intermediate<<line<<endl;
		line+=5;
		if (count==0){
			intermediate<<"$"<<endl;
			intermediate<<read_line<<endl;
			intermediate<<""<<endl;
			intermediate<<""<<endl;
			intermediate<<""<<endl;
		}
		else if (word[0]=="END"){
			BLOCK[current_block].length=LOCCTR;
			intermediate<<""<<endl;
			intermediate<<word[0]<<endl;
			intermediate<<word[1]<<endl;
			intermediate<<LOCCTR<<endl;
			intermediate<<""<<endl;
			break;
			if (lit_before_LTORG!=lit_count){
				for (int p=lit_before_LTORG;p<lit_count;p++){
					string temp = find_littab(p);
					string u=LITTAB[temp].value;
					LITTAB[temp].address=LOCCTR;
					LITTAB[temp].exist='n';
					int len=u.length()-3;
			        if(u[0]=='X') len/=2;
		            LOCCTR=toHex(toDec(LOCCTR)+len);
				}
				lit_before_LTORG=lit_count;
			}
		}
		else{
	
			if (word[0]=="USE"){
				BLOCK[current_block].length=LOCCTR;
				if(word[1]==""){
					current_block="DEFAULT";
					LOCCTR=BLOCK["DEFAULT"].length;
				}
				else if (BLOCK[word[1]].exist=='y'){
					current_block=word[1];
					LOCCTR=BLOCK[word[1]].length;
				}
				else{
					//new block is made
					block_count++;
					
					BLOCK[word[1]].num=block_count-1;
					BLOCK[word[1]].exist='y';
					BLOCK[word[1]].length="0";
					current_block=word[1];
					LOCCTR=BLOCK[word[1]].length;
				}
				intermediate<<""<<endl;
				intermediate<<word[0]<<endl;
				intermediate<<word[1]<<endl;
				intermediate<<LOCCTR<<endl;
				intermediate<<""<<endl;
			}
			else if (word[0][0]=='+'){
				intermediate<<""<<endl;
				intermediate<<word[0]<<endl;
				intermediate<<word[1]<<endl;
				intermediate<<LOCCTR<<endl;
				LOCCTR=toHex(toDec(LOCCTR)+4);
				intermediate<<LOCCTR<<endl;
			}
			// in case of LTORG 
			else if (word[0]=="LTORG"){
				intermediate<<""<<endl;
				intermediate<<word[0]<<endl;
				intermediate<<""<<endl;
				intermediate<<LOCCTR<<endl;
				
				for (int p=lit_before_LTORG;p<lit_count;p++){
					string temp = find_littab(p);
					string u=LITTAB[temp].value;
					LITTAB[temp].address=LOCCTR;
					//cout << LOCCTR;
					LITTAB[temp].exist='n';
					int len=u.length()-3;
			        if(u[0]=='X') len/=2;
		            LOCCTR=toHex(toDec(LOCCTR)+len);
				}
				lit_before_LTORG=lit_count;
				intermediate<<LOCCTR<<endl;
			}
			//IF THERE IS OPCODE AT A[0]
			else if (OPTAB[word[0]].exist=='y'){
				intermediate<<""<<endl;
				intermediate<<word[0]<<endl;
				intermediate<<word[1]<<endl;
				intermediate<<LOCCTR<<endl;
				LOCCTR=toHex(toDec(LOCCTR)+OPTAB[word[0]].format);
				intermediate<<LOCCTR<<endl;
				if (word[1][0]=='='){
					string x;
					x=word[1].substr(1);
					if(LITTAB[word[1]].exist=='y'){
						//continue;
					}
					else{
						LITTAB[word[1]].exist='y';
						LITTAB[word[1]].num=lit_count;
						LITTAB[word[1]].value=x;
						LITTAB[word[1]].block=current_block;
						lit_count++;	
					}
				}
			}
			//IN CASE OF LABEL AT A[0]
			else if(OPTAB[word[0]].exist=='n'){
				if (SYMTAB[word[0]].exist=='y'){
					error <<"duplicate error";
					error_flag=1;
					exit(1);
				}
				else{
					// new label is formed
					
					SYMTAB[word[0]].address=LOCCTR;
					SYMTAB[word[0]].block=current_block;
					SYMTAB[word[0]].exist='y';
					intermediate<<word[0]<<endl;
					intermediate<<word[1]<<endl;
					intermediate<<word[2]<<endl;
					intermediate<<LOCCTR<<endl;
					
					if (word[1]=="EQU"){
						SYMTAB[word[0]].value=word[2];
						LOCCTR=toHex(toDec(LOCCTR)+3);
					}
					//now, we need to find the ending of the location counter
					else if(word[1][0]=='+')
		                LOCCTR=toHex(toDec(LOCCTR)+4);
		            else if(OPTAB[word[1]].exist=='y')
		                LOCCTR=toHex(toDec(LOCCTR)+OPTAB[word[1]].format);
		            else if(word[1]=="WORD")      LOCCTR=toHex(toDec(LOCCTR)+3);
		            else if(word[1]=="RESW")      LOCCTR=toHex(toDec(LOCCTR)+(atoi(word[2].c_str())*3));
		            else if(word[1]=="RESB")      LOCCTR=toHex(toDec(LOCCTR)+atoi(word[2].c_str()));
		            else if(word[1]=="BYTE")	  {
		            	{
			                 int len=word[1].length()-3;
			                 if(word[1][0]=='X') len/=2;
			                 LOCCTR=toHex(toDec(LOCCTR)+len);
			            }
					}else
			            {
			                error<<"Line "<<line<<": Opcode not found"<<endl;
			                error_flag=1;
			            }
					if (word[2][0]=='='){
						string x;
						x=word[2].substr(1);
						if(LITTAB[word[2]].exist=='y'){
							//continue;
						}
						else{
							LITTAB[word[2]].exist='y';
							LITTAB[word[2]].num=lit_count;
							LITTAB[word[2]].value=x;
							LITTAB[word[2]].block=current_block;
							lit_count++;
							
						}
					}
		            
					intermediate<<LOCCTR<<endl;
					
				
				}
				// CAse in which LItearl is present at the 3rd location
				
			}
		}
		//COMPUTING .ADDRESS FOR EACH BLOCK
		hexa addr, len;
		string temp=find_block(0);
	    addr=BLOCK[temp].address;
	    len=BLOCK[temp].length;
	    for(int i=1;i<block_count;++i)
	    {
		           temp=find_block(i);
		           BLOCK[temp].address=toHex(toDec(addr)+toDec(len));
		           addr=BLOCK[temp].address;
		           len=BLOCK[temp].length;
	
	    }
	}
}

