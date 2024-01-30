// i1.c
// Erin Jones


//ok run with 
//gcc i1.c -o i1
//i1 i1test.e > i1test.out
#include <stdio.h>    // for I/O functions
#include <stdlib.h>   // for exit()
#include <time.h>     // for time functions

FILE *infile;
short r[8], mem[65536], offset6, imm5, imm9, pcoffset9, pcoffset11, 
      regsave1, regsave2;
unsigned short ir, pc, opcode, eopcode, code, dr, sr, sr1, sr2, baser, bit5, bit11,
               trapvec, n, z, c, v;
char letter;

void setnz(short r)
{
   n = z = 0;
   if (r < 0)    // is result negative?
      n = 1;     // set n flag
   else
   if (r == 0)   // is result zero?
      z = 1;     // set z flag
}

void setcv(short sum, short x, short y)
{
   v = c = 0;
   if (x >= 0 && y >= 0)   // if both non-negative, then no carry
      c = 0;
   else
   if (x < 0 && y < 0)     // if both negative, then carry
      c = 1;
   else
   if (sum >= 0)           // if signs differ and sum non-neg, then carry
      c = 1;
   else                    // if signs differ and sum neg, then no carry
      c = 0;
   // if signs differ then no overflow
   if ((x < 0 && y >= 0) || (x >= 0 && y < 0))
      v = 0;
   else
   // if signs the same and sum has different sign, then overflow
   if ((sum < 0 && x >= 0) || (sum >= 0 && x < 0))
      v = 1;
   else
      v = 0;
}

int main(int argc, char *argv[])\
{
   time_t timer;
   if (argc != 2)
   {
       printf("Wrong number of command line arguments\n");
       printf("Usage: i1 <input filename>\n");
       exit(1);
   }

   // display your name, command line args, time
   time(&timer);      // get time
   printf("Erin Jones     %s %s     %s", 
           argv[0], argv[1], asctime(localtime(&timer)));

   infile = fopen(argv[1], "rb"); // open file in binary mode
   if (!infile)
   {
      printf("Cannot open input file %s\n", argv[1]);
      exit(1);
   }

   fread(&letter, 1, 1, infile);  // test for and discard get file sig
   if (letter != 'o')
   {
      printf("%s not an lcc file", argv[1]);
      exit(1);
   }
   fread(&letter, 1, 1, infile);  // test for and discard 'C'
   if (letter != 'C')
   {
      printf("Missing C header entry in %s\n", argv[1]);
      exit(1);
   }

   fread(mem, 1, sizeof(mem), infile); // read machine code into mem

   while (1)
   {
      /* For debugging
      printf("\n");
      printf("---IR = %x---", ir);
      printf("\n");
      printf("---r0:%d, r1:%d, r2:%d, r3:%d, r4:%d, r5:%d, r6:%d, r7:%d, r8:%d, ir:%d---", r[0],r[1],r[2],r[3],r[4],r[5],r[6],r[7],r[8], pc); 
      printf("\n");*/
      // fetch instruction, load it into ir, and increment pc
      ir = mem[pc++];  
      //printf("PC = %d | ", pc);                

      // isolate the fields of the instruction in the ir
      opcode = ir >> 12;                  // get opcode
      pcoffset9 = ir << 7;                // left justify pcoffset9 field
      pcoffset9 = imm9 = pcoffset9 >> 7;  // sign extend and rt justify
      pcoffset11 = ir << 5;               // left justify pcoffset11 field
      pcoffset11 = pcoffset11 >> 5;       // sign extend and rt justify
      imm5 = ir << 11;                    // left justify imm5 field
      imm5 = imm5 >> 11;                  // sign extend andd rt justify
      offset6 = ir << 10;                 // left justify offset6 field
      offset6 = offset6 >> 10;            // sign extend and rt justify
      trapvec = ir & 0x001f;              // get trapvec and eopcode fields
      code = dr = sr = (ir & 0x0e00) >> 9;// get code/dr/sr and rt justify
      sr1 = baser = (ir & 0x01c0) >> 6;   // get sr1/baser and rt justify; 0x01c0 = 0000 0001 1100 0000
      sr2 = ir & 0x0007;                  // get third reg field; 0x0007 = 0000 0001 1100 0000
      bit5 = ir & 0x0020;                 // get bit 5; 0x0010 = 0000 0000 0001 0000
      bit11 = ir & 0x0800;                // get bit 11; 0x0800 = 0000 1000 0000 0000

      // decode (i.e., determine) and execute instruction just fetched
      switch (opcode)
      {
         
         case 0:                          // branch instructions
            //commented out printf statements were for debugging to see which instruction was being used 
            switch(code)
            {
               case 0: if (z == 1)             // brz
                  //given
                     /* printf("BRZ: pc = %d, pcoffset = %d", pc, pcoffset9);
                     printf("\n"); */
                       pc = pc + pcoffset9;
                       break;
               case 1: if (z == 0)             // brnz
                     //given
                     /* printf("BRNZ: pc = %d, pcoffset = %d", pc, pcoffset9);
                     printf("\n"); */
                          pc = pc + pcoffset9;
                       break;

                case 2: if (n == 1)            //brn
                    //010
                    //complete
                    /* printf("BRN: pc = %d, pcoffset = %d", pc, pcoffset9);
                     printf("\n"); */
                        pc = pc + pcoffset9;
                        break;
                case 3: if (n == z)            //brp
                    //011
                    //complete
                    /* printf("BRP: pc = %d, pcoffset = %d", pc, pcoffset9);
                     printf("\n"); */
                        pc = pc + pcoffset9;
                        break;

                case 4:  //brlt
                  //100
                     /* printf("BRLT: pc = %d, pcoffset = %d", pc, pcoffset9);
                     printf("\n"); */
                        if (n != v)
                            pc = pc + pcoffset9;
                        break;

                case 5:  //brgt
                  //101
                  //honestly don't know if this is even required
                     /* printf("BRGT: pc = %d, pcoffset = %d", pc, pcoffset9);
                     printf("\n"); */
                     if (n == v && z == 0)
                         pc = pc + pcoffset9;
                     break;

               case 6:  //brc
                  //110
                  //again i don't think these branch ones are required but i might as well try and see if it does anything
                     /* printf("BRC: pc = %d, pcoffset = %d", pc, pcoffset9);
                     printf("\n");   */ 
                     if (c == 1)
                         pc = pc + pcoffset9;
                     break;

               case 7:                        // br
                     //given
                     /* printf("BR: pc = %d, pcoffset = %d", pc, pcoffset9);
                     printf("\n"); */
                     pc = pc + pcoffset9;
                       break;
            }                                                   
            break;
         case 1:                               // add
            //given
            if (bit5)
            {
               /* printf("ADD: dr = %d:%d, sr1 = %d:%d, imm5 = %d", dr, r[dr], sr1, r[sr1], imm5);
               printf("\n"); */
               regsave1 = r[sr1];
               r[dr] = regsave1 + imm5;
               // set c, v flags
               setcv(r[dr], regsave1, imm5);
            }
            else
            {
              /*  printf("ADD: dr = %d:%d, sr1 = %d:%d, sr2 = %d:%d", dr, r[dr], sr1, r[sr1], sr2, r[sr2]);
               printf("\n"); */
               regsave1 = r[sr1]; regsave2 = r[sr2];
               r[dr] = regsave1 + regsave2;
               // set c, v flags
               setcv(r[dr], regsave1, regsave2);
            }
            // set n, z flags
            setnz(r[dr]);
            break;
         case 2:                          // ld
            //complete
               /* printf("LOAD: dr = %d:%d, pcoffset9 = %d", dr, r[dr], pcoffset9);
               printf("\n"); */
            
                r[dr] = mem[pc + pcoffset9];

            break;

         case 3:                          //st
            //0011 sr pcoffset9
            //complete
            /* printf("STORE: sr = %d:%d, pcoffset9 = %d, imm5 = %d", sr, r[sr], pcoffset9);
            printf("\n"); */
                mem[pc+pcoffset9] = r[sr];
            break;

         case 4:                          //bl and blr
            /*0100 1 pcoffset11
              0100 000 baser offset6
            */
         //incomplete
            if(bit11){                    //bl
               /* printf("BL: lr = %d, baser = %d:%d, pcoffset11 = %d", r[7], baser, r[baser], pcoffset11);
               printf("\n"); */
               r[7] = pc;
               pc = pc+pcoffset11;
            } 
            
            else {                      //blr
               /* printf("BLR: lr = %d, baser = %d:%d, pcoffset6 = %d", r[7], baser, r[baser], offset6);
               printf("\n"); */
               r[7] = pc;
               //pc = r[baser]; kinda works too somehow
               pc = r[baser]+offset6;
            }
            break;
            
         
         case 5:                          //and
            //0101
         //complete but should be right
            /*
                check how the add code does it;
                first check for bit 5 to see which version it is
                and is 0101 dr sr1 000 sr2    sets nz
                       0101 dr sr1 1 imm5     sets nz
                       I don't think it should test for bit 5, as the digit with a 1 signaling the imm5 version would be
                       0101 000 000 1 00000
                       op   dr  sr    imm5

                       maybe instead test for the pcoffset 6 and see if its
                       1 00000 -- ok that caused more problems, maybe bit 5 is the way to go
            */
            
            if (bit5)
            {
               
               /* printf("AND: dr = %d:%d, sr1 = %d:%d, imm5 = %d", dr, r[dr], sr1, r[sr1], imm5);
               printf("\n"); */
               /* regsave1 = r[sr1];
               r[dr] = regsave1 & imm5; */

               r[dr] = r[sr1] & imm5;
               // set n, z flags
               setnz(r[dr]);
               break;
               
            }
            else
            {
               /* printf("AND: dr = %d:%d, sr1 = %d:%d, sr2 = %d:%d", dr, r[dr], sr1, r[sr1], sr2, r[sr2]);
               printf("\n"); */
               r[dr] = r[sr1] & r[sr2];
               // set n, z flags
               setnz(r[dr]);
            }
            break;
         case 6:                          //ldr
            /* I think what was making ldr and str not work in the end was not having break statements, not sure why exactly that 
            was the case, as the different opcodes should've stopped that from being too much of a factor, but it was something
            i changed right before the output started being correct.
            I just checked and that was it, I'm guessing it had something to do with one of the variables the switch case was
            testing for being similar or the same to something in ldr and str, so they got caught in another switch case and
            added on another instruction that messed with the output */

            //0110 dr baser offset6
            //complete
            /* printf("LDR: dr = %d:%d, baser = %d:%d", dr, r[dr], sr1, r[sr1], baser, r[baser]);
            printf("\n"); */
               //r[dr] = r[baser] + offset6; nvm that's wrong
               r[dr] = mem[r[baser] + offset6];
            break;
         case 7:                          //str
            //0111 sr baser offset6
            //complate
            //stores the source register itself
            /* printf("STR: sr = %d:%d, baser = %d:%d, pcoffset6 = %d", sr, r[sr], baser, r[baser], offset6);
            printf("\n"); */
            mem[r[baser] + offset6] = r[sr];
            //mem[r[baser] + offset6] = sr; -- definitely not this, it does not just take the source register address lmao it broke so much it definitely takes in the contents
            break;
         case 9:                          // not
            //given
            /* printf("NOT: dr = %d:%d, sr1 = %d:%d", dr, r[dr], sr1, r[sr1]);
            printf("\n"); */
            // ~ is the not operator in C
            r[dr] = ~r[sr1];
            // set n, z flags
            setnz(r[dr]);
            break;

         case 12:                         // jmp/ret
            //this version works, even though it doesn't look like it should
            /* printf("JMP/RET: pc = %d, baser = %d:%d, pcoffset6 = %d", pc, baser, r[baser], offset6);
            printf("\n");
            pc = r[baser]+offset6;
            break;  */
                     
            /* complete, should be right
            the other one with the jump and return seperated looks right because it matches the documentation, but with it that way it always messed up the 7 and 8 outputs for the test file
            actually, both versions seem to work now, not sure what exactly fixed it. Maybe moving it out of the cuh folder somehow, or one of the changes i made to another instruction
            tldr: both instructions work after all, but the one I went with is more accurate to how the lcc handles it
            turns out it was a problem with ldr and str, not jmp/ret
 */
             /* 
            jmp 1100 code baser offset6 -- pc = baser + offset6
            
            ret 1100 111 111 offset6 -- pc = lr + offset6
            maybe have it store the pc value before changing it, then set pc back to the stored value after ret is called
            
            or switch on code to see whether it's jmp or ret
            this is causing an infinite loop somehow
            */
 
            if(sr1 == 7){                 //ret
               //sr1 is the 3 bits before the pcoffset6    
               /* printf("RETURN: pc = %d, baser = %d:%d, pcoffset6 = %d", pc, baser, r[baser], offset6);
               printf("\n");  */              
               pc = r[7] + offset6;    //r[7] is the link register
               break;
            } else {                      //jump
               //stores the pc value in the baser register
               /* printf("JUMP: pc = %d, baser = %d:%d, pcoffset6 = %d", pc, baser, r[baser], offset6);
               printf("\n"); */
               pc = r[baser] + offset6;                     
               break;
            }  

            

         case 14:                         // lea
            //given
            /* printf("STR: dr = %d:%d, pcoffset9 = %d", dr, r[dr], pcoffset9);
            printf("\n"); */
            r[dr] = pc + pcoffset9;
            break;
         case 15:                         // trap
            //given
            if (trapvec == 0x00)  {          // halt
               /* printf("HALT");
               printf("\n"); */
               exit(0);
            }
            else
            if (trapvec == 0x01)  {           // nl
               //complete
               /* printf("NL"); */
               printf("\n");
            }
            else
            if (trapvec == 0x02)  {           // dout
               /* printf("DOUT: sr = %d:%d", sr, r[sr]);
               printf("\n"); */
               //1111 sr 0 00000010 
               //complete 
               printf("%d", r[sr]);
            }
               


            break;
      }     // end of switch
   }        // end of while
}
