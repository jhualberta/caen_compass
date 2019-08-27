#include "TFile.h"
#include "TTree.h"

void readBin(void) {
  UShort_t ped[1024]; // an array of 16 bit unsigned integers
  FILE *fin = fopen("test500ped.dat", "r");
  if (!fin) {
    printf("Error : test500ped.dat not found!\n");
  return;
  }
  TFile *fout = TFile::Open("test500ped.root", "recreate");
  TTree *tree = new TTree("tree", "A TTree from test500ped.dat");
  // "==> Case A" in http://root.cern.ch/root/html/TTree.html 24
  tree->Branch("ped", ped, "ped[1024]/s"); // "/s" = "UShort_t"
  while ( sizeof(ped) == fread(ped, 1, sizeof(ped), fin) ) {
    //#if 0 / 0 or 1 */
    // swap the high and low bytes (endianness correction)
    for (Int_t i = 0; i < ((Int_t)sizeof(ped)); i += 2) {
      UChar_t c = *((UChar_t *)ped + i);
      *((UChar_t *)ped + i) = *((UChar_t *)ped + i + 1);
      *((UChar_t )ped + i + 1) = c;
    }
  //#endif / 0 or 1 */
  tree->Fill();
  }
  fclose(fin); // no longer needed
  tree->Write();
  delete fout; // automatically deletes the "tree", too
  return;
}
