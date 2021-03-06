// Author : Gagan Bihari Mishra
#include "generator.h"

int generator::generate(int ip)
{

  generatedCSAs = 0;
  int err = 0;
  width = ip;		//width of the operands = num of partial products
  //get the partial products
  numOfCSAs = width - 2;
  int layer = 0;
  
  vector <sigBlock> in;
  vector <sigBlock> out, temp_out;

  //print the header details
  
  cout	<<  "library IEEE;" << endl
	<<  "use IEEE.std_logic_1164.all;" << endl
	<<  "use IEEE.std_logic_arith.all;" << endl;
  
  //print the entity declaration
  
  cout	<< "entity walltree_mul is " << endl
	<< "\tport (" << endl
	<< "\t\top1\t:\tin\tstd_logic_vector(" << width-1 << " downto 0 );" << endl
	<< "\t\top2\t:\tin\tstd_logic_vector(" << width-1 << " downto 0 );" << endl
	<< "\t\tresult\t:\tout\tstd_logic_vector(" << 2*width << " downto 0 )" << endl
	<< "\t);" << endl
	<< "end walltree_mul;" << endl;
  
  //begin of architecture
  
  cout	<< "architecture rtl of walltree_mul is" << endl ;
  
  cout 	<< "-- signals for the partial products" << endl<<endl;
  
  for (int i = 0; i < width; i++ ){
    cout << "\tsignal pp" << i << " : std_logic_vector(" << width-1 << " downto 0 );"<< endl;
  }
  
  cout << endl;
  cout <<"-- internal signals"<< endl;
  
  for (int i = 0; i < ip ; i++){
    
    sigBlock s;
    s.width = ip;
    s.weightage = i;
    s.signame = "pp" + intToString(i);
    in.push_back(s);
  }
  
  while (in.size() >= 3){
    layer++;
    cout << endl << "-- layer : " << layer << endl << endl;
    
    for (unsigned int i = 0; i < in.size()-1; i=i+3){
      
      if (((i+1) > (in.size() -1)) or ((i+2) > (in.size() -1)))
	break;
      
      temp_out = generateCSA(in[i], in[i+1], in[i+2]);
      
      for (unsigned int i = 0; i < temp_out.size(); i++){
	out.push_back(temp_out[i]);
      }
      temp_out.clear();
    }
    
    for (unsigned int i = in.size()-(in.size()%3); i < in.size(); i++ ){
      out.push_back(in[i]);
    }
    
    in.clear();
    for (unsigned int i = 0; i < out.size(); i++){
      in.push_back(out[i]);
    }
    out.clear();
    
  }
  
  for (unsigned int i = 0; i < in.size(); i++){
    cout << "--" << in[i].signame << endl;
  }
  
  if(generatedCSAs == numOfCSAs)
    cout << "-- all csa structures generated" << endl;
  else
    return -1;

  cout 	<< "-- carry signal for the final RCA" << endl << endl
	<< "\tsignal carry_rca\t:\tstd_logic_vector( " << in[0].width-2 << " downto 0 );" << endl;
  
  
  cout	<< "begin" << endl << endl;
  
  for (int i = 0; i < ip ; i++){
    for (int j = 0; j < ip; j++){
      cout << "\tpp" << i << "(" << j << ")"<< " <= " << "op1" << "(" << i << ")" << " and " << "op2" << "(" << j << ");" << endl;
    }
  }
  
  cout << "-- ******************" << endl;
  
  printCSA();
  
  cout	<< "-- ******************" << endl
	<< "-- the final output" << endl;
  
  printRCA(in[0], in[1]);
  cout	<< "end rtl;";
  return err;
}

vector <sigBlock> generator::generateCSA(sigBlock s1, sigBlock s2, sigBlock s3){
  
  vector <sigBlock> out;
  csa adder;
  int temp_width1 = 0, temp_width2 = 0;
  
  adder.idx = generatedCSAs;
  adder.in1 = s1;
  adder.in2 = s2;
  adder.in3 = s3;
  
  if ((s1.weightage <= s2.weightage) and (s2.weightage <= s3.weightage)){
    adder.sum.weightage = s1.weightage; //min weightage
    adder.cry.weightage = s1.weightage + 1;
    
    temp_width1 = s2.width + (s2.weightage - s1.weightage);
    temp_width2 = s3.width + (s3.weightage - s1.weightage);
    
    if(temp_width1 >= temp_width2) {
      if (temp_width1 > s1.width)
	adder.sum.width = temp_width1;
      adder.cry.width = temp_width1;
    }
    else if (temp_width2 > s1.width) {
      adder.sum.width = temp_width2;
      adder.cry.width = temp_width2;
    }

    adder.sum.signame = "sigCSA_sum_" + intToString(generatedCSAs);
    adder.cry.signame = "sigCSA_cry_" + intToString(generatedCSAs);
  }

  cout << "-- csa : " << generatedCSAs << endl;
  cout << "\tsignal " << adder.sum.signame << " : std_logic_vector (" << adder.sum.width -1 << " downto 0);" << endl;
  cout << "\tsignal " << adder.cry.signame << " : std_logic_vector (" << adder.cry.width -1 << " downto 0);" << endl;

  csaList.push_back(adder);
  out.push_back(adder.sum);
  out.push_back(adder.cry);

  generatedCSAs++;

  return out;
}

void generator::printCSA(){

  for (unsigned int i = 0; i < csaList.size(); i++){
    cout << "-- csa : " << i << endl;

    //padding
    vector <string> input1, input2, input3;
    for (int j = 0; j < csaList[i].sum.width; j++){

      if(j < csaList[i].in1.width)
	input1.push_back(csaList[i].in1.signame + "(" + intToString(j) + ")");
      else
	input1.push_back("'0'");
    }

    for (int j = 0; j < (csaList[i].in2.weightage - csaList[i].in1.weightage ); j++)
      input2.push_back("'0'");

    for (int j = 0; j < csaList[i].in2.width; j++)
      input2.push_back(csaList[i].in2.signame + "(" + intToString(j) + ")");

    for (int j = csaList[i].in2.width; j < csaList[i].sum.width; j++ )
      input2.push_back("'0'");

    for (int j = 0; j < csaList[i].sum.width; j++){

      if(j < (csaList[i].in3.weightage - csaList[i].in1.weightage))
	input3.push_back("'0'");
      else if (j < ((csaList[i].in3.weightage - csaList[i].in1.weightage) + csaList[i].in3.width))
	input3.push_back(csaList[i].in3.signame + "(" + intToString(j - (csaList[i].in3.weightage - csaList[i].in1.weightage)) + ")");
      else
	input3.push_back("'0'");
    }

    cout << "-- generating " << csaList[i].sum.signame << " and " << csaList[i].cry.signame << endl << endl;

    for (int l = 0; l < csaList[i].sum.width; l++){
      cout << "\t" << csaList[i].sum.signame << "(" << l << ")" << " <= " << input1[l] << " xor " << input2[l] << " xor " << input3[l] << " ;" << endl;
      cout << "\t" << csaList[i].cry.signame << "(" << l << ")" << " <= " << "( " << input1[l] << " and " << input2[l] << " )" << " or " << "( "<< input3[l] << " and " << "( " << input1[l] << " xor " << input2[l] << " )) ;" << endl;
    }
  }
}

void generator::printRCA(sigBlock sum, sigBlock cry){

  cout << "\tresult(0) <= " << sum.signame << "(0);" << endl;
  int i = 1;
  for (i = 1; i < sum.width; i++){
    if (i == 1) {
      cout << "\tresult(" << i << ") <= " << sum.signame << "(" << i << ") xor " << cry.signame << "(" << i-1 << ") xor '0' ;"  << endl;
      cout << "\tcarry_rca(" << i-1 << ") <= " << sum.signame << "(" << i << ") and " << cry.signame << "(" << i-1 << ");" << endl;
      continue;
    }

    cout << "\tresult(" << i << ") <= " << sum.signame << "(" << i << ") xor " << cry.signame << "(" << i-1 << ") xor carry_rca(" << i-2 << ");" << endl;
    cout << "\tcarry_rca(" << i-1 << ") <= " << "( " << sum.signame << "(" << i << ") and " << cry.signame << "(" << i-1 << ")) or ( carry_rca(" << i-2 << ") and " << "( " << sum.signame << "(" << i << ") xor " << cry.signame << "(" << i-1 << ")));" << endl;
    if (i == 2*width) break;
  }

  if (i <= 2*width)
    cout << "\tresult(" << i << ") <= " << cry.signame << "(" << i-1 << ") xor carry_rca(" << i-2 << ");" << endl;

  if (i < 2*width)
    cout << "\tresult(" << i+1 << ") <= " << cry.signame << "(" << i-1 << ") and carry_rca(" << i-2 << ");" << endl;

}
