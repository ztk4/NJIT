            .global     init
            .global     decrypt

            .data
result_p:   .space      4 

@Method void init(char *str), mallocs space for str in result_p
init:       STMFD       sp!, {v1-v6, lr}
            BL          strlen              @get len
            ADD         a1, a1, #1
            BL          malloc              @malloc len + 1
            STR         a1, result_p
            LDMFD       sp!, {v1-v6, pc}

@Method char *decrypt(char *enc, int sh) shifts alphabetic characters in enc by sh
@returns a pointer to result
@assumes 0 <= sh <= 25
decrypt:    STMFD       sp!, {v1-v6, lr}
            MOV         v1, a1
            MOV         v2, a2
            LDR         v3, =result_p       @load in pc-relative addr of ptr
            LDR         v3, [v3]            @load in ptr from pc-relative addr

decrypt_l:  LDRB        v4, [v1], #1        @get char from encr
            MOV         a1, v4
            BL          isalpha             
            CMP         a1, #0              @test is alpha (shift needed?)
            BEQ         nonalpha

alpha:      SUB         v5, v4, #'A'        @'A' is smaller than 'a'
            CMP         v5, #26
            MOVLT       v6, #'A'            @store base of 'A' if upper
            MOVGE       v6, #'a'            @store base of 'a' if lower
            SUBGE       v5, v4, #'a'        @redo sub if new base

            ADD         v5, v5, v2          @shift 0-based char
            CMP         v5, #26
            SUBGE       v5, #26             @wrap shift
            ADD         v4, v5, v6          @save final character into v4

nonalpha:   STRB        v4, [v3], #1        @str char
            CMP         v4, #0
            BNE         decrypt_l

            LDR         a1, =result_p
            LDR         a1, [a1]            @load back ptr to front of result
            LDMFD       sp!, {v1-v6, pc}
            
            .end

