/*
Copyright 2013, Jernej Kovacic

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/


/**
 * @file
 *
 * Implementation of ARM exception handlers (except the reset handler that is 
 * implemented in vector.s).
 * 
 * The most important handler is the IRQ interrupt handler.
 * 
 * @author Jernej Kovacic
 */

/*
 * TODO
 * Right now, handling of IRQ interrupt is not supported yet.
 * Hence the handler is currently "implemented" as an infinite loop.
 * 
 * As soon as IRQ handling is supported, this will be reimpleneted.
 */
void __attribute__((interrupt)) irq_handler() 
{
    for ( ; ; );
}
 


/*
 * All other exception handlers are implemented as infinite loops. 
 */
void __attribute__((interrupt)) undef_handler(void) 
{ 
    for( ; ; ); 
}

void __attribute__((interrupt)) swi_handler(void) 
{ 
    for( ; ; ); 
}

void __attribute__((interrupt)) prefetch_abort_handler(void)
{ 
    for( ; ; ); 
}

void __attribute__((interrupt)) data_abort_handler(void)
{ 
    for( ; ; ); 
}

void __attribute__((interrupt)) fiq_handler(void) 
{ 
    for( ; ; ); 
}


/*
 * 'vectors_start' and 'vectors_end' with exception handling vectors are placed
 * to point where the code is actually loaded (0x00010000 by Qemu), they must be copied
 * to the place where the ARM CPU actually expects them, i.e. at the begining of the
 * memory (0x00000000)
 */
void copy_vectors(void) 
{
    /* Both values are declared in vectors.s: */
    extern unsigned long vectors_start;
    extern unsigned long vectors_end;
    
    unsigned long* vectors_src = &vectors_start;
    unsigned long* vectors_dst = (unsigned long*) 0x00000000;
 
    /*
     * Vectors are copied backwards in the memory (from something positive to 0x0000000) 
     * from their first to the last byte, this prevents a potential corruption
     * if both memory areas overlap.
     */
    while (vectors_src < &vectors_end)
    {
        *vectors_dst++ = *vectors_src++;
    }
}
