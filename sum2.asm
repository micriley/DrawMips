# Computer sum of an array of integers
# This version uses the address of Data in a register        
.data
Length: .word 10
Data:   .word 10 20 30 40 50 60 70 80 90 100

.text
        LW      $1,Length($0)  # Length of array in words in $1
        SLL     $1,$1,2         # Length of array in bytes in $1
        ADD     $2,$0,$0        # Running sum in $2
        LUI     $3,HI[Data]  # upper 16 bits of Address of Data in $3
        ORI     $3,$3,LO[Data]  # lower 16 bits of Address of Data in $3
        #ORI     $3,$0,Data      # Address of Data in $3
        ADD     $4,$3,$1        # Address of one past array
        # Check address in range before reading value
Loop:  SLT     $5,$3,$4        # 1 if Address less than end+1 address
        BEQ     $5,$0,Exit      # No more elements
        LW      $5,0($3)        # Next value in $5
        ADD     $2,$2,$5        # Add value to runnign sum
        ADDI    $3,$3,4         # Next address
        BEQ     $0,$0,Loop      # Loop back to sum next element
Exit:   JR      $31             # Return to operating system
        
