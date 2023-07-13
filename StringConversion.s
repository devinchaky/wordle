; StringConversion.s
; Student names: Devin Chaky
; Last modification date: 3/19/23
; Runs on TM4C123
; ECE319K lab 7 number to string conversions
;		
; Used in ECE319K Labs 7,8,9,10. You write these two functions

 
    EXPORT   Dec2String
    EXPORT   Fix2String
    AREA    |.text|, CODE, READONLY, ALIGN=2
    THUMB
    PRESERVE8

;-----------------------Dec2String-----------------------
; Convert a 32-bit number into unsigned decimal format
; String the string into the empty array add null-termination
; Input: R0 (call by value) 32-bit unsigned number
;        R1 pointer to empty array
; Output: none
; Invariables: This function must not permanently modify registers R4 to R11
i equ 0		; local variables 
num equ 4	
	
FP RN 11
Dec2String
	PUSH {R4, R11}	; save scratch reg R4, R5

	SUB SP, #8	; allocate 8 bytes on stack for local variables
	MOV FP, SP	; init frame pointer
	
	MOV R2, #0	; init i as 0 --> will be count of digits
	STR R2, [FP, #i]
	
	STR R0, [FP, #num]	; store input as local variable
	
	LDR R2, [FP, #num]  ; copy of num
	MOV R4, #10
CountDigits
	UDIV R2, R4	; divide by 10 to count digits
	
	LDR R3, [FP, #i]	; increment counter i for each digit
	ADD R3, #1
	STR R3, [FP, #i]
	
	CMP	R2, #0	; check if all digits counted
	BNE CountDigits
	
	LDR R3, [FP, #i]	; load number of digits
	MOV R2, #0		; store null at end of array
	STR R2, [R1, R3]
	
	LDR R2, [FP, #i]	; decrement array index/counter
	SUB R2, #1
	STR R2, [FP, #i]	

Loop
	LDR R2, [FP, #num]	; num % 10
	MOV R3, #10
	UDIV R4, R2, R3
	MUL R4, R3
	SUB R2, R2, R4
	ADD R2, #0x30
	LDR R4, [FP, #i]
	STRB R2, [R1, R4]	; store ascii val of lsd in array
	
	LDR R2, [FP, #num]	; divide num by 10
	UDIV R2, R3
	STR R2, [FP, #num]
	
	LDR R2, [FP, #i]	; decrement counter
	SUB R2, #1
	STR R2, [FP, #i]
	
	LDR R2, [FP, #num]	; check if every digit has been stored in array
	CMP R2, #0
	BNE Loop

	ADD SP, #8
	POP {R4, R11} ; resore R4, R11
    BX LR
;* * * * * * * * End of Dec2String * * * * * * * *


; -----------------------Fix2String----------------------
; Create characters for LCD display in fixed-point format
; unsigned decimal, resolution 0.001, range 0.000 to 9.999
; Inputs:  R0 is an unsigned 32-bit number
;          R1 pointer to empty array
; Outputs: none
; E.g., R0=0,    then create "0.000 "
;       R0=3,    then create "0.003"
;       R0=89,   then create "0.089"
;       R0=123,  then create "0.123"
;       R0=9999, then create "9.999"
;       R0>9999, then create "*.***"
; Invariables: This function must not permanently modify registers R4 to R11
offset equ 0
d equ 4
number equ 8

FP RN 11

Fix2String
	PUSH {R4, R11}
	SUB SP, #12		; allocate space for 3 local variables (12 bytes)
	MOV FP, SP ; init frame pointer
	
	MOV R2, #0
	STR R2, [FP, #offset] ; init array offset local var to 5
	
	STR R0, [FP, #number]	; init input as local var
	
	MOV R2, #9999 ; if num > 9999 invalid input
	CMP R0, R2	; check if invalid input
	BLS Valid	; if valid skip to valid
	
	MOV R2, #0x2A	; return "*.***"
	LDR R3, [FP, #offset]
	STRB R2, [R1, R3]	; store * as first element
	ADD R3, #1		; increment offset to next element
	STR R3, [FP, #offset]	
	
	MOV R2, #0x2E	; store decimal point in array
	LDR R3, [FP, #offset]
	STRB R2, [R1, R3]
	ADD R3, #1		; increment offset
	STR R3, [FP, #offset]
	
	MOV R2, #0x2A
Loop1		; loop store "***" in array
	LDR R3, [FP, #offset]
	STRB R2, [R1, R3]
	ADD R3, #1
	STR R3, [FP, #offset]
	CMP R3, #5		; when offset becomes 5, branch out of loop
	BNE Loop1
	B done
	
Valid
	LDR R2, [FP, #number]	; d = num/1000 + ascii offset
	MOV R3, #1000
	UDIV R2, R3
	ADD R2, #0x30
	STR R2, [FP, #d]
	
	LDR R3, [FP, #offset]
	STRB R2, [R1, R3]	; store d in array
	ADD R3, #1	; increment to next arr address
	STR R3, [FP, #offset]
	
	MOV R2, #0x2E	; store decimal point in array
	LDR R3, [FP, #offset]
	STRB R2, [R1, R3]
	ADD R3, #1	; move to next element
	STR R3, [FP, #offset]
	
	LDR R2, [FP, #number]	; number % 1000
	MOV R3, #1000
	UDIV R4, R2, R3
	MUL R4, R3
	SUB R2, R2, R4
	STR R2, [FP, #number]	; store num % 1000 into num
	
	LDR R2, [FP, #number]	; d = num/100 + ascii offset
	MOV R3, #100
	UDIV R2, R3
	ADD R2, #0x30
	STR R2, [FP, #d]
	
	LDR R3, [FP, #offset]
	STRB R2, [R1, R3]	; store d in array
	ADD R3, #1	; increment to next arr address
	STR R3, [FP, #offset]
	
	LDR R2, [FP, #number]	; number % 100
	MOV R3, #100
	UDIV R4, R2, R3
	MUL R4, R3
	SUB R2, R2, R4
	STR R2, [FP, #number]	; store num % 100 into num
	
	LDR R2, [FP, #number]	; d = num/10 + ascii offset
	MOV R3, #10
	UDIV R2, R3
	ADD R2, #0x30
	STR R2, [FP, #d]
	
	LDR R3, [FP, #offset]
	STRB R2, [R1, R3]	; store d in array
	ADD R3, #1	; increment to next arr address
	STR R3, [FP, #offset]
	
	LDR R2, [FP, #number]	; number % 10
	MOV R3, #10
	UDIV R4, R2, R3
	MUL R4, R3
	SUB R2, R2, R4
	STR R2, [FP, #number]	; store num % 10 into num
	
	LDR R2, [FP, #number]	; d = num + ascii offset
	ADD R2, #0x30
	STR R2, [FP, #d]
	
	LDR R3, [FP, #offset]
	STRB R2, [R1, R3]	; store d in array
	ADD R3, #1	; increment to next arr address
	
	MOV R2, #0x20	; store space at end of string
	STRB R2, [R1, R3]
	ADD R3, #1	; increment offset once more
	STR R3, [FP, #offset]

done
	MOV R4, #0	; store null at end of array
	LDR R3, [FP, #offset]
	STRB R4, [R1, R3]

	ADD SP, #12
	POP {R4, R11}
    BX LR


     ALIGN                           ; make sure the end of this section is aligned
     END                             ; end of file
