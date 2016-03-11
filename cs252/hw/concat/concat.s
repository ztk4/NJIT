            .global     jstring

@ Routine to concatenate to strings, and return a pointer to the result
jstring:    STMFD       sp!, {v1-v6, lr}        @ char *jstring(char *buf1, char *buf2);
            MOV         v1, a1
            MOV         v2, a2
            
            BL          strlen
            MOV         v3, a1                  @ Length of buf1

            MOV         a1, a2
            BL          strlen
            MOV         v4, a1                  @ Length of buf2

            ADD         a1, v3, v4              @ Sum of Lengths
            BL          malloc
            MOV         v3, a1                  @ Ptr to Result (only edit copy in v3)

jloop1:     LDRB        v4, [v1], #1
            CMP         v4, #0
            STRNE       v4, [v3], #1
            BNE         jloop1

jloop2:     LDRB        v4, [v2], #1
            CMP         v4, #0
            STRNE       v4, [v3], #1
            BNE         jloop2

            LDMFD       sp!, {v1-v6, pc}
            .end

