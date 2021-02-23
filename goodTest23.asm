;Register 1 holds the address of the beginning of an array of 32-bit integers, each representing a character. The integer zero represents a space, 
;and each integer i (1 <= i <= 26) represents the i'th letter of the uppercase alphabet. Register 2 holds the number of elements in the array (can be empty). 
;Your program should output the uppercase characters represented by the integers in the array, and return. The MIPS system allows you to output one character at a time, 
;by storing its ASCII value into the special memory location 0xffff000c (hex).

;(Hint: you may find the command man ascii helpful.)

;Example:

;Enter length of array: 11
;Enter array element 0: 8
;Enter array element 1: 5
;Enter array element 2: 12
;Enter array element 3: 12
;Enter array element 4: 15
;Enter array element 5: 0
;Enter array element 6: 23
;Enter array element 7: 15
;Enter array element 8: 18
;Enter array element 9: 12
;Enter array element 10: 4
;HELLO WORLDMIPS program completed normally.
;$01 = 0x0000006c   $02 = 0x0000000b   $03 = 0x00000000   $04 = 0x00000004
;...
;The above is what you might see if both standard output (where "HELLO WORLD" is sent) and standard error (where the MIPS register contents are sent) are 
;sent to the same place. You might want to redirect them to different places when testing.

; We know the length of the array is held in register 2 and that register 1 holds the starting address for the array.
; We simply need to loop through the array and output the corresponding characters based on the integers.
; We will use register 3 as our current index, register 4 as the constant 4, register 5 as our current element and output ascii value, register 6 will hold the output address,
; register 7 will hold the constant 64 which is the ASCII value right before the uppercase letter 'A', register 8 will hold (4 * lengthArr) + beginningAddress.

;setup
lis $6
.word 0xffff000c ; output address
lis $7
.word 64 ; decimal value representing ASCII just before 'A'
add $3, $1, $0 ; index 0
lis $4
.word 4
mult $4, $2
mflo $8
add $8, $8, $1 ; address right after last element in array


beq: beq $3, $8, end
lw $5, 0($3) ; loads the current element using the current index
; -- check if the current element is 0, if it is output a space (32 dec in ASCII), if not, change register 5 to $5 + $7 to get the corresponding letter and output this
bne $5, $0, notZero
lis $5
.word 32
sw $5, 0($6) ; output a space
beq $0, $0, endOfLoop ; jump to end of the loop
notZero:
add $5, $5, $7
sw $5, 0($6) ; output the corresponding uppercase character
; --
endOfLoop:
add $3, $3, $4
beq $0, $0, beq

end:
jr $31
