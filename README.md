# MipsAssembler

A fully functional assembler that converts a dialect of 32-bit MIPS assembly language to machine code.

It is effectively a two-pass assembler where the Analysis stage occurs on the first pass. Here we scan tokens using the maximal munch algorithm, 
construct our symbol table, and build up intermediate representations which we will then pass on to the Synthesis stage.

In the Synthesis stage we produce our output from the intermediate representations by constructing appropriate MIPS binary machine code.

If the input to the assembler is valid, it will produce the corresponding machine code nicely and you effectively have a MIPS program you can play around with.
If the input is not valid, the assembler will return an error message the moment it encounters the fault.

I hope to add a more robust testing methodology but for now, if you have a C++ compiler you can run this on a terminal to compile:

g++ -g -std=c++14 -Wall asm.cc scanner.cc -o asm

and then run the executable with any of the .asm files as input like so:

./asm < goodTest1.asm

The specs for the particular dialect of MIPS is as follows:

## MIPS Assembly Language
A MIPS Assembly program is a Unix text file consisting of a number of lines. Each line has the general format
    labels instruction comment
Each of these components -- labels, instruction, and comment is optional; a particular line may have all three, any two, any one, or none at all.
Every line with an instruction specifies a corresponding machine language instruction word. Lines without an instruction are called null lines and do not specify an instruction word. That is, an assembly language program with n non-null lines specifies a machine language program with n words, in 1-1 ordered correspondence.

## Labels
The labels component lists zero or more labels, each followed by a colon (:). A label is a string of alphanumeric characters, the first of which must be a letter of the alphabet. For example, fred123x is a valid label but 123fred is not.
A label appearing in the labels component is said to be defined; a particular label may be defined at most once in an assembly language program. Labels are case-sensitive; that is, fred and Fred are distinct labels.

The location of a line in an assembly language program is 4n, where n is the number of non-null lines preceding it. The first line therefore has location 0. If the first line is non-null, the second line has location 4. On the other hand, if the first line is null, the location of the second line is also 0. Note that the location of any non-null line is exactly the same as the location of the machine language word that it specifies. And all null lines immediately preceding it have the same location.

The value of a label is defined to be the location of the line on which it is defined.

## Comments
A comment is any sequence of characters beginning with a semicolon (;) and ending with the end-of-line character that terminates the line. Comments have meaning only to the reader; they do not contribute to the specification of the equivalent machine language program.

## Instructions
An instruction takes the form of an opcode followed by one or more operands. The opcode may be add, sub, mult, multu, div, divu, mfhi, mflo, lis, lw, sw, slt, sltu, beq, bne, jr, jalr, or the pseudo-opcode .word. The number, allowed formats, and meaning of operands depend on the opcode. Operands are, unless otherwise specified, separated by commas (,). An operand may be
a register denoted $0, $1, $2, ... $31,
an unsigned or decimal integer denoted by a string of digits 0-9,
a negative decimal integer denoted by a minus sign (-) followed by an unsigned decimal integer,
a hexadecimal number denoted by 0x followed by a string of hexadecimal digits 0-9 or a-f or A-F,
a label.

## Operand Format - add, sub, slt, sltu
These opcodes all take three register operands; for example
   add $1, $2, $3
The first operand is $d (the destination register) as specified in the MIPS Reference Sheet. The second and third operands are $s and $t respectively. So in the example above we have d=1, s=2, and t=3, and the 5-bit representations of these values are encoded in the corresponding machine instruction.

## Operand Format - mult, multu, div, divu
These opcodes take two register operands corresponding to $s and $t. For example
   mult $4, $5
specifies that s=4 and t=5 are encoded in the instruction word. $d is not used and is encoded as 0 in the instruction word.

## Operand Format - mfhi, mflo, lis
These opcodes have a single register operand, $d.

## Operand Format - lw, sw
These opcodes have two register operands, $s and $t, and in addition an immediate operand, i. The general format is
   opcode $t, i($s)  
For example,
   lw $4, 400($7)
$s and $t are registers, and i may be a decimal number (possibly negative) or a hexadecimal number. i is encoded as a 16-bit two's complement integer. If specified in decimal, i must be in the range -32768 through 32767. If specified as hexadecimal i must not exceed 0xffff.

## Operand Format - beq, bne
These opcodes take three operands: registers $s and $t, and an immediate operand i. i may be specified as a decimal number (possibly negative), a hexadecimal number, or a label. If i is a decimal or hexadecimal number, i is encoded as a 16-bit two's complement number; i must therefore be in the range -32768 through 32767 if i is decimal, and must not exceed 0xffff if i is hexadecimal.
If i is a label, the value (i-L-4)/4 is encoded where i is the value of the label and L is the location of the beq or bne instruction. (i-L-4)/4 must be in the range -32768 through 32767.

## Operand Format -- jr, jalr
These opcodes have one register operand, $s.
Operand Format .word
.word is not a true opcode as it does not necessarily encode a MIPS instruction at all. .word has one operand i which is either a number or a label. If i is a decimal number, it must be in the range -2^31 through 2^32-1 (that is, the union of the ranges for signed two's complement and unsigned 32-bit integers). If i is hexadecimal, it must not exceed 0xffffffff. If a label is used for i, its value is encoded as an unsigned 32-bit integer. Although this technically imposes a limit on the maximum value of a label operand for .word, MIPS assemblers are not required to enforce this limit, since a program several gigabytes in size would be needed to reach it. (This applies to label operands for .word only; a MIPS assembler is expected to enforce all other limits on operand ranges mentioned in this document.)
