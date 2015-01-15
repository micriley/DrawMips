# Computer sum of an array of integers
.data
Length: .word 10
Data:   .word 10 20 30 40 50 60 70 80 90 100

.text
        LW      $1,Length($0)   # Length of array in words in $1
        ADD     $2,$0,$0        # Running sum in $2
        ADD     $3,$0,$0        # Index
Loop:   SLL     $4,$3,2         # $4 = $3 * 4
        LW      $5,Data($4)     # Next value in $5
        ADD     $2,$2,$5        # Add value to runnign sum
        ADDI    $3,$3,1         # Next index
        SLT     $5,$3,$1        # See if index < Length
        BNE     $5,$0,Loop      # Get next data element
        JR      $31             # Return to operating system
