#include <cstdio>
#include <sstream>
#include <cstdlib>
#include <TTree.h>
#include <TFile.h>
#include <cmath>
#include <iostream>
#include <vector>
#include <TH1F.h>
#include <TH2.h>
#include <TCanvas.h>
#include <TApplication.h>
#include <TLegend.h>
#include <TLegendEntry.h>
#include <TPaveStats.h>
#include <TLatex.h>
#include <TROOT.h>
#include <thread> // Add this line for std::this_thread::sleep_for
#include <fstream>
#include <ctime>
#include <string> //
// #include <filesystem>
#include <dirent.h>
#include <unistd.h>
#include <termios.h>
#include <fnmatch.h> // Channelconfig
#include "TTree.h"
#include "TTreeReader.h"
#include "TDirectory.h"
#include <TBranch.h>
#include <TMath.h>
#include <cstdlib>
#include <TStyle.h> // Include the header file for gStyle
#include "TCutG.h"


// xdiff/ydiff gates
int gates[] = {	-10000,	50000,	-16000,	-4000,	-30000,	10000,	10000,	35000

};

int tdiff_gates[] = { -95000, -89000};


int plotter =0;// plotter 1 macht die bilder schoener aber blod zum arbeiten
std::string Ordner = "./";
std::string Runname = "run_9";        // no



// // CEJ this is old stuff
// int refreshcounter = 20000;            // wie oft soll der plotter geupdatet werden
// int sleeptimer = 1;                    // in sekunden
// std::string checkfile = "Data_run*.root";

// structure erstellen
struct EntryMCP
{
    ULong64_t Trigger_Time;
    Long64_t X1;
    Long64_t X2;
    Long64_t Y1;
    Long64_t Y2;
};


int countRootFilesInFolder(const std::string &folderPath)
{
  int count = 0;
  DIR *dir = opendir(folderPath.c_str());

  if (dir != nullptr)
  {
    struct dirent *entry;
    while ((entry = readdir(dir)) != nullptr)
    {
      std::string filename = entry->d_name;
      if ((filename.size() >= 9 && filename.substr(0, 4) == "Data") && (filename.substr(filename.size() - 5) == ".root"))
      // if ( filename.substr(filename.size() - 5) == ".root")

      {
        count++;
      }
    }
    closedir(dir);
  }
  return count;
}

int main()
{
  std::string folderPath = Ordner;

  std::string baseFileName = Ordner + Runname;

  std::cout << ": " << folderPath << std::endl;


  
  // Output root file
  std::string outputFileName = Runname + "_output.root";
  const char *outputFileNameCStr = outputFileName.c_str();
  TFile* of = new TFile(outputFileNameCStr, "RECREATE");
  
  // Directories are nice!
  TDirectory* dir_dssd = gDirectory->mkdir("DSSD_Energy");
  
  // ::: Define Canvases ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  TCanvas* c_delta_xy_MCP1 = new TCanvas("c_delta_xy_MCP1", "MCP1 Delta X and Y", 1920, 480);
  TCanvas* c_delta_xy_MCP2 = new TCanvas("c_delta_xy_MCP2", "MCP2 Delta X and Y", 1920, 480);
  c_delta_xy_MCP1->Divide(1, 2);
  c_delta_xy_MCP2->Divide(1, 2);
  
  TCanvas* c_heatmap_MCP1 = new TCanvas("c_heatmap_MCP1", "DDL Heatmap MCP 1", 1350, 1080);
  TCanvas* c_heatmap_MCP2 = new TCanvas("c_heatmap_MCP2", "DDL Heatmap MCP 2", 1350, 1080);
  // :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  
  // ::: Define Histograms :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  TH1F* h1_mcp_tdiff = new TH1F("h1_mcp_tdiff", "MCP TDiff Distribution", 10000, -400000, 0);
  TH1F* h1_mcp_tdiff_mcp1gate = new TH1F("h1_mcp_tdiff_mcp1gate", "MCP TDiff Distribution - Gated on MCP 1 Pos", 10000, -400000, 0);
  
  TH1F* h1_mcp1_event_dt = new TH1F("h1_mcp1_event_dt", "TDiff between MCP1 triggers", 1000, 100000, 100000000);

  TH2F* h2_DDL_heatmap_MCP1 = new TH2F("h2_DDL_heatmap_MCP1", "DDL Heatmap MCP1;#DeltaX in ns;#DeltaY in ns", 2000, -300001, 300001, 2000, -300001, 300001);
  TH2F* h2_DDL_heatmap_MCP2 = new TH2F("h2_DDL_heatmap_MCP2", "DDL Heatmap MCP2;#DeltaX in ns;#DeltaY in ns", 2000, -300001, 300001, 2000, -300001, 300001);
  h2_DDL_heatmap_MCP1->SetContour(40);
  
  TH1F* h1_deltaX_mcp1 = new TH1F("h1_deltaX_mcp1","MCP 1 xgate; #DeltaX in ns;Time (ns);Counts", 1000, gates[0], gates[1]);
  TH1F* h1_deltaY_mcp1 = new TH1F("h1_deltaY_mcp1","MCP 1 ygate; #DeltaY in ns;Time (ns);Counts", 1000, gates[4], gates[5]);
  TH1F* h1_deltaX_mcp2 = new TH1F("h1_deltaX_mcp2","MCP 2 xgate; #DeltaX in ns;Time (ns);Counts", 1000, gates[0], gates[1]);
  TH1F* h1_deltaY_mcp2 = new TH1F("h1_deltaY_mcp2","MCP 2 ygate; #DeltaY in ns;Time (ns);Counts", 1000, gates[4], gates[5]);

  // Gated spectra
  TH1F* h1_deltaX_mcp1_gated = new TH1F("h1_deltaX_mcp1_gated","MCP 1 xgate; #DeltaX in ns;Time (ns);Counts", 1000, gates[0], gates[1]);
  TH1F* h1_deltaY_mcp1_gated = new TH1F("h1_deltaY_mcp1_gated","MCP 1 ygate; #DeltaY in ns;Time (ns);Counts", 1000, gates[4], gates[5]);
  TH1F* h1_deltaX_mcp2_gated = new TH1F("h1_deltaX_mcp2_gated","MCP 2 xgate; #DeltaX in ns;Time (ns);Counts", 1000, gates[0], gates[1]);
  TH1F* h1_deltaY_mcp2_gated = new TH1F("h1_deltaY_mcp2_gated","MCP 2 ygate; #DeltaY in ns;Time (ns);Counts", 1000, gates[4], gates[5]);
  TH2F* h2_DDL_heatmap_MCP1_tgated = new TH2F("h2_DDL_heatmap_MCP1_tgated", "Gated DDL Heatmap MCP1;#DeltaX in ns;#DeltaY in ns", 2000, -300001, 300001, 2000, -300001, 300001);
  TH2F* h2_DDL_heatmap_MCP2_tgated = new TH2F("h2_DDL_heatmap_MCP2_tgated", "Gated DDL Heatmap MCP2;#DeltaX in ns;#DeltaY in ns", 2000, -300001, 300001, 2000, -300001, 300001);
  TH2F* h2_DDL_heatmap_MCP1_showgate = new TH2F("h2_DDL_heatmap_MCP1_showgate", "Show Pos Gate DDL Heatmap MCP1;#DeltaX in ns;#DeltaY in ns", 2000, -300001, 300001, 2000, -300001, 300001);
  TH2F* h2_DDL_heatmap_MCP2_showgate = new TH2F("h2_DDL_heatmap_MCP2_showgate", "Show Pos Gate DDL Heatmap MCP2;#DeltaX in ns;#DeltaY in ns", 2000, -300001, 300001, 2000, -300001, 300001);
  TH2F* h2_DDL_heatmap_MCP2_posgated = new TH2F("h2_DDL_heatmap_MCP2_posgated", "DDL Heatmap MCP2 - Pos Gate on MCP 1;#DeltaX in ns;#DeltaY in ns", 2000, -300001, 300001, 2000, -300001, 300001);

  TH1F* h1_labr_energy = new TH1F("h1_labr_energy", "LaBr Energy", 1000, 0, 2000);
  
  TH1F* h1_germanium_energy = new TH1F("h1_germanium_energy", "Germanium Energy", 1000, 0, 2000);
  
  dir_dssd->cd();
  TH1F* h1_dssd_energy[64];
  for (int i = 0; i < 64; i++) h1_dssd_energy[i] = new TH1F(Form("h1_dssd_energy_%i", i), Form("DSSD Energy Channel %i", i), 1000, 0, 6000);
  // :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  
  // ::: Gates :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  TCutG *mcp1_cut = new TCutG("tshape",8);
  mcp1_cut->SetVarX("DDL Heatmap MCP1");
  mcp1_cut->SetVarY("");
  mcp1_cut->SetTitle("Graph");
  mcp1_cut->SetFillStyle(1000);
  mcp1_cut->SetPoint(0,-44951.4,9486.16);
  mcp1_cut->SetPoint(1,-48494.4,-1243.8);
  mcp1_cut->SetPoint(2,-44287.1,-5075.93);
  mcp1_cut->SetPoint(3,-39636.9,-477.374);
  mcp1_cut->SetPoint(4,-38751.1,5654.03);
  mcp1_cut->SetPoint(5,-40744.1,9102.95);
  mcp1_cut->SetPoint(6,-43844.2,9102.95);
  mcp1_cut->SetPoint(7,-44951.4,9486.16);
	
  // :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::


  const char *filepathCStr = baseFileName.c_str();
  //const char *checkfilechar = checkfile.c_str();

  int p = 0;
  p = countRootFilesInFolder(folderPath.c_str());
  DIR *directory = opendir(folderPath.c_str());
  if (directory == nullptr)
  {
    std::cerr << "Failed to open directory." << std::endl;
    return 1;
  }

//   for (int i = 0; i < p; i++) // hier sollte p stehen
//   {
//     s
//     std::stringstream IfilePathStream;
//     IfilePathStream << baseFileName;
//     if (i > 0)
//     { // kondition das der erste file richtige gelesen wird
//       IfilePathStream << "_" << i;
//     }
    
//     std::string IfilePath = IfilePathStream.str() + "_intermediate.root";
//     const char *IfilepathCStr = IfilePath.c_str();
    
    
    // ** CEJ: Change to loop later // 
    //TFile *file = TFile::Open(IfilepathCStr);
    TFile *file = TFile::Open("run_9_intermediate.root");
    //std::cout << "Open file: " << IfilepathCStr << std::endl;

    TTree *tree = dynamic_cast<TTree *>(file->Get("IntermediateTree"));
    
    // Initialise TreeReader
	TTreeReader tr(tree);
	
	// Retrieve Branches
	TTreeReaderValue<ULong64_t> Trigger_Time(tr, "Trigger_Time");
	TTreeReaderValue<Long64_t> X1(tr, "X1");
	TTreeReaderValue<Long64_t> X2(tr, "X2");
	TTreeReaderValue<Long64_t> Y1(tr, "Y1");
	TTreeReaderValue<Long64_t> Y2(tr, "Y2");
    TTreeReaderValue<Bool_t> Complete(tr, "Complete");
    TTreeReaderValue<Int_t> MCP(tr, "MCP");

	std::cout << "Processing " << tree->GetEntries() << " entries." << std::endl;
	
    // more counters
    int completeEvent = 0;
    int incompleteEvent = 0;
    int completeMCP1 = 0;
    int completeMCP2 = 0;
    int incompleteMCP1 = 0;
    int incompleteMCP2 = 0;

    Long64_t T1 = 0;
    Long64_t T2 = 0;
    Bool_t CompleteMCP1 = false;
    Bool_t CompleteMCP2 = false;
    EntryMCP entryMCP1;
    EntryMCP entryMCP2;
    int countMCP1 = 0;
    int countMCP2 = 0;

	while (tr.Next())
	{
        if (*MCP == 1 && *Complete && countMCP1 == 0)
        {
            entryMCP1.Trigger_Time = *Trigger_Time;
            entryMCP1.X1 = *X1;
            entryMCP1.X2 = *X2;
            entryMCP1.Y1 = *Y1;
            entryMCP1.Y2 = *Y2;

            completeMCP1++;
            countMCP1++;
        }

        if (*MCP == 2 && *Complete && countMCP2 == 0)
        {
            entryMCP2.Trigger_Time = *Trigger_Time;
            entryMCP2.X1 = *X1;
            entryMCP2.X2 = *X2;
            entryMCP2.Y1 = *Y1;
            entryMCP2.Y2 = *Y2;

            completeMCP2++;
            countMCP2++;
        }

        if (entryMCP1.Trigger_Time != 0 && entryMCP2.Trigger_Time != 0)
        {   
            Long64_t tdiff = entryMCP1.Trigger_Time - entryMCP2.Trigger_Time;
            h1_mcp_tdiff->Fill(tdiff);
        
            Long64_t xdiffMCP1 = entryMCP1.X1 - entryMCP1.X2;
            Long64_t ydiffMCP1 = entryMCP1.Y1 - entryMCP1.Y2;
            Long64_t xdiffMCP2 = entryMCP2.X1 - entryMCP2.X2;
            Long64_t ydiffMCP2 = entryMCP2.Y1 - entryMCP2.Y2;
            

            h2_DDL_heatmap_MCP1->Fill(xdiffMCP1, ydiffMCP1);
            if (xdiffMCP1 > gates[0] && xdiffMCP1 < gates[1] && ydiffMCP1 > gates[2] && ydiffMCP1 < gates[3])
            {
                h1_deltaX_mcp1->Fill(xdiffMCP1);
            }
            if (xdiffMCP1 > gates[6] && xdiffMCP1 < gates[7] && ydiffMCP1 > gates[4] && ydiffMCP1 < gates[5])
            {
                h1_deltaY_mcp1->Fill(ydiffMCP1);
            }

            
            h2_DDL_heatmap_MCP2->Fill(xdiffMCP2, ydiffMCP2);
            if (xdiffMCP2 > gates[0] && xdiffMCP2 < gates[1] && ydiffMCP2 > gates[2] && ydiffMCP2 < gates[3])
            {
                h1_deltaX_mcp2->Fill(xdiffMCP2);
            }
            if (xdiffMCP2 > gates[6] && xdiffMCP2 < gates[7] && ydiffMCP2 > gates[4] && ydiffMCP2 < gates[5])
            {
                h1_deltaY_mcp2->Fill(ydiffMCP2);
            }

            // Gate on time difference between MCPs
            if (tdiff > tdiff_gates[0] && tdiff < tdiff_gates[1])
            {
                // other stuff
                h2_DDL_heatmap_MCP1_tgated->Fill(xdiffMCP1, ydiffMCP1);
                h2_DDL_heatmap_MCP2_tgated->Fill(xdiffMCP2, ydiffMCP2);
            }

            // Positional gate on MCP
            if (mcp1_cut->IsInside(xdiffMCP1, ydiffMCP1))
            {
                h2_DDL_heatmap_MCP1_showgate->Fill(xdiffMCP1, ydiffMCP1);
                h2_DDL_heatmap_MCP2_posgated->Fill(xdiffMCP2, ydiffMCP2);
                h1_mcp_tdiff_mcp1gate->Fill(tdiff);

                // mcp2 gate?
            }


            completeEvent++;

            // reset
            countMCP1 = 0;
            countMCP2 = 0;
            entryMCP1.Trigger_Time = 0;
            entryMCP2.Trigger_Time = 0;

        
        }
        

        // if (!*Complete)
        // {
        //     //??
        //     incompleteEvent++;
        //     if (*MCP == 1) incompleteMCP1++;
        //     if (*MCP == 2) incompleteMCP2++;
        // }
    
	} // tree reader loop

	std::cout << "Complete events: " << completeEvent << "(" << 100 * completeEvent / (completeEvent + incompleteEvent) << "%)" << std::endl;
	std::cout << "Incomplete events: " << incompleteEvent<< "(" << 100 * incompleteEvent / (completeEvent + incompleteEvent) << "%)" << std::endl;

    std::cout << "Complete MCP1: " << completeMCP1 << std::endl;
	std::cout << "Complete MPC2: " << completeMCP2 << std::endl;

    // std::cout << "Incomplete MCP1: " << incompleteMCP1 << std::endl;
	// std::cout << "Incomplete MPC2: " << incompleteMCP2 << std::endl;


//   } // file loop
  
  std::cout << "Writing histograms to file." << std::endl;
  
  // Write histograms to file
  of->cd();
  
  c_delta_xy_MCP1->cd(1);
  h1_deltaX_mcp1->Draw();
  c_delta_xy_MCP1->cd(2);
  h1_deltaY_mcp1->Draw();
  
  c_delta_xy_MCP2->cd(1);
  h1_deltaX_mcp2->Draw();
  c_delta_xy_MCP2->cd(2);
  h1_deltaY_mcp2->Draw();
  
  c_heatmap_MCP1->cd();
  gPad->SetLogz(); // logcolorbar
  gStyle->SetOptStat(110010);
  h2_DDL_heatmap_MCP1->Draw("COLZ");
  
  c_heatmap_MCP2->cd();
  gPad->SetLogz(); // logcolorbar
  gStyle->SetOptStat(110010);
  h2_DDL_heatmap_MCP2->Draw("COLZ");
  
  // CEJ: keeping in case desired later  
  if(plotter ==1)
  {
	TAxis *xaxis = h2_DDL_heatmap_MCP1->GetXaxis();

	//et custom labels for x-axis
	for (int i = -150000; i <= 150000; i+=50000) 
	{
		TString label = TString::Format("%d", i/1000); // convert to k units
		xaxis->SetBinLabel(xaxis->FindBin(i), label);
	}

	// Get the y-axis
	TAxis *yaxis = h2_DDL_heatmap_MCP1->GetYaxis();

	// Set custom labels for y-axis
	for (int i = -150000; i <= 150000; i+=50000) 
	{
		TString label = TString::Format("%d", i/1000); // convert to k units
		yaxis->SetBinLabel(yaxis->FindBin(i), label);
	}
  }

  of->cd();
  h1_mcp_tdiff->Write();
  h1_mcp1_event_dt->Write();
  h1_mcp_tdiff_mcp1gate->Write();
  h1_labr_energy->Write();
  h1_germanium_energy->Write();
  c_delta_xy_MCP1->Write();
  c_delta_xy_MCP2->Write();
  c_heatmap_MCP1->Write();
  c_heatmap_MCP2->Write();
  h2_DDL_heatmap_MCP1_tgated->Write();
  h2_DDL_heatmap_MCP2_tgated->Write();
  h2_DDL_heatmap_MCP1_showgate->Write();
  h2_DDL_heatmap_MCP2_showgate->Write();
  h2_DDL_heatmap_MCP2_posgated->Write();
  dir_dssd->cd();
  dir_dssd->Write();


  of->Close();

  // Clean up
  delete c_delta_xy_MCP1;
  delete c_delta_xy_MCP2;
  delete c_heatmap_MCP1;
  delete c_heatmap_MCP2;
  delete of;

  return 0;
}
