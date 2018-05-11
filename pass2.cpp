#include<iostream>
#include<cstdlib>
#include<fstream>
#include<string>
#include<cstdio>
#include<map>
#include<climits>

using namespace std;

#include "Pass1.cpp"

ofstream obj,lst,mod;
ifstream inter;
int curr_block_num;

void modify_object_file()
{
   ifstream fin;
   fin.open("modification.txt");
   string s;
   while(true)
   {
       getline(fin,s);
       if(s=="") break;
       obj<<s<<endl;
   }
}
/*IMM IS USED IF THERE IS IMMEDIATE ADDRESSING
IND IS USED IF THERE IS INDIRECT ADDRESSING*/ 
bool imm,ind,lit;


//METHOD TAKES THE 6 LINES FROM THE INTERMEDIATE FILE
void next_line(string a[])
{
    int i;
    for(i=0;i<6;++i)
        getline(inter,a[i]);
}



string TR="",TR_object_code="";
/*TR and TR_object_code are used for the text record
TR stores the starting address
whereas TR_object_code stores the object code */


int text_length=0,base=INT_MAX;
//text_length stores the length in the text record file

//GENERATES THE OPCODE FOR EVERY LINE
string OPCODE(string a[])
{
    string ob1,ob2,ob3;
    hexa operand_addr,prgm_ctr;
    int format;
    if(a[2]=="BYTE")
    {
        int i;
        ob1="";
        if(a[3][0]=='X')
            for(i=2;i<a[3].length()-1;++i) ob1+=a[3][i];
        else 
            for(i=2;i<a[3].length()-1;++i)
                ob1+=toHex((int)a[3][i]);
        return ob1;
    }
    if(a[2]=="WORD")
    {
        ob1=toHex(atoi(a[3].c_str()));
        return ob1;
    }
    if(a[2]=="RSUB")
    {
        ob1="4F0000";
        return ob1;
    }
    if(a[2]=="+RSUB")
    {
        ob1="4F000000";
        return ob1;
    }
    if(a[2][0]=='+')
    {
        format=4;
        a[2]=a[2].substr(1);
    }
    else
        format=OPTAB[a[2]].format;
    if(format==1)
    {
        ob1=OPTAB[a[2]].opcode;
        return ob1;
    }
    if(format==2)
    {
        ob1=OPTAB[a[2]].opcode;
        if(a[3].length()==3)
        {
            ob2=toHex(reg_num(a[3][0]));
            if(isdigit(a[3][2]))  ob2=ob2+toHexDig(a[3][2]-1);
            else
            {
                ob2=ob2+toHexDig(reg_num(a[3][2]));
            }
        }
        else 
        {
            if(isdigit(a[3][0]))
            {
                ob2=toHex(atoi(a[3].c_str()))+"0";
            }
            else
            {
                ob2=toHex(reg_num(a[3][0]))+"0";
            }
        }
        return (ob1+extendTo(2,ob2));
    }
    if(format==3)
    {
        ob1=OPTAB[a[2]].opcode;
        if(a[3][0]=='#')
        {
            imm=true;
            ob1=toHex(toDec(ob1)+1);
            a[3]=a[3].substr(1);
            if(SYMTAB[a[3]].exist=='y'){
            	//DO nothing
			}
            else if(isdigit(a[3][0]))
            {
                ob2="0";
                ob3=toHex(atoi(a[3].c_str()));
                return extendTo(2,ob1)+ob2+extendTo(3,ob3);
            }
        }
        else if(a[3][0]=='@')
        {
            ind=true;
            ob1=toHex(toDec(ob1)+2);
            a[3]=a[3].substr(1);
        }
        else if (a[3][0]=='='){
        	lit=true;
        	ob1=toHex(toDec(ob1)+1);
        	//a[3]=a[3].substr(1);
        	
		}
        else
            ob1=toHex(toDec(ob1)+3);
        ob2="0";
        bool x=false;
        if(a[3][a[3].length()-2]==',')
        {
            x=true;
            ob2=toHex(toDec(ob2)+8);
            a[3]=a[3].substr(0,a[3].length()-2);
        }
        prgm_ctr=toHex(toDec(BLOCK[current_block].address)+toDec(a[5]));
        if(lit){
        	operand_addr=toHex(toDec(LITTAB[a[3]].address)+toDec(BLOCK[LITTAB[a[3]].block].address));
        	//cout << a[3];
        	//cout << LITTAB[a[3]].address;
        	//cout << BLOCK[LITTAB[a[3]].block].address;
        	//cout << operand_addr<<endl;
        	//cout << prgm_ctr<<endl;
		}
		else{
        	operand_addr=toHex(toDec(SYMTAB[a[3]].address)+toDec(BLOCK[SYMTAB[a[3]].block].address));
        }
        if(x) a[3]+=",X";
        int disp=toDec(operand_addr)-toDec(prgm_ctr);
        if(disp>=-2048 && disp<2048)
        {
            ob2=toHex(toDec(ob2)+2);
            if(disp<0) disp+=4096;
            ob3=toHex(disp);
            return extendTo(2,ob1)+extendTo(1,ob2)+extendTo(3,ob3);
        }
        disp=toDec(operand_addr)-base;
        if(disp>=-2048 && disp<2048)
        {
            ob2=toHex(toDec(ob2)+4);
            if(disp<0) disp+=4096;
            ob3=toHex(disp);
            return extendTo(2,ob1)+extendTo(1,ob2)+extendTo(3,ob3);
        }
        error_flag=1;
        error<<"Line "<<a[0]<<": Overflow detected"<<endl;
    }
    if(format==4)
    {
        ob1=OPTAB[a[2]].opcode;
        if(a[3][0]=='#')
        {
            imm=true;
            ob1=toHex(toDec(ob1)+1);
            a[3]=a[3].substr(1);
            if(isdigit(a[3][0]))
            {
                ob2="0";
                ob3=toHex(atoi(a[3].c_str()));
                a[2]="+"+a[2];
                return ob1+ob2+extendTo(5,ob3);
            }
        }
        else if(a[3][0]=='@')
        {
            ind=true;
            ob1=toHex(toDec(ob1)+2);
            a[3]=a[3].substr(1);
        }
        else
            ob1=toHex(toDec(ob1)+3);
        bool x=false;
        ob2="1";
        if(a[3][a[3].length()-2]==',')
        {
            x=true;
            ob2=toHex(toDec(ob2)+8);
            a[3]=a[3].substr(0,a[3].length()-2);
        }
        operand_addr=toHex(toDec(SYMTAB[a[3]].address)+toDec(BLOCK[SYMTAB[a[3]].block].address));
        if(x) a[3]+=",X";
        ob3=operand_addr;
        a[2]="+"+a[2];
        hexa loc_ctr=toHex(toDec(BLOCK[current_block].address)+toDec(a[4])+1);
        mod<<"M^"<<extendTo(6,loc_ctr)<<"^05"<<endl;
        return extendTo(2,ob1)+extendTo(1,ob2)+extendTo(5,ob3);
    }
}
int main()
{
	//BY pass METHOD INTERMEDIATE FILE IS FORMED USING pass1.cpp
    pass();
    
    string a[6];
    char ch;
    hexa start;
    //IN CASE IF ANY ERROR ENCOUNTERED
    //CAN CHECK error.txt FOR DETERMINING THE ERROR
    if(error_flag)
    {
        cout<<"Errors encountered!"<<endl;
        exit(1);
    }
    
    
    inter.open("intermediate.txt");
    obj.open("object.txt");
    lst.open("list.txt");
    mod.open("modification.txt");
    lst<<"Line\tLoc  Block\t\tSource Statement\t\t\tObject Code"<<endl;
    lst<<endl;
    next_line(a);
    current_block="DEFAULT";
    curr_block_num=0;
    while(a[1]=="$")
    {
        lst<<a[0]<<"\t\t "<<a[2]<<endl;
        next_line(a);
    }
    if(a[2]=="START")
    {
        lst<<a[0]<<"\t"<<extendTo(4,a[4])<<"  "<<curr_block_num<<"\t\t"<<a[1]<<"\t\t"<<a[2]<<"\t\t"<<a[3]<<endl;
        
        //WRIETING IN THE OBJECT FILE
        obj<<"H^";
        int i;
        for(i=0;i<a[1].length();++i)
            obj<<a[1][i];
        for(;i<6;++i)
            obj<<" ";
        string temp=find_block(block_count-1);
        hexa len;
        len=toHex(toDec(BLOCK[temp].address)+toDec(BLOCK[temp].length));
        obj<<"^"<<extendTo(6,a[3])<<"^"<<extendTo(6,len)<<endl;
        start=a[3];
    }
    else
    {
        error_flag=1;
        error<<"Intermediate File has no start!"<<endl;
    }
    
    
    /*THIS LOOP READS EACH LINE FROM THE INTERMEDIATE FILE
	GENERATE OPCODE WITH THE METHOD
	USING DIFFERENT BLOCKS*/
	
    while(true)
    {
        next_line(a);
        //IF THE INITIAL LINES ARE EMPTY OR COMMENTED 
        if(a[1]=="$")
        {
            lst<<a[0]<<"\t\t "<<a[2]<<endl;
            continue;
        }
        if(a[2]=="END")
        {
            lst<<a[0]<<"\t\t\t\t\t\t"<<a[2]<<"\t\t"<<a[3]<<endl;
            if(text_length>0)
            {
                obj<<TR<<"^"<<extendTo(2,toHex(text_length/2))<<TR_object_code<<endl;
                cout<<"!!"<<endl;
            }
            text_length=0;
            TR="";
            TR_object_code="";
            modify_object_file();
            obj<<"E^"<<extendTo(6,start)<<endl;
            cout << "PROGRAM ASSEMBLED SUCCESSFULLY";
            break;
        }
        if(a[2]=="LTORG"){
        	lst<<a[0]<<"\t\t\t\t\t"<<a[2]<<"\t\t"<<endl;
        	continue;
		}
        string object_code;
	    hexa loc_ctr;
	    int format;
	    // IN CASE OF SWITICHING BETTWWEN THE BLOCKS
	    if(a[2]=="USE")
	    {	
	    	if (a[3]==""){
	    		current_block="DEFAULT";
			}
			else{
	        	current_block=a[3];
	        }
	        curr_block_num=BLOCK[current_block].num;
	        lst<<a[0]<<"\t0000  "<<curr_block_num<<"\t\t\t"<<a[2]<<"\t\t"<<a[3]<<endl;
	        if(text_length>0) obj<<TR<<"^"<<extendTo(2,toHex(text_length/2))<<TR_object_code<<endl;
	        TR="";
	        TR_object_code="";
	        text_length=0;
	    }
	    else if(a[2]=="RESB"||a[2]=="RESW"||a[2]=="EQU")
	    {
	        lst<<a[0]<<"\t"<<extendTo(4,a[4])<<"  "<<curr_block_num<<"\t\t"<<a[1]<<"\t"<<a[2]<<"\t\t"<<a[3]<<endl;
	        if(text_length>0) obj<<TR<<"^"<<extendTo(2,toHex(text_length/2))<<TR_object_code<<endl;
	        TR="";
	        TR_object_code="";
	        text_length=0;
	    }
		    else{
		    imm=ind=false;
		    lit=false;
		    object_code=OPCODE(a);
		    if(a[2]=="BYTE"||a[2]=="WORD")
		        lst<<a[0]<<"\t"<<extendTo(4,a[4])<<"  "<<curr_block_num<<"\t\t"<<a[1]<<"\t"<<a[2]<<"\t\t"<<a[3]<<endl;
		    else
		    {
		        if(imm){      
					lst<<a[0]<<"\t"<<extendTo(4,a[4])<<"  "<<curr_block_num<<"\t\t"<<a[1]<<"\t\t"<<a[2]<<"\t\t#"<<a[3]<<"\t\t"<<object_code<<endl;
					//IMMEDIATE ADDRESSING
				}
		        else if(ind) {
					lst<<a[0]<<"\t"<<extendTo(4,a[4])<<"  "<<curr_block_num<<"\t\t"<<a[1]<<"\t\t"<<a[2]<<"\t\t@"<<a[3]<<"\t\t"<<object_code<<endl;
					//INDIRECT ADDRESSING
				}
				else if(lit) {
					lst<<a[0]<<"\t"<<extendTo(4,a[4])<<"  "<<curr_block_num<<"\t\t"<<a[1]<<"\t\t"<<a[2]<<"\t\t"<<a[3]<<"\t\t"<<object_code<<endl;
				}
		        else   {
			    	lst<<a[0]<<"\t"<<extendTo(4,a[4])<<"  "<<curr_block_num<<"\t\t"<<a[1]<<"\t\t"<<a[2]<<"\t\t"<<a[3]<<"\t\t"<<object_code<<endl;
			    	//DIRECT ADDRESSING
			    }
		    }
		    //INITIALIZING test_s
		    if(TR=="")
		    {
		        loc_ctr=toHex(toDec(BLOCK[current_block].address)+toDec(a[4]));
		        TR="T^"+extendTo(6,loc_ctr);
		        TR_object_code="^"+object_code;
		        text_length=object_code.length();
		    }
		    //CASE IN WHICH THE LENGTH OF THE TEXT RCORD EXCEED 60
		    else if(text_length+object_code.length()>60)
		    {
		        obj<<TR<<"^"<<extendTo(2,toHex(text_length/2))<<TR_object_code<<endl;
		        loc_ctr=toHex(toDec(BLOCK[current_block].address)+toDec(a[4]));
		        TR="T^"+extendTo(6,loc_ctr);
		        TR_object_code="^"+object_code;
		        text_length=object_code.length();
		    }
		    else
		    {
		        TR_object_code+="^"+object_code;
		        text_length+=object_code.length();
		    }
		    //UPDATING BASE REGISTER VALUE IN CASE OF BASE ADDRESSING
		    if(a[2]=="LDB")
		    {
		        base=toDec(SYMTAB[a[3]].address)+toDec(BLOCK[SYMTAB[a[3]].block].address);
		    }
		}
    }
    if(error_flag)
    {
        cout<<"Errors encountered!"<<endl;
    }
}

