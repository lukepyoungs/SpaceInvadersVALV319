; Print.s
; Student names: change this to your names or look very silly
; Last modification date: change this to the last modification date or look very silly
; Runs on LM4F120 or TM4C123
; EE319K lab 7 device driver for any LCD
;
; As part of Lab 7, students need to implement these LCD_OutDec and LCD_OutFix
; This driver assumes two low-level LCD functions
; ST7735_OutChar   outputs a single 8-bit ASCII character
; ST7735_OutString outputs a null-terminated string 

       EXPORT   ConvertUDec
       EXPORT   ConvertDistance
       EXPORT   String
       AREA  DATA, ALIGN=2
String SPACE 12 ; unsigned char String[10]
       AREA    |.text|, CODE, READONLY, ALIGN=2
       THUMB

    MACRO
    UMOD  $Mod,$Divnd,$Divsr ;MOD,DIVIDEND,DIVISOR
    UDIV  $Mod,$Divnd,$Divsr ;Mod = DIVIDEND/DIVISOR
    MUL   $Mod,$Mod,$Divsr   ;Mod = DIVISOR*(DIVIDEND/DIVISOR)
    SUB   $Mod,$Divnd,$Mod   ;Mod = DIVIDEND-DIVISOR*(DIVIDEND/DIVISOR)
    MEND
  

;-----------------------ConvertUDec-----------------------
; Convert a 32-bit number in unsigned decimal format
; Input: R0, 32-bit number to be transferred
; Output: none
; Lab 11 requirement is for at least one local variable on the stack with symbolic binding
; Converts a 32-bit number in unsigned decimal format
; Output: store the conversion in global variable String[12]
; Fixed format 4 digits, one space after, null termination
; Examples
;    4 to "   4 "  
;   31 to "  31 " 
;  102 to " 102 " 
; 2210 to "2210 "
;10000 to "**** "  any value larger than 9999 converted to "**** "
; Invariables: This function must not permanently modify registers R4 to R11
ConvertUDec
; copy your lab 11 solution here
	LDR R1, =10000
	CMP R0, R1
	BHS spec

first EQU 0
sec	EQU 1
third	EQU 2
fourth 	EQU 3
ascspace EQU 4
	SUB SP, #8
	MOV R2, #0
	MOV R1, R0
checkTH	
	CMP R1, #1000
	BLO storTH	; if r1 < 1000 then will store counter value
	SUB R1, R1, #1000
	ADD R2, #1
	B checkTH
checkHO
	CMP R1, #100
	BLO storHO
	SUB R1, R1, #100
	ADD R2, #1
	B checkHO
checkTE
	CMP R1, #10
	BLO storTE
	SUB R1, R1, #10
	ADD R2, #1
	B checkTE
checkON
	CMP R1, #1
	BLO storON
	SUB R1, R1, #1
	ADD R2, #1
	B checkON
storTH 
	ADD R2, R2, #0x30
	STR R2, [SP, #first]
	MOV R2, #0
	B checkHO
storHO
	ADD R2, R2, #0x30
	STR R2, [SP, #sec]
	MOV R2, #0
	B checkTE
storTE
	ADD R2, R2, #0x30
	STR R2, [SP, #third]
	MOV R2, #0
	B checkON
storON
	ADD R2, R2, #0x30
	STR R2, [SP, #fourth]
storSP
	MOV R3, #0x20; ascii for space
	STR R3, [SP, #ascspace]
; LOAD FROM STACK TO ARRAY	
	LDR R1, =String
	LDRB R3, [SP, #first]
	STRB R3, [R1]
	

	LDRB R3, [SP, #sec]
	STRB R3, [R1, #1]
	

	LDRB R3, [SP, #third]
	STRB R3, [R1, #2]
	

	LDRB R3, [SP, #fourth]
	STRB R3, [R1, #3]
	

	LDRB R3, [SP, #ascspace]
	STRB R3, [R1, #4]
	ADD SP, SP, #8

	B donefr


spec
	LDR R1, =String
	LDR R3, =0x2A ;ascii for *
	STRB R3, [R1] 
	LDR R3, =0x2A
	STR R3, [R1, #1]  ; ascii for space x20
	LDR R3, =0x2A ;ascii for *
	STRB R3, [R1, #2] 
	LDR R3, =0x2A ;ascii for *
	STRB R3, [R1, #3] 
	MOV R3, #0x20
	STRB R3, [R1, #4]
	MOV R3, #0
	STRB R3, [R1, #5]

donefr
    
    BX  LR
;* * * * * * * * End of ConvertUDec * * * * * * * *

; -----------------------ConvertDistance-----------------------
; Converts a 32-bit distance into an ASCII string
; Input: R0, 32-bit number to be converted (resolution 0.001cm)
; Output: store the conversion in global variable String[12]
; Fixed format 1 digit, point, 3 digits, space, units, null termination
; Examples
;    4 to "0.004 cm"  
;   31 to "0.031 cm" 
;  102 to "0.102 cm" 
; 2210 to "2.210 cm"
;10000 to "*.*** cm"  any value larger than 9999 converted to "*.*** cm"
; Invariables: This function must not permanently modify registers R4 to R11
; Lab 11 requirement is for at least one local variable on the stack with symbolic binding
ConvertDistance
; copy your lab 11 solution here
	LDR R1, =10000
	CMP R0, R1
	BHS spec2
fir EQU 0
per	EQU 1
dec1 EQU 2
dec10 EQU 3
dec100 EQU 4
	SUB SP, #16
	MOV R2, #0
	MOV R1, R0
checkfir
	CMP R1, #1000
	BLO storfir	; if r1 < 1000 then will store counter value
	SUB R1, R1, #1000
	ADD R2, #1
	B checkfir
checkdec1
	CMP R1, #100
	BLO stordec1
	SUB R1, R1, #100
	ADD R2, #1
	B checkdec1
checkdec10
	CMP R1, #10
	BLO stordec10
	SUB R1, R1, #10
	ADD R2, #1
	B checkdec10
checkdec100
	CMP R1, #1
	BLO stordec100
	SUB R1, R1, #1
	ADD R2, #1
	B checkdec100
storfir
	ADD R2, R2, #0x30
	STR R2, [SP, #fir]
	MOV R2, #0
	B checkdec1
stordec1 ; and period before
	MOV R3, #0x2E
	STRB R3, [SP, #per] ; ascii for .
	ADD R2, R2, #0x30
	STR R2, [SP, #dec1]
	MOV R2, #0
	B checkdec10
stordec10
	ADD R2, R2, #0x30
	STR R2, [SP, #dec10]
	MOV R2, #0
	B checkdec100
stordec100
	ADD R2, R2, #0x30
	STR R2, [SP, #dec100]
	
; LOAD FROM ATCK TO ARRAY
	LDR R1, =String
	LDRB R3, [SP, #fir]
	STRB R3, [R1]
	

	LDRB R3, [SP, #per]
	STRB R3, [R1,#1]
	

	LDRB R3, [SP, #dec1]
	STRB R3, [R1,#2]


	LDRB R3, [SP, #dec10]
	STRB R3, [R1,#3]
	

	LDRB R3, [SP, #dec100]
	STRB R3, [R1,#4]
	
	
	MOV R3, #0x20  ; store space
	STRB R3, [R1,#5]
	
	
	MOV R3, #0x63
	STRB R3, [R1,#6]; ascii for c
	
	
	MOV R3, #0x6D
	STRB R3, [R1,#7] ; ascii for m
	
	
	MOV R3, #0
	STRB R3, [R1,#8]; null character

	
	ADD SP, SP, #16
	B donezo
spec2
	LDR R2, =String
	MOV R3, #0x2A
	STRB R3, [R2] ; ascii for *
	
	MOV R3, #0x2E
	STRB R3, [R2,#1] ; ascii for .
	
	MOV R3, #0x2A
	STRB R3, [R2,#2] ; ascii for *
	
	MOV R3, #0x2A
	STRB R3, [R2,#3] ; ascii for *
	
	MOV R3, #0x2A
	STRB R3, [R2,#4] ; ascii for *
	
	MOV R3, #0x20
	STRB R3, [R2,#5] ; ascii for space
	
	MOV R3, #0x63
	STRB R3, [R2,#6]; ascii for c
	
	MOV R3, #0x6D
	STRB R3, [R2,#7] ; ascii for m
	
	MOV R3, #0
	STRB R3, [R2,#8]; null character
	

	
donezo
    BX   LR
 
     ALIGN
;* * * * * * * * End of ConvertDistance * * * * * * * *

     ALIGN          ; make sure the end of this section is aligned
     END            ; end of file
