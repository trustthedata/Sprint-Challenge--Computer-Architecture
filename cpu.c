#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cpu.h"

/**
 * Push a value on the CPU stack
 */
void cpu_push(struct cpu *cpu, unsigned char val)
{
  cpu->reg[SP]--;

  cpu->ram[cpu->reg[SP]] = val;
}

/**
 * Pop a value from the CPU stack
 */
unsigned char cpu_pop(struct cpu *cpu)
{
  unsigned char val = cpu->ram[cpu->reg[SP]];

  cpu->reg[SP]++;

  return val;
}

/**
 * Load the binary bytes from a .ls8 source file into a RAM array
 */
void cpu_load(char *filename, struct cpu *cpu)
{
  FILE *fp;
  char line[256];
  int address = ADDR_PROGRAM_ENTRY;

  // Open the source file
  if ((fp = fopen(filename, "r")) == NULL)
  {
    fprintf(stderr, "Cannot open file %s\n", filename);
    exit(2);
  }

  // Read all the lines and store them in RAM
  while (fgets(line, sizeof line, fp) != NULL)
  {

    // Convert string to a number
    char *endchar;
    unsigned char byte = strtol(line, &endchar, 2);
    ;

    // Ignore lines from whicn no numbers were read
    if (endchar == line)
    {
      continue;
    }

    // Store in ram
    cpu->ram[address++] = byte;
  }
}

/**
 * ALU
 */
void alu(struct cpu *cpu, enum alu_op op, unsigned char regA, unsigned char regB)
{
  unsigned char *reg = cpu->reg;

  unsigned char valB = reg[regB];

  switch (op)
  {
  case ALU_MUL:
    reg[regA] *= valB;
    break;

  case ALU_ADD:
    reg[regA] += valB;
    break;
  }
}

/**
 * Run the CPU
 */
void cpu_run(struct cpu *cpu)
{
  // Just so we don't have to type cpu-> every time
  unsigned char *reg = cpu->reg;
  unsigned char *ram = cpu->ram;

  int running = 1; // True until we get a HLT instruction

  while (running)
  {
    // Get the value of the current instruction (in address PC)
    unsigned char IR = ram[cpu->PC];

    unsigned char operandA = ram[(cpu->PC + 1) & 0xff];
    unsigned char operandB = ram[(cpu->PC + 2) & 0xff];

    // True if this instruction might set the PC
    int instruction_set_pc = (IR >> 4) & 1;

    // switch() over it to decide on a course of action
    switch (IR)
    {

    case LDI:
      reg[operandA] = operandB;
      break;

    case PRN:
      printf("%d\n", reg[operandA]);
      break;

    case MUL:
      alu(cpu, ALU_MUL, operandA, operandB);
      break;

    case ADD:
      alu(cpu, ALU_ADD, operandA, operandB);
      break;

    case HLT:
      running = 0;
      break;

    case PRA:
      printf("%c\n", reg[operandA]);
      break;

    case PUSH:
      cpu_push(cpu, reg[operandA]);
      break;

    case POP:
      reg[operandA] = cpu_pop(cpu);
      break;

    //Compare the values in two registers.
    //If they are equal, set the flag to 1, otherwise set it to 0.
    case CMP:
      if (cpu->reg[operandA] == cpu->reg[operandB])
      {
        cpu->fl = 1;
      }
      else
      {
        cpu->fl = 0;
      }
      break;

    //JMP register - Set the program counter (PC) to the address stored in the given register.
    case JMP:
      cpu->PC = cpu->reg[operandA];
      break;

    // If `equal` flag is set (true), jump to the address stored in the given register.
    case JEQ:
      if (cpu->fl == 1)
      {
        cpu->PC = cpu->reg[operandA];
      }
      else
      {
        cpu->PC += 2;
      }
      break;

    // If flag is clear (false, 0), jump to the address stored in the given register.
    case JNE:
      if (cpu->fl == 0)
      {
        cpu->PC = cpu->reg[operandA];
      }
      else
      {
        cpu->PC += 2;
      }
      break;

    default:
      fprintf(stderr, "PC %02x: unknown instruction %02x\n", cpu->PC, IR);
      exit(3);
    }

    if (!instruction_set_pc)
    {
      cpu->PC += ((IR >> 6) & 0x3) + 1;
    }
  }
}

/**
 * Initialize a CPU struct
 */
void cpu_init(struct cpu *cpu)
{
  cpu->PC = 0;

  // Zero registers and RAM
  memset(cpu->reg, 0, sizeof cpu->reg); // filling the reg array defined in cpu.h with zeros
  memset(cpu->ram, 0, sizeof cpu->ram); // filling the ram array defined in cpu.h with zeros

  // Initialize SP
  cpu->reg[SP] = ADDR_EMPTY_STACK;
  cpu->fl = 0; //initialize the flag to start at 0
}