/*=============================================================================
| CDA3103 – Computer Logic and Organization
| This program was completed by: Edelis Molina
| Write the core part of a mini processor simulator called MySPIM
|=============================================================================*/
#include "spimcore.h"


/* ALU */
/* 10 Points */
void ALU(unsigned A, unsigned B, char ALUControl, unsigned *ALUresult, char *Zero)
{
  switch((int)ALUControl) //typecast to int
  {
    // Z = A + B
    case 0:
      *ALUresult = A + B;
      break;
    // Z = A - B
    case 1:
      *ALUresult = A - B;
      break;
    // if A < B, Z = 1; otherwise, Z = 0
    case 2:
      if((signed)A < (signed)B)
        *ALUresult = 1;
      else
        *ALUresult = 0;
      break;
    // if A < B, Z = 1; otherwise, Z = 0 (A and B are unsigned integers)
    case 3:
      if(A < B)
        *ALUresult = 1;
      else
        *ALUresult = 0;
      break;
    // Z = A AND B
    case 4:
      *ALUresult = A & B;
      break;
    // Z = A OR B
    case 5:
      *ALUresult = A | B;
      break;
    // Shift left B by 16 bits
    case 6:
      *ALUresult = B << 16;
      break;
    // Z = NOT A
    case 7:
      *ALUresult = ~A;
      break;

  } // End of switch

  // Takes care of Zero bit. Only if all the bits of the result are 0, Zero will be 1
  if(*ALUresult == 0)
    *Zero = 1;
  else
    *Zero = 0;
}


/* instruction fetch */
/* 10 Points */
int instruction_fetch(unsigned PC,unsigned *Mem,unsigned *instruction)
{
  // Memory is byte addressed and to access it we need to multiply by 4 (4 bytes beyond the based address of PC)
  // Multiply by 4 is the same as Shiftleft by 2

  // Invalid word alignment
  if(PC % 4 != 0)
    return 1; // set halt condition to 1

  // An illegal instruction
  if(Mem[(PC >> 2)] == 0)
    return 1; // set halt condition to 1

  // Return instruction from memory
  *instruction = Mem[(PC >> 2)];

  return 0;
}


/* instruction partition */
/* 10 Points */
void instruction_partition(unsigned instruction, unsigned *op, unsigned *r1,unsigned *r2, unsigned *r3, unsigned *funct, unsigned *offset, unsigned *jsec)
{
  // See MIPS Reference PPoint for R-Type, I-Type and J-Type instructions formats
  // Partition the 32 bits instruction by shifting the bits and bitmask to access specific bits
  *op = (instruction >> 26) & 0x0000003f; // bits [31-26]. 0000 0000 0000 0000 0000 0000 0011 1111 
  *r1 = (instruction >> 21) & 0x0000001f; // bits [25-21]. 0000 0000 0000 0000 0000 0000 0001 1111
  *r2 = (instruction >> 16) & 0x0000001f; // bits [20-16]. 0000 0000 0000 0000 0000 0000 0001 1111
  *r3 = (instruction >> 11) & 0x0000001f; // bits [15-11]. 0000 0000 0000 0000 0000 0000 0001 1111
  *funct = instruction & 0x0000003f;      // bits [5 - 0]. 0000 0000 0000 0000 0000 0000 0011 1111
  *offset = instruction & 0x0000ffff;     // bits [15 -0]. 0000 0000 0000 0000 1111 1111 1111 1111
  *jsec = instruction & 0x03ffffff;       // bits [25 -0]. 0000 0011 1111 1111 1111 1111 1111 1111

  // printf("Op %x\n", *op);
  // printf("r1 %x\n", *r1);
  // printf("r2 %x\n", *r2);
  // printf("r3 %x\n", *r3);
  // printf("funct %x\n", *funct);
  // printf("offset %x\n", *offset);
  // printf("jset %x\n", *jsec);
}


/* instruction decode */
/* 15 Points */
int instruction_decode(unsigned op, struct_controls *controls)
{
  // Set eah of the signals in the Control Unit based on the instruction's op code
  switch(op)
  {
    case 0:  // 000000. R-Type instruction (add, sub, and, or, slt, sltu) Total of 6
      controls->RegDst = 1;      // Write Register [15-11]
      controls->Jump = 0; 
      controls->Branch = 0;
      controls->MemRead = 0;     // not lw. not reading from Data Memory
      controls->MemtoReg = 0;    // ALU result from the ALU will be sent to write data register
      controls->ALUOp = 7;       // 111. R-type instruction. further decoding of the secondary source: Funct bits [5-0]
      controls->MemWrite = 0;    // not sw. not writing to Data Memory
      controls->ALUSrc = 0;      // determine 2nd input of ALU. Choose read data 2
      controls->RegWrite = 1;    // permission to write to the register file
      break;

    case 2: // 000010. J-Type instruction (jump)
      controls->RegDst = 0;
      controls->Jump = 1;
      controls->Branch = 0;
      controls->MemRead = 0;
      controls->MemtoReg = 0;
      controls->ALUOp = 0;
      controls->MemWrite = 0; 
      controls->ALUSrc = 0; 
      controls->RegWrite = 0;
      break;

    case 4: // 0001000. (beq)
      controls->RegDst = 2;       // don't care
      controls->Jump = 0;
      controls->Branch = 1;       // branching
      controls->MemRead = 0;
      controls->MemtoReg = 2;     // don't care
      controls->ALUOp = 1;        // subtract
      controls->MemWrite = 0;
      controls->ALUSrc = 0;
      controls->RegWrite = 0;
      break;

    case 8: // 001000. I-Type instruction (addi)
      controls->RegDst = 0;     
      controls->Jump = 0;
      controls->Branch = 0;
      controls->MemRead = 0;
      controls->MemtoReg = 0;   // ALU result from the ALU will be sent to write data input of the register file register
      controls->ALUOp = 0;      // ALU will do addition
      controls->MemWrite = 0;
      controls->ALUSrc = 1;     // Instruction [15-0] sign-extended (32 bits) immediate val is the 2nd input of the ALU
      controls->RegWrite = 1;   // permission to write to the register file
      break;

    case 10: // 001010. slti
      controls->RegDst = 0;   
      controls->Jump = 0;  
      controls->Branch = 0;
      controls->MemRead = 0;
      controls->MemtoReg = 0;   // ALU result from the ALU will be sent to write data input of the register file register
      controls->ALUOp = 2;      // ALU will do “set less than” operation
      controls->MemWrite = 0;
      controls->ALUSrc = 1;     
      controls->RegWrite = 1;   // permission to write to the register file
      break;

    case 11: // 001011. I-Type instruction (sltiu, set on less than imm unsigned)
    	controls->RegDst = 0;     
      controls->Jump = 0;
      controls->Branch = 0;
      controls->MemRead = 0;
      controls->MemtoReg = 0;   // ALU result from the ALU will be sent to write data input of the register file register
      controls->ALUOp = 3;      // ALU will do “set less than unsigned” operation
      controls->MemWrite = 0;
      controls->ALUSrc = 1;     // Instruction [15-0] sign-extended (32 bits) immediate val is the 2nd input of the ALU
      controls->RegWrite = 1;   // permission to write to the register file
      break;
        
    case 15: // 001111. I-Type instruction (load upper immediate, lui)
      controls->RegDst = 0;     // Write register is Instruction[20-16]
      controls->Jump = 0;
      controls->Branch = 0;
      controls->MemRead = 0;   // no lw
      controls->MemtoReg = 0;
      controls->ALUOp = 6;     // ALU will shift left extended_value by 16 bits
      controls->MemWrite = 0;  // no sw
      controls->ALUSrc = 1;    // Instruction [15-0] sign-extended (32 bits) immediate val is the 2nd input of the ALU
      controls->RegWrite = 1;
      break;

    case 35: // 100000. I-Type instruction (lw)
      controls->RegDst = 0;     // Write register is Instruction[20-16]
      controls->Jump = 0;
      controls->Branch = 0;
      controls->MemRead = 1;    // permission to read from Data Memory
      controls->MemtoReg = 1;   // value from Data Memory will be sent to the register file
      controls->ALUOp = 0;      // add base address and the offset to get final mem address
      controls->MemWrite = 0; 
      controls->ALUSrc = 1;     // Instruction [15-0] sign-extended (32 bits) immediate val is the 2nd input of the ALU
      controls->RegWrite = 1;   // permission to write to the register file
      break;

    case 43: // 101000. I-Type instruction (sw)
    	controls->RegDst = 2;     // don't care
      controls->Jump = 0;
      controls->Branch = 0;
      controls->MemRead = 0;
      controls->MemtoReg = 2;   // don't care
      controls->ALUOp = 0;      // add base address and the offset to get final mem address
      controls->MemWrite = 1;   // permission to write to Data Memory
      controls->ALUSrc = 1;     // Instruction [15-0] sign-extended (32 bits) immediate val is the 2nd input of the ALU
      controls->RegWrite = 0;
      break;

    default:
      return 1; // halt 

  } // end of switch

  return 0; // SUCCESS

}


/* Read Register */
/* 5 Points */
void read_register(unsigned r1, unsigned r2, unsigned *Reg, unsigned *data1, unsigned *data2)
{
  *data1 = Reg[r1];
	*data2 = Reg[r2];
}


/* Sign Extend */
/* 10 Points */
void sign_extend(unsigned offset, unsigned *extended_value)
{
  unsigned flag = offset >> 15; // get the MSB of the offset

  // if MSB == 1 (negative), extend with 1's. Pad with 1's the first 16 bits
  if(flag == 1)
    *extended_value = offset | 0xFFFF0000;  // 1111 1111 1111 1111 0000 0000 0000 0000

  // extend with 0's
  else
    *extended_value = offset & 0x0000FFFF; // 0000 0000 0000 0000 1111 1111 1111 1111
}


/* ALU operations */
/* 10 Points */
int ALU_operations(unsigned data1, unsigned data2, unsigned extended_value, unsigned funct, char ALUOp, char ALUSrc, unsigned *ALUresult, char *Zero)
{
  // if ALUOp is 7 (R-Type Instruction), do further decoding of the function field and set ALUOp accordingly 
  if(ALUOp == 7)
  {
    switch(funct)
    {
      case 32: // 10 0000 add
        ALUOp = 0;
        break;

      case 34: // 10 0010 subtract
        ALUOp = 1;
        break;

      case 42: // 10 1010 set on less than signed
        ALUOp = 2;
        break;

      case 43: // 10 1011 set on less than unsigned
        ALUOp = 3; 
        break;

      case 36: // 10 0100 AND
        ALUOp = 4;
        break;

      case 37: // 10 0101 OR
        ALUOp = 5;
        break;

      default: // return 1 is a halt condition occurs
        return 1; 
    }
  }

  // if ALUSrc is 1, the 2nd input of the ALU will be the extended_value
  if(ALUSrc == 1)
    ALU(data1, extended_value, ALUOp, ALUresult, Zero);
  // if ALUSrc is 0, the 2nd input of the ALU will be Read Data 2
  else 
    ALU(data1, data2, ALUOp, ALUresult, Zero);

  return 0; // don't halt   
}


/* Read / Write Memory */
/* 10 Points */
int rw_memory(unsigned ALUresult, unsigned data2,char MemWrite,char MemRead,unsigned *memdata,unsigned *Mem)
{
  // if ALUresult (address) when reading or writing is not word aligned, halt
  if((MemRead == 1 | MemWrite == 1) && ALUresult % 4 != 0)
    return 1; // halt

  // load word. Read content of the mem location addressed by ALUresult to memdata
  if(MemRead == 1)
    *memdata = Mem[ALUresult >> 2];

  // store word. Write the value of data2 into the mem location addressed by ALUresult
  if(MemWrite == 1)
    Mem[ALUresult >> 2] = data2;

  return 0; // don't halt
}


/* Write Register */
/* 10 Points */
void write_register(unsigned r2, unsigned r3, unsigned memdata, unsigned ALUresult, char RegWrite, char RegDst, char MemtoReg, unsigned *Reg)
{
  if(RegWrite == 1) // allow writing to the register file
  {
    // Write ALUResult (MemtoReg == 0) to register 3 (RegDest == 1, [15-11]) for R-Type instructions
    if(MemtoReg == 0 && RegDst == 1)
      Reg[r3] = ALUresult;
    
    // For immmediate instructions (addi, lui, slti, sltui). 
    // Write ALUResult (MemtoReg == 0) to register 2 (RegDest == 0, [20-16]) for I-Type instructions
    if(MemtoReg == 0 && RegDst == 0)
      Reg[r2] = ALUresult;

    // Send value from Data Memory (MemtoReg == 1) to register 2 (RegDest == 0, [20-16]) for load word
    if(MemtoReg == 1 && RegDst == 0)
      Reg[r2] = memdata;
  }
}



/* PC update */
/* 10 Points */
void PC_update(unsigned jsec,unsigned extended_value,char Branch,char Jump,char Zero,unsigned *PC)
{
  // Next instruction (add 4)
  *PC = *PC + 4;

  // if controls->Branch == 1 and Zero (ALU) == 1, add extended_value shift left 2 (multiply by 4)
  if(Branch == 1 && Zero == 1)
    *PC += extended_value << 2;

  // if Jump == 1, take bits [25 -0] of the instruction and shift left 2 and concatenate with upper 4 bits of PC
  if(Jump == 1)
    *PC = (jsec << 2) | (*PC & 0xf0000000);
}

