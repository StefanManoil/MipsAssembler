;Register 1 holds a 32-bit integer (in two's complement notation). Your program should format this integer in base 10, print it, then print a newline character, and return.
;Example:

;Enter value for register 1: 5
;Enter value for register 2: 3
;Running MIPS program.
;5
;$01 ......
;Another Example:

;Enter value for register 1: -55
;Enter value for register 2: 3
;Running MIPS program.
;-55
;$01 ......

;Enter value for register 1: -257257
;Enter value for register 2: 3
;Running MIPS program.
;-
;$01 ......

; First we will see if the number is negative, if it is, we will print '-' and then take the absolute value of the number, if not we won't do anything.
; Next, the idea will be to divide the number (positive now) by 10 repeatedly while keeping the remainder at each step. We will be dividing $1 by 10 until the
; $1 is 0, at which point we won't go into the loop. With the remainders, we will store them on the free memory stack until the loop is done. We will use a counter
; to keep track of how many numbers we put on the stack, aka how many digits in the original number. Once the loop is done, we will load the remainders off the stack by using
; a loop that will simply iterate #number of digit times and in each iteration we load a number off the stack storing it in a temp register, then we print it.
; At the end of the loop, we have printed our number and we can then print a newline character and return.

; $2 will be used as the absolute value determiner, $4 will be the output address, $5 will have a constant value of 10,
; we will be modifying $1 as we change it as we divide by 10, we will store the remainders in $2, $3 will be our counter,
; keeping track of how many things we put on the stack, $2 will be our temp register when we load remainders off the stack. Of course we will also be using $30, and
; $4 at this point will hold the output address.

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
jr $31
