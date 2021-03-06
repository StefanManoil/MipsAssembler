;Write a program that will be executed with an array of 32-bit integers (with register 1 holding the address of the beginning of the array, and register 2 holding 
;the number of elements in the array, as before). Your program should call your integer-printing procedure from Part A for each integer in the array in turn, 
;to print all the integers, and return. The program should simply return if the array is of length 0.
;Example:

;Enter length of array: 3
;Enter array element 0: 1
;Enter array element 1: 2
;Enter array element 2: 3
;1
;2
;3
;MIPS program completed normally.

lis $4
.word 4 ; constant 4
add $3, $1, $0 ; index 0 of array
mult $4, $2
mflo $8
add $8, $8, $1 ; address right after last element in array

loop:
beq $3, $8, endMain
; push $31 on stack
sw $31, -4($30) ; store return address
sub $30, $30, $4

lw $1, 0($3)
lis $2
.word print
jalr $2
add $3, $3, $4

; pop $31 from stack
add $30, $30, $4
lw $31, -4($30)
beq $0, $0, loop

endMain:

jr $31


; your code for print procedure should be added here
print:
; store on the stack all registers we will use
sw $1, -4($30)
sw $2, -8($30)
sw $3, -12($30)
sw $4, -16($30)
sw $5, -20($30)
sw $6, -24($30)
sw $7, -28($30)
sw $8, -32($30)
lis $2
.word 32
sub $30, $30, $2

lis $4
.word 0xffff000c
slt $2, $1, $0
beq $2, $0, positiveByDefault
lis $2
.word 45 ; ASCII for '-'
sw $2, 0($4) ; print '-'
sub $1, $0, $1 ; negate $1

positiveByDefault:
lis $5 ; constant 10 used in division
.word 10
add $3, $0, $0 ; counter of digits
lis $6 ; constant 4
.word 4
lis $7 ; constant 1
.word 1
lis $8
.word 48 ; constant value of 48 representing ASCII '0'

loop1:
beq $1, $0, loop2
divu $1, $5 ; divide $1 by 10
mflo $1 ; store the quotient in $1, thus modifying it
mfhi $2 ; store remainder in $2, rightmost digit
add $3, $3, $7 ; increment counter
sw $2, -4($30) ; store remainder
sub $30, $30, $6 ; update stack pointer
beq $0, $0, loop1

loop2:
beq $3, $0, end
lw $2, 0($30) ; load remainder at top of stack into $2
add $30, $30, $6 ; move the stack pointer 4 bytes down
add $2, $2, $8 ; add the remainder value to constant 48 to get the ASCII of exact digit required
sw $2, 0($4) ; print the remainder
sub $3, $3, $7 ; decrement counter
beq $0, $0, loop2

end:
lis $2
.word 10
sw $2, 0($4) ; print newline character

; restore all original register values from the stack for the caller
lis $2
.word 32
add $30, $30, $2
lw $1, -4($30)
lw $2, -8($30)
lw $3, -12($30)
lw $4, -16($30)
lw $5, -20($30)
lw $6, -24($30)
lw $7, -28($30)
lw $8, -32($30)
jr $31
