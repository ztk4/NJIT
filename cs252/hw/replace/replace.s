            .global replace

@ function int replace(char *str, int lr, rp) - returns number of replacements
replace:    STMFD       sp!, {v1-v6, lr}    @ All occurences of lr in str are replaced with rp
            MOV         v1, a1              @ Save str ptr in v1
            MOV         a1, #0              @ a1 will count number of replacements
            
rep_loop:   LDRB        v2, [v1], #1        @ Read char and post increment v1
            CMP         v2, #0
            BEQ         rep_ret             @ End of str
            CMP         v2, a2
            STREQB      a3, [v1, #-1]       @ Replace char at index - 1 (bc of post incr)
            ADDEQ       a1, a1, #1
            B           rep_loop

rep_ret:    LDMFD       sp!, {v1-v6, pc}    @ Branch back to link, return val is in a1
            
            .end

