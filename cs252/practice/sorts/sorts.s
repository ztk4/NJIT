            .extern rand
            .extern printf
            .extern putchar

            @ Swap Utility
swap:       LDR         a3, [a1]            @ Uses 3rd and 4th arg reg as tmp to avoid push/pop from stack
            LDR         a4, [a2]
            STR         a4, [a1]
            STR         a3, [a2]
            MOV         pc, lr

            @ Sorting Methods
            .global ssort
ssort:      STMFD       sp!, {v1-v6, lr}    @ Selection Sort - ssort(int *arr, int N)
            MOV         v1, a1              
            ADD         v2, v1, a2, LSL #2  

ssort_loop1:ADD         v3, v1, #4          @ Pointer to just after element to be swapped with
            CMP         v3, v2
            BGE         ssort_ret

            MOV         v4, v1              @ Pointer to minimum value of unsorted portion
            LDR         v5, [v1]            @ Value of minimum of unsorted portion

ssort_loop2:LDR         v6, [v3], #4        @ Get next value
            CMP         v6, v5
            MOVLT       v5, v6              @ Record new min
            SUBLT       v4, v3, #4          @ Record pointer to new min
            CMP         v3, v2
            BNE         ssort_loop2 

            LDR         v6, [v1]            @ Load current value at v1 (v6 was tmp to loop2)
            STR         v5, [v1], #4        @ Store minimum value into v1
            STR         v6, [v4]            @ Store v1's old value into v4
            B           ssort_loop1

ssort_ret:  LDMFD       sp!, {v1-v6, pc}

            .global isort
isort:      STMFD       sp!, {v1-v6, lr}    @ Insertion Sort - isort(int *arr, int N)
            MOV         v1, a1
            ADD         v2, v1, a2, LSL #2
            MOV         v3, v1              @ Walking Pointer (v1 has to stay at beginning)

isort_loop1:ADD         v3, v3, #4          @ Move v3 to just after sorted portion
            CMP         v3, v2
            BGE         isort_ret
            
            LDR         v4, [v3]            @ Val to insert
            SUB         v5, v3, #4          @ End of sorted portion

isort_loop2:LDR         v6, [v5], #-4       @ Load val to compare, decrement pointer
            CMP         v6, v4
            STRLE       v4, [v5, #8]        @ Store where last elt shifted from
            BLE         isort_loop1

            STR         v6, [v5, #8]        @ Store one above where it came from (shift up)
            CMP         v5, v1              @ Check if done
            BGE         isort_loop2
            
            STR         v4, [v5, #4]        @ Store at front
            B           isort_loop1
            
isort_ret:  LDMFD       sp!, {v1-v6, pc}
            
            .global hsort
hsort:      STMFD       sp!, {v1-v6, lr}    @ Heap Sort - hsort(int *arr, int N)
            MOV         v1, a1
            MOV         v2, a2
            RSB         a2, #1, v2, LSR #1  @ a2 = v2/2 - 1
            ADD         a1, v1, a2, LSL #2  
            SUB         a2, v2, a2          @ Invert a2 about size of list

hsort_loop1:BL          sift_down           @ Heapify
            CMP         a1, v1
            SUB         a1, a1, #4
            ADD         a2, a2, #1
            BGT         hsort_loop1

            MOV         a1, v1
            SUB         a2, v2, #1
            ADD         v2, v1, a2, LSL #2  @ v2 points to end of heap

hsort_loop2:LDR         v3, [v1]            @ Head of heap
            LDR         v4, [v2], #-4       @ Last child in heap
            BL          sift_down
            SUBS        a2, a2, #1
            BGT         hsort_loop2
            
            LDMFD       sp!, {v1-v6, pc}

            @ Utility Method for Heap Sort - Sifts down semi-heap headed at arr
sift_down:  STMFD       sp!, {v1-v6, lr}    @ Sift Down - sift_down(int *arr, int N)
            MOV         v1, a1              
            MOV         v2, #0              @ Index
            LDR         v3, [v1]            @ Value to Shift

siftd_loop: ADD         v6, #1, v2, LSL #1  @ v2 += 1 + (v2 << 1) - 2n + 1
            CMP         v6, a2
            BGE         siftd_ret           @ If past end of list
            LDR         v4, [v1, v6, LSL #2]@ Load in left child

            ADD         v6, v6, #1          @ 2n + 2
            CMP         v6, a2
            BGE         siftd_ret           @ If past end of list
            LDR         v5, [v1, v6, LSL #2]@ Load in right child

            CMP         v5, v4
            MOVGT       v4, v5              @ Let v4 be the max of v5, v4
            SUBLT       v6, #1              @ If LT, then we want to swap with other child

            CMP         v3, v4
            MOVLT       v2, v6              @ Set new Index
            STRLT       v4, [v1, v2, LSL #2]@ Swap up
            BLT         siftd_loop          @ Continue down

siftd_ret:  STR         v3, [v1, v2, LSL #2]@Place and exit
            LDMFD       sp!, {v1-v6, pc}

            @ Utilities for Printing and Generating Arrays
            .global rand_arr
rand_arr:   STMFD       sp!, {v1-v6, lr}    @ Randomize array - void rand_arr(int *arr, int N)
            MOV         v1, a1              @ v1 = a1 => arr
            ADD         v2, v1, a2, LSL #2  @ v2 = v1 + (a2)*4 => arr + N

rand_loop:  CMP         v1, v2
            BEQ         rand_ret
            BL          rand
            MOV         a1, a1, LSR #24     @ a1 >>= 24 (result is random 8bit number)
            STR         a1, [v1], #4        @ *v1 = a1; v1 += 4
            B           rand_loop

rand_ret:   LDMFD       sp!, {v1-v6, pc}


            .global print_arr
print_arr:  STMFD       sp!, {v1-v6, lr}    @ Print array - void print_arr(int *arr, int N)
            MOV         v1, a1              @ v1 = a1 => arr
            ADD         v2, v1, a2, LSL #2  @ v2 = v1 + (a2)*4 => arr + N
            
            @ Print opening brace
            MOV         a1, #'['
            BL          putchar

            @ check if done
            CMP         v1, v2
            BEQ         print_end

            @ Print first elt
            LDR         a1, =format_d
            LDR         a2, [v1], #4        @ a2 = *v1++
            BL          printf
        
print_loop: CMP         v1, v2
            BEQ         print_end
            LDR         a1, =format_arr
            LDR         a2, [v1], #4
            BL          printf
            B           print_loop

print_end:  MOV         a1, #']'
            BL          putchar
            MOV         a1, #'\n'
            BL          putchar

            LDMFD       sp!, {v1-v6, pc}


format_d:  .asciz  "%d"
format_arr:.asciz  ", %d"

            .end

