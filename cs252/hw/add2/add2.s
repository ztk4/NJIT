        .global add2                @ Globalize label add2 to allow call from driver

add2:   STMFD   sp!, {v1-v6, lr}    @ add2 routine to take two integers and return the sum
        ADD     a1, a1, a2          @ sum a1 and a2 into a1
        LDMFD   sp!, {v1-v6, pc}
        .end

