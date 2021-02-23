;Register 1 holds the address of the beginning of an array of 32-bit two's complement integers. Register 2 holds the number of elements in the array. Determine the maximum of all the elements of the array, write it into register 3, and return. Assume the array is not empty.
;Example:

;Enter length of array: 5
;Enter array element 0: 32
;Enter array element 1: 24
;Enter array element 2: 54
;Enter array element 3: 9
;Enter array element 4: 0
;MIPS program completed normally.
;$01 = 0x00000060   $02 = 0x00000005   $03 = 0x00000036   $04 = 0xffffffff
;...
;Note: Register 3 contains 0x36 which is 54 in hexadecimal.

; We know the length of the array is held in register 2 and that register 1 holds the starting address for the array.
; We need to store the max of all elements in array in register 3.
; We will use register 3 as our currentMax, register 5 as our current index, register 6 as our current element,
; register 7 as the 1 or 0 for comparison using slt where we compare our current elemnent and the currentMax, and register 8 as the address in memory right after the last
; element in the array, essentially 4 * lengthArr. Let's also use register 4 which will just be the constant 4.
; By the end of the loop, the actual max of all array elements will be stored in register 3.

; First is setup
; we can immediately set our currentMax to arr[0] and then make the currentIndex 1 since we will start our loop there, also set register 8 to (4 * lengthArr) + beginningAddress
lw $3, 0($1)
lis $4
.word 4
add $5, $1, $4
mult $4, $2
mflo $8
add $8, $8, $1

loop:
beq $5, $8, end ; if we reached beyond the loop check, don't go in it again
lw $6, 0($5) ; loads the current element using the current index
; -- do max comparison between currentMax and currentIndex
slt $7, $3, $6
beq $7, $0, 1
add $3, $6, $0
; --
add $5, $5, $4
beq $0, $0, loop

end:
jr $31
