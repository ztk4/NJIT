            .global     init
            .global     decrypt

@Method void init(char *str), mallocs space for str in result_p
init:       LDMFD       sp!, {v1-v6, lr}
            BL          strlen              @get len
            ADD         a1, a1, #1
            BL          malloc              @malloc len + 1
            STR         a1, result_p
            STMFD       sp!, {v1-v6, pc}

debug:      .asciz      "Here!\n"
            .align
@Method char *decrypt(char *enc, int sh) shifts alphabetic characters in enc by sh
@returns a pointer to result
@assumes 0 <= sh <= 25
decrypt:    LDMFD       sp!, {v1-v6, lr}
            MOV         v1, a1
            MOV         v2, a2
            LDR         v3, =result_p       @load in result pointer

decrypt_l:  LDR         a1, =debug
            BL          printf
            LDRB        v4, [v1], #1        @get char from encr
            MOV         a1, v4
            BL          isalpha             
            CMP         a1, #1              @test is alpha (shift needed?)
            BNE         nonalpha

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

            MOV         a1, v3
            STMFD       sp!, {v1-v6, pc}

            .text
result_p:   .space      4                   @0-fill 4 bytes

            .end

