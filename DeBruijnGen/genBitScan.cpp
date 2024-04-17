// Copied from https://www.chessprogramming.org/De_Bruijn_Sequence_Generator
// Generator by Gerd Isenberg.

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef unsigned __int64 U64; // long long

class CGenBitScan
{
public:
   //==========================================
   // constructor immediately starts the search
   //==========================================
   CGenBitScan(int match4nth) {
      clock_t start, stop;
      m_dBCount = 0;
      m_Match4nth  = match4nth;
      initPow2();
      start = clock();
      m_Lock = pow2[32]; // optimization to exclude 32, see remarks 
      try {findDeBruijn(0, 64-6, 0, 6);} catch(int){}
      stop = clock();
      printf("\n%.3f Seconds for %d De Bruijn Sequences found\n", (float)(stop - start) / CLOCKS_PER_SEC, m_dBCount);
   }

private:
   U64 pow2[64];    // single bits
   U64 m_Lock;      // locks each bit used
   int m_dBCount;   // counter
   int m_Match4nth; // to match

   //==========================================
   // on the fly initialization of pow2
   //==========================================
   void initPow2()  {
      pow2[0] = 1;
      for (int i=1; i < 64; i++)
         pow2[i] = 2*pow2[i-1];
   }

   //==========================================
   // print the bitscan routine and throw
   //==========================================
   void bitScanRoutineFound(U64 deBruijn) {
      int index[64], i;
      for (i=0; i<64; i++) // init magic array
         index[ (deBruijn<<i) >> (64-6) ] = i;
      printf("\nconst U64 magic = 0x%08x%08x; // the %d.\n\n",
              (int)(deBruijn>>32), (int)(deBruijn), m_dBCount);
      printf("const unsigned int magic_table[64] =\n{\n");
      for (i=0; i<64; i++) {
         if ( (i & 7) == 0 ) printf("\n  ");
         printf(" %2d,", index[i]);
      }
      printf("\n};\n\nunsigned int bit_scan_forward (U64 b) {\n");
      printf("    return magic_table[((b&-b)*magic) >> 58];\n}\n");
      // throw 0; // unwind the stack until catched
   }

   //============================================
   // recursive search
   //============================================
   void findDeBruijn(U64 seq, int depth, int vtx, int nz) {
      if ( (m_Lock & pow2[vtx]) == 0 && nz <= 32 ) { // only if vertex is not locked
         if ( depth == 0 ) { // depth zero, De Bruijn Sequence found, see remarks
            if ( ++m_dBCount == m_Match4nth )
               bitScanRoutineFound(seq);
         } else {
            m_Lock ^= pow2[vtx]; // set bit, lock the vertex to don't appear multiple
            if ( vtx == 31 && depth > 2 ) { // optimization, see remarks
                findDeBruijn( seq | pow2[depth-1], depth-2, 62, nz+1);
            } else {
                findDeBruijn( seq, depth-1, (2*vtx)&63, nz+1); // even successor
                findDeBruijn( seq | pow2[depth-1], depth-1, (2*vtx+1)&63, nz); // odd successor
            }
            m_Lock ^= pow2[vtx]; // reset bit, unlock
         }
      }
   }
};

int main(int argc, char* argv[])
{
   if (argc < 2)
      printf("usage: genBitScan 1 .. %d\n", 1<<26);
   else
      CGenBitScan(atoi(argv[1]));
   return 0;
}