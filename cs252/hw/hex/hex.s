            .global     int2hex

@Method char *int2hex(int) returns a char * to a hex repr of int
int2hex:    STMFD       sp!, {v1-v6, lr}
            MOV         v1, a1

            MOV         a1, #11
            BL          malloc
            MOV         v2, a1              @copy of char * to incr

            MOV         v3, #'0'
            STRB        v3, [v2], #1
            MOV         v3, #'x'
            STRB        v3, [v2], #1

            MOV         v3, #0xF            @bit mask
            MOV         v4, #32             @number of bits
hexloop:    SUBS        v4, v4, #4          @move 4 bits
            BLT         hexbreak
            AND         v5, v1, v3, LSL v4  @shift to correct digit
            MOV         v5, v5, LSR v4      @shift down to rightmost 4 bits
            CMP         v5, #10
            ADDLT       v6, v5, #'0'        @decimal digits have offsets from '0'
            ADDGE       v6, v5, #'A'-10     @extended digits have offsets from 'A'-10
            STRB        v6, [v2], #1
            B           hexloop
hexbreak:
            MOV         v3, #0
            STRB        v3, [v2]

            LDMFD       sp!, {v1-v6, pc}
            .end

